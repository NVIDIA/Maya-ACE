# SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

ENV_DEFAULT_URL = "ACE_ANIMATION_CONTROLLER_DEFAULT_URL"

# to ensure nvidia_ace can be imported because all the grpc files use absolute import.
current_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(current_dir, "grpc_py"))

from grpc_py import health_pb2_grpc
from grpc_py.nvidia_ace.services.a2f_authoring import (
    v1_pb2_grpc as a2f_authoring_pb2_grpc,
)
from grpc_py.nvidia_ace.services.a2f_controller import (
    v1_pb2_grpc as a2f_controller_pb2_grpc,
)
from grpc_py.nvidia_ace.services.a2x_export_config import (
    v1_pb2_grpc as a2x_export_config_pb2_grpc,
)
from pb2_all import *

BLENDSHAPES = [f"face{i}" for i in range(52)]


class MockA2XExportConfigServiceServicer(a2x_export_config_pb2_grpc.A2XExportConfigServiceServicer):

    def GetConfigs(self, request, context):
        print("Request:", request)
        yield A2XConfig(name="config1", content='{"key1":"val1"}')
        yield A2XConfig(name="config2", content='{"key2":"val2"}')


class MockHealthServicer(health_pb2_grpc.HealthServicer):

    def Check(self, request, context):
        return HealthCheckResponse(status=HealthCheckResponse.SERVING)


class MockA2FAuthoringServiceServicer(a2f_authoring_pb2_grpc.A2FAuthoringServiceServicer):

    def UploadAudioClip(self, request, context):
        sha256Handle = "0" * 64
        return AudioClipHandle(
            audio_clip_id=sha256Handle, blendshape_names=[f"blendshape_{i}" for i in range(55)]
        )

    def GetAvatarFacePose(self, request, context):
        return BlendShapeData(time_code=request.time_stamp, blendshapes=[0.0] * 55)


class MockA2FControllerServiceServicer(a2f_controller_pb2_grpc.A2FControllerServiceServicer):

    def ProcessAudioStream(self, request_iterator, context):
        """Receives AudioStream and yields AnimationDataStream"""
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
                                values=[
                                    1.0 + i / len(BLENDSHAPES) for i in range(len(BLENDSHAPES))
                                ],  # fake data
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
    a2f_controller_pb2_grpc.add_A2FControllerServiceServicer_to_server(
        MockA2FControllerServiceServicer(), server
    )
    a2f_authoring_pb2_grpc.add_A2FAuthoringServiceServicer_to_server(
        MockA2FAuthoringServiceServicer(), server
    )
    a2x_export_config_pb2_grpc.add_A2XExportConfigServiceServicer_to_server(
        MockA2XExportConfigServiceServicer(), server
    )
    health_pb2_grpc.add_HealthServicer_to_server(MockHealthServicer(), server)
    try:
        port = server.add_insecure_port(addr_port)
        return server, port
    except Exception as e:
        print(f"Error adding insecure port: {e}")

    # fallback to find a different port
    print("Trying to find a different port")
    port = server.add_insecure_port(addr_port.split(":")[0] + ":0")
    return server, port


class MockServerContext:

    def __init__(self, host="127.0.0.1", port="50051"):
        self.host = host
        self.port = port
        self.server = None

    def __enter__(self):
        mock_server_addr_port = f"{self.host}:{self.port}"

        self.server, self.port = setup_grpc_server(mock_server_addr_port)
        self.server.start()

        # actual port might be different if the one we requested is already in use
        mock_server_addr_port = f"{self.host}:{self.port}"

        # set target server to the mock server for child process
        os.environ[ENV_DEFAULT_URL] = f"http://{mock_server_addr_port}"
        os.environ["MAYA_ACE_UNIT_TEST"] = "1"
        print(f"Mock server started, listening on {mock_server_addr_port}")

    def __exit__(self, exc_type, exc_value, traceback):
        self.server.stop(0)
        self.server.wait_for_termination()

        if exc_type is not None:
            raise exc_value

        return False


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="run ace mock server locally")
    parser.add_argument(
        "--host", default="127.0.0.1", help="IP address that the mock server should listen on"
    )
    parser.add_argument(
        "--port", default="50051", help="Port number that the mock server should listen on"
    )
    parser.add_argument(
        "args", nargs=argparse.REMAINDER, help="commands to run after started the mock server"
    )
    args = parser.parse_args()

    mock_server_addr_port = f"{args.host}:{args.port}"

    use_mock_server = not os.getenv(ENV_DEFAULT_URL, "")

    script_dir = os.path.dirname(__file__)
    project_root_dir = os.path.join(script_dir, "..")
    print("cwd:", os.path.abspath(project_root_dir))
    print(" ".join(args.args))

    if use_mock_server:
        with MockServerContext(args.host, args.port) as ctx:
            # user commands
            result = subprocess.run(" ".join(args.args or []), shell=True, cwd=project_root_dir)
    else:
        result = subprocess.run(" ".join(args.args), shell=True, cwd=project_root_dir)

    sys.exit(result.returncode)
