# SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

# ------------------------------------------------------------------------------------------------
# this script is based on the ACE sample scripts provided by NVIDIA on github
# https://github.com/NVIDIA/ACE/tree/main/microservices/audio_2_face_microservice/1.2
# it has been modified to work with the A2F-3D microservice
# it has been modified to work with the MetaHuman Mapping
# it has been modified to work with the MetaHuman FBX axis system in Maya and Unreal
# ------------------------------------------------------------------------------------------------
import argparse
import asyncio
import json
import os
import sys

import batch_functions as bf
import grpc
import numpy as np
import scipy
import yaml
from nvidia_ace.a2f.v1_pb2 import (
    AudioWithEmotion,
    BlendShapeParameters,
    EmotionPostProcessingParameters,
    FaceParameters,
)
from nvidia_ace.animation_data.v1_pb2 import AnimationData, AnimationDataStreamHeader
from nvidia_ace.audio.v1_pb2 import AudioHeader
from nvidia_ace.controller.v1_pb2 import AudioStream, AudioStreamHeader
from nvidia_ace.emotion_aggregate.v1_pb2 import EmotionAggregate
from nvidia_ace.emotion_with_timecode.v1_pb2 import EmotionWithTimeCode
from nvidia_ace.services.a2f_controller.v1_pb2_grpc import A2FControllerServiceStub

# import A2F Microservice communications gRPC proto - this needs to be configured before running the script
# see README.md for more details

# Bit depth of the audio file, only 16 bit PCM audio is currently supported.
BITS_PER_SAMPLE = 16
# Channel count, only mono audio is currently supported.
CHANNEL_COUNT = 1
# Audio format, only PCM is supported.
AUDIO_FORMAT = AudioHeader.AUDIO_FORMAT_PCM


def get_audio_bit_format(audio_header: AudioHeader):
    """
    Reads the audio_header parameters and returns the write type to interpret
    the audio data sent back by the server.
    """
    if audio_header.audio_format == AudioHeader.AUDIO_FORMAT_PCM:
        # We only support 16 bits PCM.
        if audio_header.bits_per_sample == 16:
            return np.int16
    return None


def save_audio_data_to_file(outdir: str, audio_header: AudioHeader, audio_buffer: bytes):
    """
    Reads the AudioHeader and output the content of the audio buffer into a wav
    file.
    """
    # Type of the audio data to output.
    dtype = get_audio_bit_format(audio_header)
    if dtype is None:
        print("Error while downloading data, unknown format for audio output", file=sys.stderr)
        return

    audio_data_to_save = np.frombuffer(audio_buffer, dtype=dtype)
    # Write the audio data output as a wav file.
    scipy.io.wavfile.write(f"{outdir}/out.wav", audio_header.samples_per_second, audio_data_to_save)


def parse_emotion_data(animation_data, emotion_key_frames):
    """
    Fills the emotion key frames dictionnary using the data found in the emotion_aggregate metadata.

    Each emotion aggregate contains the following values:
    - input_emotions: Emotions that are manually inputed by the user.
    - a2e_output: The output of the emotion inference on the audio out of Audio2Emotion.
    - a2f_smoothed_output: The smoothed and post-processed emotions output, used for the actual
        blendshape generation.

    They are grouped into `emotion key frames` which are a timestamp as well as emotion parameters.
    """
    emotion_aggregate: EmotionAggregate = EmotionAggregate()
    # Metadata is an Any type, try to unpack it into an EmotionAggregate object
    if animation_data.metadata["emotion_aggregate"] and animation_data.metadata[
        "emotion_aggregate"
    ].Unpack(emotion_aggregate):
        for emotion_with_timecode in emotion_aggregate.a2e_output:
            emotion_key_frames["a2e_output"].append(
                {
                    "time_code": emotion_with_timecode.time_code,
                    "emotion_values": dict(emotion_with_timecode.emotion),
                }
            )
        for emotion_with_timecode in emotion_aggregate.input_emotions:
            emotion_key_frames["input"].append(
                {
                    "time_code": emotion_with_timecode.time_code,
                    "emotion_values": dict(emotion_with_timecode.emotion),
                }
            )
        for emotion_with_timecode in emotion_aggregate.a2f_smoothed_output:
            emotion_key_frames["a2f_smoothed_output"].append(
                {
                    "time_code": emotion_with_timecode.time_code,
                    "emotion_values": dict(emotion_with_timecode.emotion),
                }
            )


