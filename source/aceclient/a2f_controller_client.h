// SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>
#include <grpcpp/channel.h>
#include <grpcpp/security/credentials.h>
#include "ace_grpc_cpp/health.grpc.pb.h"
#include "ace_grpc_cpp/nvidia_ace.services.a2f_controller.v1.grpc.pb.h"

#include "aceclient/aceclient.h"
#include "aceclient/parameters.h"
#include "aceclient/frame_receiver.h"

using grpc::health::v1::Health;
using ::nvidia_ace::services::a2f_controller::v1::A2FControllerService;
using ::nvidia_ace::a2f::v1::EmotionPostProcessingParameters;
using ::nvidia_ace::controller::v1::AudioStreamHeader;

class TestA2FControllerClient_TestSetup_Test;
class TestA2FControllerClient_TestSetBlendshapeParameters_Test;
class TestA2FControllerClient_TestBuildAudioStreamHeader_Test;

namespace mace {

class A2FControllerClient {
public:
    A2FControllerClient(
        std::shared_ptr<grpc::Channel> channel,
        std::string apiKey,
        std::string functionId
    );
    A2FControllerClient(
        std::shared_ptr<A2FControllerService::StubInterface> stub,
        std::string apiKey,
        std::string functionId
    );

    AceClientStatus ProcessAudioStream(
        const int16_t *samples,
        size_t sample_count,
        AceEmotionState input_emotion_state,
        std::vector<AnimDataFrame> *out_frames
    );

    void SetFaceParam(const char *key, float val);
    void SetEmotionPostProcessingParams(EmotionPostProcessingParameters &param);
    void SetBlendshapeMultiplier(const char *key, float value);
    void SetBlendshapeOffset(const char *key, float value);

protected:
    std::shared_ptr<A2FControllerService::StubInterface> m_stub;
    std::string m_apiKey;
    std::string m_functionId;

    EmotionPostProcessingParameters m_emotionPostProcessingParameters;
    google::protobuf::Map<std::string, float> m_faceParameters;
    google::protobuf::Map<std::string, float> m_blendshapeMultipliers;
    google::protobuf::Map<std::string, float> m_blendshapeOffsets;

    void buildAudioStreamHeader(AudioStreamHeader* stream_header);

// allow tests/test_a2f_controller_client.cpp to verify the private member variables
friend class ::TestA2FControllerClient_TestSetup_Test;
friend class ::TestA2FControllerClient_TestSetBlendshapeParameters_Test;
friend class ::TestA2FControllerClient_TestBuildAudioStreamHeader_Test;
};

AceClientStatus A2FControllerHealthCheck(
    std::shared_ptr<grpc::Channel> channel, std::string apiKey, std::string functionId);
AceClientStatus A2FControllerHealthCheck(
    std::shared_ptr<Health::StubInterface> stub, std::string apiKey, std::string functionId);

} // namespace mace
