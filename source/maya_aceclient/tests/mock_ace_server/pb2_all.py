# SPDX-FileCopyrightText: Copyright (c) 2018-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

from health_pb2 import HealthCheckResponse
from nvidia_ace.a2f.v1_pb2 import (
    AudioWithEmotion,
    EmotionPostProcessingParameters,
    FaceParameters,
)
from nvidia_ace.animation_data.v1_pb2 import (
    AnimationData,
    AudioWithTimeCode,
    Camera,
    Float3,
    Float3ArrayWithTimeCode,
    Float3WithTimeCode,
    FloatArrayWithTimeCode,
    FloatWithTimeCode,
    QuatF,
    QuatFArrayWithTimeCode,
    QuatFWithTimeCode,
    SkelAnimation,
    SkelAnimationHeader,
)
from nvidia_ace.animation_id.v1_pb2 import AnimationIds
from nvidia_ace.audio.v1_pb2 import AudioHeader
from nvidia_ace.controller.v1_pb2 import (
    AnimationDataStream,
    AnimationDataStreamHeader,
    AudioStream,
    AudioStreamHeader,
    Event,
    EventType,
)
from nvidia_ace.emotion_aggregate.v1_pb2 import EmotionAggregate
from nvidia_ace.emotion_with_timecode.v1_pb2 import EmotionWithTimeCode
from nvidia_ace.status.v1_pb2 import Status