async def read_from_stream(stream, mapping, destinationpath, FBX_axis_system):
    """
    Reads the content of the stream using the read() method of the StreamStreamCall object.
    """

    # List of blendshapes names recovered from the model data in the AnimationDataStreamHeader
    bs_names = []
    # List of animation key frames, meaning a time code and the values of the blendshapes
    animation_key_frames = []
    # Audio buffer that contains the result
    audio_buffer = b""
    # Audio header to store metadata for audio saving
    audio_header: AudioHeader = None
    # Emotions 'key frames' data from input, a2e output and final a2f smoothed output.
    emotion_key_frames = {"input": [], "a2e_output": [], "a2f_smoothed_output": []}
    # Reads the content of the stream using the read() method of the StreamStreamCall object.
    while True:
        # Read an incoming packet.
        message = await stream.read()
        if message == grpc.aio.EOF:

            # call extra function to save animation in various pipeline formats
            bf.save_keyframes_to_MH_animation(
                mapping, animation_key_frames, destinationpath, FBX_axis_system
            )

            return

        if message.HasField("animation_data_stream_header"):
            # Message is a header
            print("Receiving data from server...")
            animation_data_stream_header: AnimationDataStreamHeader = (
                message.animation_data_stream_header
            )
            # Save blendshapes names for later use
            bs_names = animation_data_stream_header.skel_animation_header.blend_shapes
            # Save audio header for later use
            audio_header = animation_data_stream_header.audio_header

        elif message.HasField("animation_data"):
            print(".", end="", flush=True)
            # Message is animation data.
            animation_data: AnimationData = message.animation_data
            parse_emotion_data(animation_data, emotion_key_frames)
            blendshape_list = animation_data.skel_animation.blend_shape_weights
            for blendshapes in blendshape_list:
                # We assign each blendshape name to its corresponding weight.
                bs_values_dict = dict(zip(bs_names, blendshapes.values))
                time_code = blendshapes.time_code
                # Append an object to the list of animation key frames
                animation_key_frames.append({"timeCode": time_code, "blendShapes": bs_values_dict})
            # Append audio data to the final audio buffer.
            audio_buffer += animation_data.audio.audio_buffer
        elif message.HasField("status"):
            # Message is status
            print()
            status = message.status
            print(f"Received status message with value: '{status.message}'")
            print(f"Status code: '{status.code}'")


