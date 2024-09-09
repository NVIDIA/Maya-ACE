// SPDX-FileCopyrightText: Copyright (c) 2023-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#include "animation.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <optional>
#include <iostream>
#include <system_error>
#include <limits>
#include <vector>
#include <thread>

#include "frame_receiver.h"
#include "logger.h"
#include "parameters.h"

#pragma warning(disable : 4244)


namespace mace {

    long long GetCurrentTime() {
        std::chrono::high_resolution_clock::time_point t = std::chrono::steady_clock::now();
        return t.time_since_epoch().count();
    }

    AnimationClient::AnimationClient() {
        // LOG_DEBUG("AnimationClient::AnimationClient()");
    }

    AnimationClient::~AnimationClient() {
        // LOG_DEBUG("AnimationClient::~AnimationClient()");
        Destroy();
    }

    void AnimationClient::Destroy() {
    }

    bool AnimationClient::HasAnimation(float seconds) {
        return HasAnimation(GetFrameIndex(seconds));
    }

    bool AnimationClient::HasAnimation(size_t frame_index) {
        return frames.size() > frame_index;
    }

    size_t AnimationClient::GetFrameIndex(float seconds) {
        size_t frame_index = (size_t)std::ceil(seconds * framerate);
        return frame_index;
    }

    size_t AnimationClient::GetFramesCount() {
        return frames.size();
    }

    float AnimationClient::GetAnimationLength() {
        return frames.size() / (float) framerate;
    }

    AnimDataFrame AnimationClient::GetFrame(size_t frame_index) {
        if (frame_index >= frames.size()) {
            // index out of range
            return AnimDataFrame();
        }
        return frames[frame_index];
    }

    size_t AnimationClient::AddFrame(AnimDataFrame &frame) {
        frames.push_back(frame);
        return frames.size();
    }

    size_t AnimationClient::RemoveFrames(size_t first, size_t last) {
        if (first > frames.size()) {
            return 0;
        }
        size_t last_limit = std::min(frames.size(), last);
        frames.erase(frames.begin() + first, frames.begin() + last_limit);
        return last_limit - first;
    }

    size_t AnimationClient::InsertFrame(size_t after, AnimDataFrame &frame) {
        if (after > frames.size()) {
            return -1;
        }
        frames.insert(frames.begin() + after, frame);
        return frames.size();
    }

    size_t AnimationClient::ReplaceFrame(size_t frame_index, AnimDataFrame &frame) {
        if (frame_index >= frames.size()) {
            return -1;
        }
        frames[frame_index] = frame;
        return frames.size();
    }

    std::vector<float> AnimationClient::GetBlendshapeWeights(float seconds, Infinity postinfinity) {
        float frame_number = seconds * framerate;
        size_t right = std::ceil(frame_number);
        size_t left = std::floor(frame_number);

        std::vector<float> weights_r = GetBlendshapeWeights(right, postinfinity);

        if (right == left) {
            return weights_r;
        }
        // else
        float w1 = frame_number - left;
        std::vector<float> weights_l = GetBlendshapeWeights(left, postinfinity);

        std::vector<float> result;
        for (int i=0; i < weights_r.size(); i++) {
            result.push_back(weights_r[i] * w1 + weights_l[i] * (1.0 - w1));
        }
        return result;
    }

    std::vector<float> AnimationClient::GetBlendshapeWeights(size_t frame_index, Infinity postinfinity) {
        size_t valid_frame_idx = getValidFrameIndex(frame_index, postinfinity);
        if (valid_frame_idx < 0) {
            return {};
        }
        AnimDataFrame frame = GetFrame(valid_frame_idx);
        return frame.blend_shape_weights;
    }

    std::vector<float> AnimationClient::GetEmotionState(size_t frame_index, Infinity postinfinity) {
        size_t valid_frame_idx = getValidFrameIndex(frame_index, postinfinity);
        if (valid_frame_idx < 0) {
            return {};
        }
        AnimDataFrame frame = GetFrame(valid_frame_idx);
        return frame.emotion_state;
    }

    size_t AnimationClient::getValidFrameIndex(size_t frame_index, Infinity postinfinity) {
        size_t frame_count = GetFramesCount();
        // adjust frame index
        if (postinfinity == Constant) {
            frame_index = std::min(frame_index, frame_count - 1);
        }
        else if (postinfinity == Cycle) {
            while (frame_index >= frame_count) {
                frame_index -= frame_count;
            }
        }
        else if (frame_index > frame_count) {
            return -1;
        }
        return frame_index;
    }

    std::vector<std::string> AnimationClient::GetBlendshapeNames() {
        if (GetFramesCount() < 1) {
            return {};
        }
        AnimDataFrame frame = GetFrame(0);
        return frame.blend_shape_names;
    }

    std::vector<std::string> AnimationClient::GetEmotionStateNames() {
        if (GetFramesCount() < 1) {
            return {};
        }
        AnimDataFrame frame = GetFrame(0);
        return frame.emotion_state_names;
    }

