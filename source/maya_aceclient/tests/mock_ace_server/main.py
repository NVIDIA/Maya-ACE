# SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: MIT
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
import argparse
import concurrent
import os
import subprocess
import sys
import time

import grpc
import numpy as np

# to ensure nvidia_ace can be imported because all the grpc files use absolute import.
current_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(current_dir, "grpc_generated"))

from mock_ace_server.grpc_generated import health_pb2_grpc
from mock_ace_server.grpc_generated.nvidia_ace.services.a2f_controller import (
    v1_pb2_grpc as pb2_grpc,
)
from mock_ace_server.pb2_all import *

BLENDSHAPES = [f"face{i}" for i in range(52)]
MOCK_SERVER_ADDR_PORT = "127.0.0.1:50051"


class MockHealthServicer(health_pb2_grpc.HealthServicer):

    def Check(self, request, context):
        return HealthCheckResponse()


class MockA2FControllerServiceServicer(pb2_grpc.A2FControllerServiceServicer):

    def ProcessAudioStream(self, request_iterator, context):
        """Receives AudioStream and yields AnimationDataStream
        """
        start_time = time.time()
        # process header
        first_chunk = next(request_iterator)
        if not first_chunk.HasField("audio_stream_header"):
            msg = "The header must be sent as the first message. Streaming aborted."
            status = Status(message=msg, code=Status.Code.ERROR)
            yield AnimationDataStream(status=status)
            for _ in request_iterator:
                pass
            raise Exception(msg)

        # respond header
        audio_stream_header = first_chunk.audio_stream_header
        audio_header = audio_stream_header.audio_header
        anim_header = AnimationDataStreamHeader(
            audio_header=audio_header,
            skel_animation_header=SkelAnimationHeader(
                blend_shapes=BLENDSHAPES,
                joints=["head", "neck"],
            ),
            start_time_code_since_epoch=start_time,
        )
        yield AnimationDataStream(animation_data_stream_header=anim_header)

        received_audio_samples = 0
        for chunk in request_iterator:
            if chunk.HasField("end_of_audio"):
                event = Event(event_type=EventType.END_OF_A2F_AUDIO_PROCESSING)
                yield AnimationDataStream(event=event)
                break  # stop processing audio

            audio_data = chunk.audio_with_emotion
            audio_buffer = audio_data.audio_buffer
            audio_array = np.frombuffer(audio_buffer, dtype=np.int16)
            num_audio_array = len(audio_array)
            timecode_offset = received_audio_samples / audio_header.samples_per_second
            received_audio_samples += len(audio_array)

            # assuming the sent audio buffer is of 16k sample rate. and we generate frames at 30 fps
            # 533 = 16000 / 30
            for cursor in range(0, num_audio_array, 533):
                timecode = cursor / audio_header.samples_per_second + timecode_offset
                return_buffer = audio_array[cursor : min(cursor + 533, num_audio_array)]
                if len(return_buffer) < 1:
                    continue
                animation_data = AnimationData(
                    skel_animation=SkelAnimation(
                        blend_shape_weights=[
                            FloatArrayWithTimeCode(
                                time_code=timecode,
                                values=[1.0 + i / len(BLENDSHAPES) for i in range(len(BLENDSHAPES))],  # fake data
                            )
                        ]
                    ),
                    audio=AudioWithTimeCode(
                        time_code=timecode,
                        audio_buffer=return_buffer.tobytes(),  # audio feedback
                    ),
                )
                data = AnimationDataStream(animation_data=animation_data)
                yield data

        # clear all leftover in the stream
        for _ in request_iterator:
            status = Status(message=f"received data after end of audio.", code=Status.Code.WARNING)
            yield AnimationDataStream(status=status)

        # send ending marker
        status = Status(message=f"sent all data", code=Status.Code.SUCCESS)
        yield AnimationDataStream(status=status)


def setup_grpc_server(url):
    addr_port = url.replace("http://", "").replace("https://", "")
    server = grpc.server(concurrent.futures.ThreadPoolExecutor(max_workers=10))
    pb2_grpc.add_A2FControllerServiceServicer_to_server(MockA2FControllerServiceServicer(), server)
    health_pb2_grpc.add_HealthServicer_to_server(MockHealthServicer(), server)
    server.add_insecure_port(addr_port)
    return server


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="run ace mock server locally")
    parser.add_argument("args", nargs=argparse.REMAINDER, help="commands to run after started the mock server")
    args = parser.parse_args()

    server = setup_grpc_server(MOCK_SERVER_ADDR_PORT)
    server.start()
    print(f"Mock server started, listening on {MOCK_SERVER_ADDR_PORT}")
    if args.args:
        script_dir = os.path.dirname(__file__)
        project_root_dir = os.path.join(script_dir, "../../../../")
        print("cwd:", os.path.abspath(project_root_dir))
        print(" ".join(args.args))
        subprocess.run(" ".join(args.args), shell=True, cwd=project_root_dir)
        server.stop(0)
        print("Mock server stopped.")
    else:
        server.wait_for_termination()
