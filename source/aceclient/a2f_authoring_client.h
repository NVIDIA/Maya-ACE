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
#include "a2f_client_interface.h"

#include "ace_grpc_cpp/nvidia_ace.services.a2f_authoring.v1.grpc.pb.h"

using ::nvidia_ace::services::a2f_authoring::v1::A2FAuthoringService;

namespace mace {

class A2FAuthoringClient : public IA2FClient {
public:
    A2FAuthoringClient(
        std::shared_ptr<grpc::Channel> channel,
        std::string api_key,
        std::string function_id
    );
    A2FAuthoringClient(
        std::shared_ptr<A2FAuthoringService::StubInterface> stub,
        std::string api_key,
        std::string function_id
    );

    AceClientResult SendAudio(
        std::vector<int16_t> const &samples,
        AceEmotionState input_emotion_state,
        std::vector<AnimDataFrame> *out_frames
    ) override;

    AceClientResult FetchFrame(
        float timestamp,
        AceEmotionState input_emotion_state,
        AnimDataFrame *out_frame
    ) override;

    void SetFaceParam(const std::string key, float val) override;
    void SetEmotionPostProcessingParams(EmotionPostProcessingParameters &param) override;
    void SetBlendshapeMultiplier(const std::string key, float value) override;
    void SetBlendshapeOffset(const std::string key, float value) override;
private:
    AceClientResult UploadAudioClip(
        const int16_t *samples,
        size_t sample_count,
        std::string& out_clip_id,
        std::vector<std::string>& out_blendshape_names
    );
    AceClientResult GetAvatarFacePose(
        std::string& audio_clip_id,
        AceEmotionState input_emotion_state,
        float timestamp,
        std::vector<float>& out_blendshapes,
        std::vector<float>& out_emotion_states
    );

    std::shared_ptr<A2FAuthoringService::StubInterface> m_stub;

    std::string m_api_key = "";
    std::string m_function_id = "";
    std::string m_audio_clip_id = "";

    EmotionPostProcessingParameters m_emotion_params;
    google::protobuf::Map<std::string, float> m_face_params;
    google::protobuf::Map<std::string, float> m_blendshape_mutlipliers;
    google::protobuf::Map<std::string, float> m_blendshape_offsets;
};

}