    long long AnimationClient::GetLastUpdated() {
        return lastUpdated;
    }

    AceClientStatus AnimationClient::UpdateAnimation(std::vector<int16_t> const &samples) {
        // TODO: thread lock and release
        frames.clear();
        AceClientStatus result = RequestAnimation(samples, &frames);
        if (result != AceClientStatus::OK) {
            LOG_ERROR("Error while updating animation: " << result);
            return result;
        }

        // set updated tick to check last-updated
        lastUpdated = GetCurrentTime();

        return result;
    }

    AceClientStatus AnimationClient::RequestAnimation(
        std::vector<int16_t> const &samples,
        std::vector<AnimDataFrame> *frames
    ) {
        /*This is a blocking ace animation communicator.*/

        // establish connection to a2f controller using grpc client
        std::string address = GetNetworkAddress();
        std::shared_ptr<grpc::Channel> channel = CreateChannel(address);

        std::string apiKey = GetAPIKey();
        std::string functionId = GetFunctionId();
        AceClientStatus status = A2FControllerHealthCheck(channel, apiKey, functionId);
        if (status != AceClientStatus::OK) {
            return status;
        }
        LOG_DEBUG("Connection secured(using https): " << isConnectionSecured());

        std::unique_ptr<A2FControllerClient> a2f_client(
            new A2FControllerClient(channel, apiKey, functionId));

        // set parameters
        FetchClientParameters(*a2f_client);

        // send audio samples to a2f controller and retreive blendshape frames etc
        LOG_INFO("Sending " << samples.size() << " audio samples.");
        return a2f_client->ProcessAudioStream(
            samples.data(), samples.size(), emotionState, frames);
    }

    std::shared_ptr<grpc::Channel> AnimationClient::CreateChannel(std::string address) {
        // establish connection to a2f controller using grpc client
        std::shared_ptr<grpc::Channel> channel;
        if (isConnectionSecured()) {
            grpc::SslCredentialsOptions ssl_opts;
            auto creds = grpc::SslCredentials(ssl_opts);
            channel = grpc::CreateChannel(address, creds);
        }
        else {
            channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
        }
        return channel;
    }

    void AnimationClient::FetchClientParameters(A2FControllerClient &a2f_client) {
        /// face parameters
        for (auto const& [key, val]: faceParameters.GetParameterMap()) {
            a2f_client.SetFaceParam(key, val);
        }

        /// emotion parameters
        EmotionPostProcessingParameters emotionPostProcessingParams;
        emotionPostProcessingParams.set_emotion_contrast(emotionParameters.emotion_contrast);
        emotionPostProcessingParams.set_live_blend_coef(emotionParameters.live_blend_coef);
        emotionPostProcessingParams.set_enable_preferred_emotion(emotionParameters.enable_preferred_emotion);
        emotionPostProcessingParams.set_preferred_emotion_strength(emotionParameters.preferred_emotion_strength);
        emotionPostProcessingParams.set_emotion_strength(emotionParameters.emotion_strength);
        emotionPostProcessingParams.set_max_emotions(emotionParameters.max_emotions);
        a2f_client.SetEmotionPostProcessingParams(emotionPostProcessingParams);

        /// blendshape parameters
        for (auto entry: blendshapeMultipliers) {
            a2f_client.SetBlendshapeMultiplier(entry.first.c_str(), entry.second);
        }
        for (auto entry: blendshapeOffsets) {
            a2f_client.SetBlendshapeOffset(entry.first.c_str(), entry.second);
        }
    }

    bool AnimationClient::SetFaceParameters(AceFaceParameters const &new_parameters) {
        faceParameters = new_parameters;
        return true;
    }

    AceFaceParameters const AnimationClient::GetFaceParameters() {
        return faceParameters;
    }

    std::vector<char *> const AnimationClient::ListFaceParameters() {
        std::vector<char *> params;
        for (auto const& [key, val]: faceParameters.GetParameterMap()) {
            params.push_back(key);
        }
        return params;
    }

    bool AnimationClient::SetEmotionParameters(AceEmotionParameters const &new_parameters) {
        emotionParameters = new_parameters;
        return true;
    }

    AceEmotionParameters const AnimationClient::GetEmotionParameters() {
        return emotionParameters;
    }

    bool AnimationClient::SetEmotionState(AceEmotionState const &new_emotions) {
        emotionState = new_emotions;
        return true;
    }

    AceEmotionState const AnimationClient::GetEmotionState() {
        return emotionState;
    }

    int AnimationClient::SetBlendshapeMultiplier(std::string const &key, float value) {
        int pos = findKeyIndex(key, blendshapeMultipliers);
        if (pos >= 0) {
            blendshapeMultipliers[pos].second = value;
        }
        else {
            pos = blendshapeMultipliers.size();
            blendshapeMultipliers.push_back(KEY_VALUE(key, value));
        }
        return pos;
    }

