// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#pragma once

#include <memory>

#include <grpcpp/grpcpp.h>
#include <grpcpp/channel.h>
#include <grpcpp/security/credentials.h>

#include "ace_grpc_cpp/nvidia_ace.a2f.v1.pb.h"

#include "frame_receiver.h"
#include "parameters.h"
#include "status.h"

using ::nvidia_ace::a2f::v1::EmotionPostProcessingParameters;

namespace mace {

class IA2FClient {
public:
    virtual ~IA2FClient() {};

    virtual AceClientResult SendAudio(
        std::vector<int16_t> const &samples,
        AceEmotionState input_emotion_state,
        std::vector<AnimDataFrame> *out_frames
    ) = 0;

    virtual AceClientResult FetchFrame(
        float timestamp,
        AceEmotionState input_emotion_state,
        AnimDataFrame *out_frame
    ) = 0;

    virtual void SetFaceParam(const std::string key, float val) = 0;
    virtual void SetEmotionPostProcessingParams(EmotionPostProcessingParameters &param) = 0;
    virtual void SetBlendshapeMultiplier(const std::string key, float value) = 0;
    virtual void SetBlendshapeOffset(const std::string key, float value) = 0;
};

}
