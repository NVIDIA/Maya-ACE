// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#include <chrono>
#include <optional>
#include <iostream>
#include <vector>

#include "frame_receiver.h"
#include "grpc_client.h"
#include "logger.h"
#include "parameters.h"
#include "utility.h"

#pragma warning(disable : 4244)


namespace mace {

bool NetworkedAnimation::HasAnimation(float seconds) {
    return HasAnimation(GetFrameIndex(seconds));
}

bool NetworkedAnimation::HasAnimation(size_t frame_index) {
    return m_frames.size() > frame_index;
}

size_t NetworkedAnimation::GetFrameIndex(float seconds) {
    size_t frame_index = (size_t)std::ceil(seconds * m_framerate);
    return frame_index;
}

size_t NetworkedAnimation::GetFramesCount() {
    return m_frames.size();
}

float NetworkedAnimation::GetAnimationLength() {
    return m_frames.size() / (float) m_framerate;
}

AnimDataFrame NetworkedAnimation::GetFrame(size_t frame_index) {
    if (frame_index >= m_frames.size()) {
        // index out of range
        return AnimDataFrame();
    }
    return m_frames[frame_index];
}

size_t NetworkedAnimation::AddFrame(AnimDataFrame &frame) {
    m_frames.push_back(frame);
    return m_frames.size();
}

size_t NetworkedAnimation::RemoveFrames(size_t first, size_t last) {
    if (first > m_frames.size()) {
        return 0;
    }
    size_t last_limit = std::min(m_frames.size(), last);
    m_frames.erase(m_frames.begin() + first, m_frames.begin() + last_limit);
    return last_limit - first;
}

size_t NetworkedAnimation::InsertFrame(size_t after, AnimDataFrame &frame) {
    if (after > m_frames.size()) {
        return -1;
    }
    m_frames.insert(m_frames.begin() + after, frame);
    return m_frames.size();
}

size_t NetworkedAnimation::ReplaceFrame(size_t frame_index, AnimDataFrame &frame) {
    if (frame_index >= m_frames.size()) {
        return -1;
    }
    m_frames[frame_index] = frame;
    return m_frames.size();
}

std::vector<float> NetworkedAnimation::GetBlendshapeWeights(
    float seconds, Infinity post_infinity)
/* Read blendshape weights from frames, and interpolate for the specific time.

    Returns an empty vector if there's no blendshape weights available.
*/
{
    float frame_number = seconds * m_framerate;
    size_t right = std::ceil(frame_number);
    size_t left = std::floor(frame_number);

    std::vector<float> weights_r = GetBlendshapeWeights(right, post_infinity);

    if (right == left) {
        return weights_r;
    }
    // else

    std::vector<float> weights_l = GetBlendshapeWeights(left, post_infinity);
    if (weights_l.size() != weights_r.size()) {
        return {};
    }

    float w1 = frame_number - left;
    std::vector<float> result;
    for (int i=0; i < weights_r.size(); i++) {
        result.push_back(weights_r[i] * w1 + weights_l[i] * (1.0 - w1));
    }
    return result;
}

std::vector<float> NetworkedAnimation::GetBlendshapeWeights(
    size_t frame_index, Infinity post_infinity)
/* Read blendshape weights from a specific frame.
    Take alternative frame per post_infinity.

    Returns an empty vector if there's no blendshape weights available.
*/
{
    size_t valid_frame_idx = GetValidFrameIndex(frame_index, post_infinity);
    if (valid_frame_idx < 0) {
        return {};
    }
    AnimDataFrame frame = GetFrame(valid_frame_idx);
    return frame.blend_shape_weights;
}

std::vector<float> NetworkedAnimation::GetEmotionState(
    size_t frame_index, Infinity post_infinity)
{
    size_t valid_frame_idx = GetValidFrameIndex(frame_index, post_infinity);
    if (valid_frame_idx < 0) {
        return {};
    }
    AnimDataFrame frame = GetFrame(valid_frame_idx);
    return frame.emotion_state;
}

size_t NetworkedAnimation::GetValidFrameIndex(
    size_t frame_index, Infinity post_infinity)
{
    size_t frame_count = GetFramesCount();
    // adjust frame index
    if (post_infinity == Constant) {
        frame_index = std::min(frame_index, frame_count - 1);
    }
    else if (post_infinity == Cycle) {
        while (frame_index >= frame_count) {
            frame_index -= frame_count;
        }
    }
    else if (frame_index > frame_count) {
        return -1;
    }
    return frame_index;
}

std::vector<std::string> NetworkedAnimation::GetBlendshapeNames()
{
    if (GetFramesCount() < 1) {
        return {};
    }
    AnimDataFrame frame = GetFrame(0);
    return frame.blend_shape_names;
}

std::vector<std::string> NetworkedAnimation::GetEmotionStateNames()
{
    if (GetFramesCount() < 1) {
        return {};
    }
    AnimDataFrame frame = GetFrame(0);
    return frame.emotion_state_names;
}

long long NetworkedAnimation::GetLastUpdated()
{
    return m_last_updated;
}

AceClientResult NetworkedAnimation::ConnectAndSendAudio(std::vector<int16_t> const &audio_samples)
{
    // TODO: thread lock and release

    // establish connection to a2f controller using grpc client
    std::string address = GetAddress();
    std::shared_ptr<grpc::Channel> channel = CreateChannel(address);

    AceClientResult result = CheckServiceStatus(channel);
    if (result.status != AceClientStatus::OK) {
        return result;
    }
    LOG_DEBUG("Connection secured(using https): " << IsHttps(address));

    m_a2f_client = InitializeClient(channel);
    if (m_a2f_client == nullptr) {
        return AceClientResult{AceClientStatus::ERROR_INVALID_INPUT, "Failed to initialize client."};
    }

    m_frames.clear();
    result = SendAudioWithParameters(audio_samples, &m_frames);
    if (result.status != AceClientStatus::OK) {
        LOG_ERROR("Error while sending audio: status: " << result.status << ", message: " << result.message);
        return result;
    }

    // set updated tick to check last-updated
    m_last_updated = mace::GetTick();

    return result;
}

std::unique_ptr<IA2FClient> NetworkedAnimation::InitializeClient(std::shared_ptr<grpc::Channel> channel)
{
    std::string api_key = GetAPIKey();
    std::string function_id = GetFunctionId();
    return std::make_unique<A2FControllerClient>(channel, api_key, function_id);
}

AceClientResult NetworkedAnimation::SendAudioWithParameters(
    std::vector<int16_t> const &audio_samples, std::vector<AnimDataFrame> *out_animation_frames)
{
    if (m_a2f_client == nullptr) {
        return AceClientResult{AceClientStatus::ERROR_CONNECTION, "Connection hasn't been established."};
    }

    // set parameters
    SetClientParamaters(*m_a2f_client);

    // send audio samples to a2f controller and retreive blendshape frames etc
    LOG_INFO("Sending " << audio_samples.size() << " audio samples.");
    return m_a2f_client->SendAudio(audio_samples, m_emotion_state, out_animation_frames);
}

AceClientResult NetworkedAnimation::CheckServiceStatus(std::shared_ptr<grpc::Channel> channel)
{
    if (channel == nullptr) {
        std::string address = GetAddress();
        channel = CreateChannel(address);
    }
    std::string api_key = GetAPIKey();
    std::string function_id = GetFunctionId();

    return A2FHealthCheck(channel, api_key, function_id);
}

AceClientResult NetworkedAnimation::FetchAnimationFrame(size_t frame_index)
{
    if (m_a2f_client == nullptr) {
        return AceClientResult{AceClientStatus::OK_NO_MORE_FRAMES, "No more frames!"};
    }

    SetClientParamaters(*m_a2f_client);

    AnimDataFrame frame = GetFrame(frame_index); // get cached frames

    /* Notes for m_a2f_client->FetchFrame

        for a2f controller client, all the frames are pre-fetched and stored in frames,
            so this function would do nothing
    */
    AceClientResult result = m_a2f_client->FetchFrame(frame.timestamp, m_emotion_state, &frame);

    ReplaceFrame(frame_index, frame);

    return result;
}

void NetworkedAnimation::SetClientParamaters(IA2FClient &client)
{
    /// face parameters
    for (auto const& [key, val]: m_face_params.GetParameterMap()) {
        client.SetFaceParam(key, val);
    }

    /// emotion parameters
    EmotionPostProcessingParameters emotionPostProcessingParams;
    emotionPostProcessingParams.set_emotion_contrast(m_emotion_params.emotion_contrast);
    emotionPostProcessingParams.set_live_blend_coef(m_emotion_params.live_blend_coef);
    emotionPostProcessingParams.set_enable_preferred_emotion(m_emotion_params.enable_preferred_emotion);
    emotionPostProcessingParams.set_preferred_emotion_strength(m_emotion_params.preferred_emotion_strength);
    emotionPostProcessingParams.set_emotion_strength(m_emotion_params.emotion_strength);
    emotionPostProcessingParams.set_max_emotions(m_emotion_params.max_emotions);
    client.SetEmotionPostProcessingParams(emotionPostProcessingParams);

    /// blendshape parameters
    for (auto entry: m_blendshape_multipliers) {
        client.SetBlendshapeMultiplier(entry.first, entry.second);
    }
    for (auto entry: m_blendshape_offsets) {
        client.SetBlendshapeOffset(entry.first, entry.second);
    }
}

bool NetworkedAnimation::SetFaceParameters(AceFaceParameters const &new_parameters) {
    m_face_params = new_parameters;
    return true;
}

AceFaceParameters const NetworkedAnimation::GetFaceParameters() {
    return m_face_params;
}

bool NetworkedAnimation::SetEmotionParameters(AceEmotionParameters const &new_parameters) {
    m_emotion_params = new_parameters;
    return true;
}

AceEmotionParameters const NetworkedAnimation::GetEmotionParameters() {
    return m_emotion_params;
}

bool NetworkedAnimation::SetEmotionState(AceEmotionState const &new_emotions) {
    m_emotion_state = new_emotions;
    return true;
}

AceEmotionState const NetworkedAnimation::GetEmotionState() {
    return m_emotion_state;
}

int NetworkedAnimation::SetBlendshapeMultiplier(std::string const &key, float value) {
    int pos = mace::FindPairIndex(key, m_blendshape_multipliers);
    if (pos >= 0) {
        m_blendshape_multipliers[pos].second = value;
    }
    else {
        pos = m_blendshape_multipliers.size();
        m_blendshape_multipliers.push_back(KEY_VALUE(key, value));
    }
    return pos;
}

int NetworkedAnimation::RemoveBlendshapeMultiplier(std::string const &key) {
    int pos = mace::FindPairIndex(key, m_blendshape_multipliers);
    if (pos >= 0) {
        m_blendshape_multipliers.erase(m_blendshape_multipliers.begin() + pos);
    }
    return pos;
}

void NetworkedAnimation::ClearBlendshapeMultiplier() {
    m_blendshape_multipliers.clear();
}

std::map<std::string, float> const NetworkedAnimation::GetBlendshapeMultipliers() {
    std::map<std::string, float> multipliers;
    for (auto entry: m_blendshape_multipliers) {
        multipliers[entry.first] = entry.second;
    }
    return multipliers;
}

std::vector<std::string> const NetworkedAnimation::GetBlendshapeMultiplierKeys() {
    std::vector<std::string> keys;
    for (auto entry: m_blendshape_multipliers) {
        keys.push_back(entry.first);
    }
    return keys;
}

std::vector<float> const NetworkedAnimation::GetBlendshapeMultiplierValues() {
    std::vector<float> values;
    for (auto entry: m_blendshape_multipliers) {
        values.push_back(entry.second);
    }
    return values;
}

int NetworkedAnimation::SetBlendshapeOffset(std::string const &key, float value) {
    int pos = mace::FindPairIndex(key, m_blendshape_offsets);
    if (pos >= 0) {
        m_blendshape_offsets[pos].second = value;
    }
    else {
        pos = m_blendshape_offsets.size();
        m_blendshape_offsets.push_back(KEY_VALUE(key, value));
    }
    return pos;
}

int NetworkedAnimation::RemoveBlendshapeOffset(std::string const &key) {
    int pos = mace::FindPairIndex(key, m_blendshape_offsets);
    if (pos >= 0) {
        m_blendshape_offsets.erase(m_blendshape_offsets.begin() + pos);
    }
    return pos;
}

void NetworkedAnimation::ClearBlendshapeOffset() {
    m_blendshape_offsets.clear();
}

std::map<std::string, float> const NetworkedAnimation::GetBlendshapeOffsets() {
    std::map<std::string, float> multipliers;
    for (auto entry: m_blendshape_offsets) {
        multipliers[entry.first] = entry.second;
    }
    return multipliers;
}

std::vector<std::string> const NetworkedAnimation::GetBlendshapeOffsetKeys() {
    std::vector<std::string> keys;
    for (auto entry: m_blendshape_offsets) {
        keys.push_back(entry.first);
    }
    return keys;
}

std::vector<float> const NetworkedAnimation::GetBlendshapeOffsetValues() {
    std::vector<float> values;
    for (auto entry: m_blendshape_offsets) {
        values.push_back(entry.second);
    }
    return values;
}

std::string const NetworkedAnimation::GetAddress() {
    return SanitizeString(m_address);
}

std::string const NetworkedAnimation::GetAPIKey() {
    std::string value = ResolveValue(m_api_key);
    return SanitizeString(value);
}

std::string const NetworkedAnimation::GetFunctionId() {
    std::string value = ResolveValue(m_function_id);
    return SanitizeString(value);
}

long long GetTick() {
    std::chrono::high_resolution_clock::time_point t = std::chrono::steady_clock::now();
    return t.time_since_epoch().count();
}

int FindPairIndex(std::string const &key, std::vector<KEY_VALUE> const &vec) {
    for (int i = 0; i < vec.size(); i++) {
        auto entry = vec[i];
        if (entry.first == key) {
            return i;
        }
    }
    return -1;
}

bool IsPrefix(std::string_view prefix, std::string_view full) {
    return full.substr(0, prefix.size()) == prefix;
}

} // namespace mace
