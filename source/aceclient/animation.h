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
#include <system_error>
#include <vector>

#include "a2f_controller_client.h"
#include "aceclient.h"

#include "frame_receiver.h"
#include "parameters.h"

#define KEY_VALUE std::pair<std::string, float>

namespace mace {

const uint16_t DEFAULT_FRAMERATE = 30;

enum Infinity {
    Constant, Cycle,
};

long long GetCurrentTime();

class AnimationClient {
public:
    AnimationClient();
    ~AnimationClient();

    // Main Param Setters
    bool SetFaceParameters(AceFaceParameters const &new_parameters);
    AceFaceParameters const GetFaceParameters();
    std::vector<char *> const ListFaceParameters();

    bool SetEmotionParameters(AceEmotionParameters const &new_parameters);
    AceEmotionParameters const GetEmotionParameters();

    bool SetEmotionState(AceEmotionState const &new_emotions);
    AceEmotionState const GetEmotionState();

    int SetBlendshapeMultiplier(std::string const &key, float value=1.0f);
    int RemoveBlendshapeMultiplier(std::string const &key);
    void ClearBlendshapeMultiplier();
    std::map<std::string, float> const GetBlendshapeMultipliers();
    std::vector<std::string> const GetBlendshapeMultiplierKeys();
    std::vector<float> const GetBlendshapeMultiplierValues();

    int SetBlendshapeOffset(std::string const &key, float value=1.0f);
    int RemoveBlendshapeOffset(std::string const &key);
    void ClearBlendshapeOffset();
    std::map<std::string, float> const GetBlendshapeOffsets();
    std::vector<std::string> const GetBlendshapeOffsetKeys();
    std::vector<float> const GetBlendshapeOffsetValues();


    // Main communication triggers
    AceClientStatus RequestAnimation(
        std::vector<int16_t> const &samples,
        std::vector<AnimDataFrame> *frames
    );
    AceClientStatus UpdateAnimation(
        std::vector<int16_t> const &samples
    );
    long long GetLastUpdated();
    void FetchClientParameters(A2FControllerClient &client);

    // Access to the animation frames
    // TODO: consider splitting these to a separate class
    std::vector<float> GetBlendshapeWeights(float seconds, Infinity postinfinity=Constant);
    std::vector<float> GetBlendshapeWeights(size_t frame_index, Infinity postinfinity=Constant);
    std::vector<std::string> GetBlendshapeNames();
    std::vector<float> GetEmotionState(size_t frame_index, Infinity postinfinity=Constant);
    std::vector<std::string> GetEmotionStateNames();

    bool HasAnimation(float seconds=0.0f);
    bool HasAnimation(size_t frame_index=0);

    size_t GetFrameIndex(float seconds);
    size_t GetFramesCount();
    float GetAnimationLength();
    AnimDataFrame GetFrame(size_t frame_index);
    size_t AddFrame(AnimDataFrame &frame);
    size_t RemoveFrames(size_t first, size_t last);
    size_t InsertFrame(size_t after, AnimDataFrame &frame);
    size_t ReplaceFrame(size_t frame_index, AnimDataFrame &frame);

    AceClientStatus SetUrl(std::string const &newUrl);
    std::string const GetUrl();
    AceClientStatus SetAPIKey(std::string const &newApiKey);
    std::string const GetAPIKey();
    AceClientStatus SetFunctionId(std::string const &newFunctionId);
    std::string const GetFunctionId();

    bool IsClientCreated();

    // Destroy
    void Destroy();

    // Non-API

protected:
    std::string _url = "https://grpc.nvcf.nvidia.com:443";
    // a secret key obtained from  https://build.nvidia.com/nvidia/audio2face/api
    std::string _apiKey = "";
    // vary for different models and configs. obtained from https://build.nvidia.com/nvidia/audio2face/api
    std::string _functionId = "462f7853-60e8-474a-9728-7b598e58472c";
    uint16_t framerate = DEFAULT_FRAMERATE;
    long long lastUpdated = 0l;

    std::vector<AnimDataFrame> frames;

    AceFaceParameters faceParameters;
    AceEmotionState emotionState;
    AceEmotionParameters emotionParameters;

    std::vector<KEY_VALUE> blendshapeMultipliers;
    std::vector<KEY_VALUE> blendshapeOffsets;

    int findKeyIndex(std::string const &key, std::vector<KEY_VALUE> const &vec);
    size_t getValidFrameIndex(size_t frame_index, Infinity postinfinity);
    std::string const GetNetworkAddress();
    bool isConnectionSecured();
    std::shared_ptr<grpc::Channel> CreateChannel(std::string address);
};
} // namespace mace