    int AnimationClient::RemoveBlendshapeMultiplier(std::string const &key) {
        int pos = findKeyIndex(key, blendshapeMultipliers);
        if (pos >= 0) {
            blendshapeMultipliers.erase(blendshapeMultipliers.begin() + pos);
        }
        return pos;
    }

    void AnimationClient::ClearBlendshapeMultiplier() {
        blendshapeMultipliers.clear();
    }

    std::map<std::string, float> const AnimationClient::GetBlendshapeMultipliers() {
        std::map<std::string, float> multipliers;
        for (auto entry: blendshapeMultipliers) {
            multipliers[entry.first] = entry.second;
        }
        return multipliers;
    }

    std::vector<std::string> const AnimationClient::GetBlendshapeMultiplierKeys() {
        std::vector<std::string> keys;
        for (auto entry: blendshapeMultipliers) {
            keys.push_back(entry.first);
        }
        return keys;
    }

    std::vector<float> const AnimationClient::GetBlendshapeMultiplierValues() {
        std::vector<float> values;
        for (auto entry: blendshapeMultipliers) {
            values.push_back(entry.second);
        }
        return values;
    }

    int AnimationClient::SetBlendshapeOffset(std::string const &key, float value) {
        int pos = findKeyIndex(key, blendshapeOffsets);
        if (pos >= 0) {
            blendshapeOffsets[pos].second = value;
        }
        else {
            pos = blendshapeOffsets.size();
            blendshapeOffsets.push_back(KEY_VALUE(key, value));
        }
        return pos;
    }

    int AnimationClient::RemoveBlendshapeOffset(std::string const &key) {
        int pos = findKeyIndex(key, blendshapeOffsets);
        if (pos >= 0) {
            blendshapeOffsets.erase(blendshapeOffsets.begin() + pos);
        }
        return pos;
    }

    void AnimationClient::ClearBlendshapeOffset() {
        blendshapeOffsets.clear();
    }

    std::map<std::string, float> const AnimationClient::GetBlendshapeOffsets() {
        std::map<std::string, float> multipliers;
        for (auto entry: blendshapeOffsets) {
            multipliers[entry.first] = entry.second;
        }
        return multipliers;
    }

    std::vector<std::string> const AnimationClient::GetBlendshapeOffsetKeys() {
        std::vector<std::string> keys;
        for (auto entry: blendshapeOffsets) {
            keys.push_back(entry.first);
        }
        return keys;
    }

    std::vector<float> const AnimationClient::GetBlendshapeOffsetValues() {
        std::vector<float> values;
        for (auto entry: blendshapeOffsets) {
            values.push_back(entry.second);
        }
        return values;
    }

    int AnimationClient::findKeyIndex(std::string const &key, std::vector<KEY_VALUE> const &vec) {
        for (int i = 0; i < vec.size(); i++) {
            auto entry = vec[i];
            if (entry.first == key) {
                return i;
            }
        }
        return -1;
    }

    AceClientStatus AnimationClient::SetUrl(std::string const &newUrl) {
        if ((newUrl.length() >= 7 && newUrl.substr(0, 7) == "http://") ||
            (newUrl.length() >= 8 && newUrl.substr(0, 8) == "https://")) {
            _url = newUrl;
            return AceClientStatus::OK;
        }
        return AceClientStatus::ERROR_INVALID_INPUT;
    }

    std::string const AnimationClient::GetUrl() {
        return _url;
    }

    std::string const AnimationClient::GetNetworkAddress() {
        // remove the protocol prefix
        std::string networkAddress = _url;
        if (networkAddress.length() >= 7 && networkAddress.substr(0, 7) == "http://") {
            networkAddress.erase(0, 7);
        } else if (networkAddress.length() >= 8 && _url.substr(0, 8) == "https://") {
            networkAddress.erase(0, 8);
        }
        return networkAddress;
    }

    bool AnimationClient::isConnectionSecured() {
        // return true if _url starts with https://
        return (_url.length() >= 8 && _url.substr(0, 8) == "https://");
    }

    AceClientStatus AnimationClient::SetAPIKey(std::string const &newApiKey) {
        _apiKey = newApiKey;
        return AceClientStatus::OK;
    }

    std::string const AnimationClient::GetAPIKey() {
        if (_apiKey.rfind("$", 0) == 0) {
            // read key from the environment variable
            std::string key = _apiKey.substr(1);
            auto val = std::getenv(key.c_str());
            if (val != NULL) {
                return std::string(val);
            }
            else {
                return std::string("");
            }
        }
        return _apiKey;
    }

    AceClientStatus AnimationClient::SetFunctionId(std::string const &newFunctionId) {
        _functionId = newFunctionId;
        return AceClientStatus::OK;
    }

    std::string const AnimationClient::GetFunctionId() {
        return _functionId;
    }

} // namespace mace
