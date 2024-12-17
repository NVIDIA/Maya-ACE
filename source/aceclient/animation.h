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

#include <iostream>
#include <string>
#include <vector>

#include "a2f_authoring_client.h"
#include "a2f_controller_client.h"
#include "a2f_healthcheck_client.h"

#include "grpc_client.h"
#include "frame_receiver.h"
#include "parameters.h"
#include "status.h"

#define KEY_VALUE std::pair<std::string, float>


namespace mace {

const uint16_t DEFAULT_FRAMERATE = 30;

enum Infinity {
    Constant, Cycle,
};

class NetworkedAnimation : public NVCFGrpcClient {

public:
    // Access to the Animation
    std::vector<float> GetBlendshapeWeights(float seconds, Infinity post_infinity=Constant);
    std::vector<float> GetBlendshapeWeights(size_t frame_index, Infinity post_infinity=Constant);
    std::vector<std::string> GetBlendshapeNames();

    std::vector<float> GetEmotionState(size_t frame_index, Infinity post_infinity=Constant);
    std::vector<std::string> GetEmotionStateNames();

    bool HasAnimation(float seconds=0.0f);
    bool HasAnimation(size_t frame_index=0);

    // Manage Frames
    size_t GetFrameIndex(float seconds);
    size_t GetFramesCount();
    float GetAnimationLength();
    AnimDataFrame GetFrame(size_t frame_index);
    size_t AddFrame(AnimDataFrame &frame);
    size_t RemoveFrames(size_t first, size_t last);
    size_t InsertFrame(size_t after, AnimDataFrame &frame);
    size_t ReplaceFrame(size_t frame_index, AnimDataFrame &frame);

    // Communication Pipeline
    AceClientResult ConnectAndSendAudio(std::vector<int16_t> const &audio_samples);
    AceClientResult FetchAnimationFrame(size_t frame_index);
    AceClientResult CheckServiceStatus(std::shared_ptr<grpc::Channel> channel=nullptr);
    long long GetLastUpdated();

    // Face Parameters
    bool SetFaceParameters(AceFaceParameters const &new_parameters);
    AceFaceParameters const GetFaceParameters();
    std::vector<char *> const ListFaceParameters();

    // Emotion Parameters
    bool SetEmotionParameters(AceEmotionParameters const &new_parameters);
    AceEmotionParameters const GetEmotionParameters();

    // Emotion State (preferred emotion)
    //  NOTE: need change to support keyframes
    bool SetEmotionState(AceEmotionState const &new_emotions);
    AceEmotionState const GetEmotionState();

    // Blendshape Multipliers
    int SetBlendshapeMultiplier(std::string const &key, float value=1.0f);
    int RemoveBlendshapeMultiplier(std::string const &key);
    void ClearBlendshapeMultiplier();
    std::map<std::string, float> const GetBlendshapeMultipliers();
    std::vector<std::string> const GetBlendshapeMultiplierKeys();
    std::vector<float> const GetBlendshapeMultiplierValues();

    // Blendshape Offsets
    int SetBlendshapeOffset(std::string const &key, float value=1.0f);
    int RemoveBlendshapeOffset(std::string const &key);
    void ClearBlendshapeOffset();
    std::map<std::string, float> const GetBlendshapeOffsets();
    std::vector<std::string> const GetBlendshapeOffsetKeys();
    std::vector<float> const GetBlendshapeOffsetValues();

    // Communication Settings
    AceClientResult SetClientType(short new_clieyt_type);
    AceClientResult SetClientType(A2FClientType new_clieyt_type);
    A2FClientType const GetClientType();
    bool IsClientCreated();
    std::string const GetAddress();
    std::string const GetAPIKey();
    std::string const GetFunctionId();

    // Non-API

protected:
    A2FClientType m_client_type = A2FClientType::eA2FControllerClient;

    std::unique_ptr<IA2FClient> m_a2f_client;

    std::vector<AnimDataFrame> m_frames;
    uint16_t m_framerate = DEFAULT_FRAMERATE;
    long long m_last_updated = 0l;

    AceFaceParameters m_face_params;
    AceEmotionState m_emotion_state;
    AceEmotionParameters m_emotion_params;
    std::vector<KEY_VALUE> m_blendshape_multipliers;
    std::vector<KEY_VALUE> m_blendshape_offsets;

    std::unique_ptr<IA2FClient> InitializeClient(std::shared_ptr<grpc::Channel> channel);
    AceClientResult SendAudioWithParameters(
        std::vector<int16_t> const &audio_samples,
        std::vector<AnimDataFrame> *out_animation_frames
    );
    void SetClientParamaters(IA2FClient &client);

    // utilities
    size_t GetValidFrameIndex(size_t frame_index, Infinity post_infinity);

};

long long GetTick();
int FindPairIndex(std::string const &key, std::vector<KEY_VALUE> const &vec);
bool IsPrefix(std::string_view prefix, std::string_view full);

} // namespace mace