async def write_to_stream(stream, config_path, audio_file_path):

    remove_audio_file = False

    # Read the content of the audio file, extracting sample rate and data.

    samplerate, data = scipy.io.wavfile.read(audio_file_path)

    # convert file format if not supported by A2F Microservice
    try:
        if (data.dtype != np.int16) or (samplerate != 16000):
            audio_file_path = bf.convert_audio(audio_file_path)
            samplerate, data = scipy.io.wavfile.read(audio_file_path)
            remove_audio_file = True
    except Exception as e:
        print(
            f"Error: {e}. Please check the audio file format and sample rate. A2F Microservice only supports 16 bit PCM audio at 16000 Hz."
        )
        sys.exit(1)

    config = None

    if os.path.splitext(config_path)[1] == ".json":
        try:
            with open(config_path, "r") as jsonparamsfile:
                json_data = json.load(jsonparamsfile)
                keys = list(json_data.keys())
                if "face_params" in keys:
                    a2f_params = json_data
                elif "a2f" in keys:
                    # convert params format to old json format for A2F Microservice
                    a2f_params = bf.process_a2f_params(json_data)
                else:
                    print(f"Error: {config_path} is not a valid A2F parameters file")
                    sys.exit(1)
        except Exception as e:
            print(f"Error: {e}")
            sys.exit(1)

    elif os.path.splitext(config_path)[1] == ".yaml":
        try:
            with open(config_path, "r") as yamlparamsfile:
                yaml_data = yaml.safe_load(yamlparamsfile)
                a2f_params = bf.process_a2f_params(yaml_data)
        except Exception as e:
            print(f"Error: {e}")
            sys.exit(1)

    else:
        print(f"Error: {config_path} is not a valid A2F parameters file")
        sys.exit(1)

    # build parameters for the A2F Microservice gRPC proto

    emotion_post_processing_parameters = {
        "emotion_contrast": a2f_params["emotion_params"]["emotion_contrast"],
        "live_blend_coef": a2f_params["emotion_params"]["live_blend_coef"],
        "enable_preferred_emotion": a2f_params["emotion_params"]["enable_preferred_emotion"],
        "preferred_emotion_strength": a2f_params["emotion_params"]["preferred_emotion_strength"],
        "emotion_strength": a2f_params["emotion_params"]["emotion_strength"],
        "max_emotions": a2f_params["emotion_params"]["max_emotions"],
    }

    face_parameters = {
        "upperFaceStrength": a2f_params["face_params"]["upper_face_strength"],
        "upperFaceSmoothing": a2f_params["face_params"]["upper_face_smoothing"],
        "lowerFaceStrength": a2f_params["face_params"]["lower_face_strength"],
        "lowerFaceSmoothing": a2f_params["face_params"]["lower_face_smoothing"],
        "faceMaskLevel": a2f_params["face_params"]["face_mask_level"],
        "faceMaskSoftness": a2f_params["face_params"]["face_mask_softness"],
        "skinStrength": a2f_params["face_params"]["skin_strength"],
        "eyelidOpenOffset": a2f_params["face_params"]["eyelid_offset"],
        "lipOpenOffset": a2f_params["face_params"]["lip_close_offset"],
    }

    BlendShapeMultipliers = {}
    BlendShapeOffsets = {}
    count = 0

    for a in a2f_params["blendshape_names"]:
        BlendShapeMultipliers[a] = a2f_params["blendshape_params"]["bsWeightMultipliers"][count]
        BlendShapeOffsets[a] = a2f_params["blendshape_params"]["bsWeightOffsets"][count]
        count += 1

    # Each message in the Stream should be an AudioStream message.
    # An AudioStream message can be composed of the following messages:
    # - AudioStreamHeader: must be the first message to be send,
    #       contains metadata about the audio file.
    # - AudioWithEmotion: audio bytes as well as emotions to apply.
    # - EndOfAudio: final message to signal audio sending termination.
    audio_stream_header = AudioStream(
        audio_stream_header=AudioStreamHeader(
            audio_header=AudioHeader(
                samples_per_second=samplerate,
                bits_per_sample=BITS_PER_SAMPLE,
                channel_count=CHANNEL_COUNT,
                audio_format=AUDIO_FORMAT,
            ),
            emotion_post_processing_params=EmotionPostProcessingParameters(
                **emotion_post_processing_parameters
            ),
            face_params=FaceParameters(float_params=face_parameters),
            blendshape_params=BlendShapeParameters(
                bs_weight_multipliers=BlendShapeMultipliers,
                bs_weight_offsets=BlendShapeOffsets,
            ),
        )
    )

    # Sending the AudioStreamHeader message encapsulated into an AudioStream object.
    await stream.write(audio_stream_header)

    for i in range(len(data) // samplerate + 1):
        # Cutting the audio into arbitrary chunks, here we use sample rate to send exactly one
        # second of audio per packet but the size does not matter.
        chunk = data[i * samplerate : i * samplerate + samplerate]
        # Send audio buffer to A2F.
        # Packet 0 contains the emotion with timecode list
        # Here we send all the emotion with timecode alongside the first audio buffer
        # as they are available. In a streaming scenario if you don't have access
        # to some emotions right away you can send them in the next audio buffers.
        if i == 0:

            TimeCodeName = "emotion_with_timecode1"
            KeyframeTime = 0.0

            EmotionAtTimeCode = {
                TimeCodeName: {
                    "time_code": KeyframeTime,
                    "emotions": {
                        "amazement": a2f_params["preferred_emotion"][0],
                        "anger": a2f_params["preferred_emotion"][1],
                        "cheekiness": a2f_params["preferred_emotion"][2],
                        "disgust": a2f_params["preferred_emotion"][3],
                        "fear": a2f_params["preferred_emotion"][4],
                        "grief": a2f_params["preferred_emotion"][5],
                        "joy": a2f_params["preferred_emotion"][6],
                        "outofbreath": a2f_params["preferred_emotion"][7],
                        "pain": a2f_params["preferred_emotion"][8],
                        "sadness": a2f_params["preferred_emotion"][9],
                    },
                }
            }

            list_emotion_tc = [
                EmotionWithTimeCode(emotion={**v["emotions"]}, time_code=v["time_code"])
                for v in EmotionAtTimeCode.values()
            ]

            await stream.write(
                AudioStream(
                    audio_with_emotion=AudioWithEmotion(
                        audio_buffer=chunk.astype(np.int16).tobytes(), emotions=list_emotion_tc
                    )
                )
            )
        else:
            # Send only the audio buffer
            await stream.write(
                AudioStream(
                    audio_with_emotion=AudioWithEmotion(
                        audio_buffer=chunk.astype(np.int16).tobytes()
                    )
                )
            )
    # Sending the EndOfAudio message to signal end of sending.
    # This is necessary to obtain the status code at the end of the generation of
    # blendshapes. This status code tells you about the end of animation data stream.
    await stream.write(AudioStream(end_of_audio=AudioStream.EndOfAudio()))

    # remove audio file if it was converted
    if remove_audio_file and os.path.exists(audio_file_path):
        os.remove(audio_file_path)

    return


# command line arguments before main function
# function to capture optional arguments to the batch script
parser = argparse.ArgumentParser(
    description=("Sample python batch for audio2face."),
    epilog="NVIDIA CORPORATION.  All rights reserved.",
)

# optional arguments list
parser.add_argument(
    "-bl",
    type=str,
    default=r"BatchList.json",
    help="Optionally specify the json file containing the batch list (default: BatchList.json)",
)
parser.add_argument(
    "-mhm",
    type=str,
    default=r"MH_Mapping.json",
    help="Optionally specify the json file containing the MetaHuman Mapping (default: MH_Mapping.json)",
)
parser.add_argument(
    "-url",
    type=str,
    default=r"127.0.0.1:52000",
    help="Optionally specify the a2f url to connect to (default: 127.0.0.1:52000)",
)


# main
async def main():

    # get arguments from the command line if they have been provided (or use defaults)
    # defaults assume a local A2F-3D microservice (V1.2) running locally and listening to port 52000
    args = parser.parse_args()
    BatchListFile = args.bl
    a2f_ms_url = args.url
    jsonMappingpath = args.mhm

    # open batch list file
    try:
        with open(BatchListFile, "r") as json_file:
            batchlist = json.load(json_file)
    except FileNotFoundError:
        print(f"Error: Batch list file '{BatchListFile}' not found")
        sys.exit(1)
    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON in batch list file: {e}")
        sys.exit(1)

    # extract audio, paramters and destination folder from the batch list
    audiofiles_folder = batchlist["audiofiles_folder"]
    paramsfiles_folder = batchlist["paramsfiles_folder"]
    destination_folder = batchlist["destination_folder"]
    audiofiles = batchlist["audiofiles"]
    FBX_axis_system = batchlist["FBX_axis_system"]

    # for all audio & parameters pair, generate the animation
    for audio_pair in audiofiles:

        # build full paths for audio, parameters and destination folders
        audiofilepath = audiofiles_folder + "/" + audio_pair[0]
        a2f_paramspath = paramsfiles_folder + "/" + audio_pair[1]
        destinationpath = (
            destination_folder
            + "/"
            + os.path.splitext(audio_pair[0])[0]
            + "_"
            + os.path.splitext(audio_pair[1])[0]
        )
        print("Generating A2F animation : " + destinationpath)

        # Creating an insecure channel to connect to the A2F controller.
        # If behind HTTPS proxy or using HTTPS, please refer to
        # https://grpc.github.io/grpc/python/grpc_asyncio.html#grpc.aio.secure_channel
        async with grpc.aio.insecure_channel(a2f_ms_url) as c:
            # Creating a stub for the service. This allows us to use the remote channel to communicate
            # via RPC to the controller.
            stub = A2FControllerServiceStub(c)

            # ProcessAudioStream is a bidirectionnal stream, or StreamStreamCall object
            # It exposes a read and write interface as shown here:
            # https://grpc.github.io/grpc/python/grpc_asyncio.html#grpc.aio.StreamStreamCall
            stream = stub.ProcessAudioStream()
            # We create an asyncio task for reading the content of the string, into a async function
            # called read_from_stream.
            read = asyncio.create_task(
                read_from_stream(stream, jsonMappingpath, destinationpath, FBX_axis_system)
            )
            # We create another asyncio task for writing into the stream. This allows us to run them
            # both in parrallel instead of sequentially.
            write = asyncio.create_task(write_to_stream(stream, a2f_paramspath, audiofilepath))
            # Await both tasks termination.
            await write
            await read


if __name__ == "__main__":
    asyncio.run(main())
