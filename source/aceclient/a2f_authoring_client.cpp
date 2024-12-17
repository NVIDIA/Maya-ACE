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
#include "a2f_authoring_client.h"

#include "logger.h"

using nvidia_ace::a2f_authoring::v1::AudioClip;
using nvidia_ace::a2f_authoring::v1::AudioClipHandle;
using nvidia_ace::a2f_authoring::v1::FacePoseRequest;
using nvidia_ace::a2f_authoring::v1::BlendShapeData;
using nvidia_ace::audio::v1::AudioHeader_AudioFormat_AUDIO_FORMAT_PCM;

const size_t SAMPLE_RATE = 16000;
const size_t FPS = 30;

#define CHECK_NO_ERROR(expr) \
    do { \
        AceClientResult result = (expr);     \
        if (result.status != AceClientStatus::OK) { \
            return result; \
        } \
    } while(0)

namespace mace {

A2FAuthoringClient::A2FAuthoringClient(
    std::shared_ptr<grpc::Channel> channel, std::string api_key, std::string function_id
) : m_stub(A2FAuthoringService::NewStub(channel).release()), m_api_key(api_key), m_function_id(function_id) {}

A2FAuthoringClient::A2FAuthoringClient(
    std::shared_ptr<A2FAuthoringService::StubInterface> stub, std::string api_key, std::string function_id
) : m_stub(stub), m_api_key(api_key), m_function_id(function_id) {}

void A2FAuthoringClient::SetFaceParam(const std::string key, float val) {
    m_face_params[key] = val;
}

void A2FAuthoringClient::SetEmotionPostProcessingParams(EmotionPostProcessingParameters &param)
{
    m_emotion_params = param;
}

void A2FAuthoringClient::SetBlendshapeMultiplier(const std::string key, float value) {
    m_blendshape_mutlipliers[key] = value;
}

void A2FAuthoringClient::SetBlendshapeOffset(const std::string key, float value) {
    m_blendshape_offsets[key] = value;
}

/**
 * This method uploads the audio clip to authoring service, store the clip id and return placeholder frames.
 * The actual frame will be requested when we invoke this->FetchFrame()
 */
AceClientResult A2FAuthoringClient::SendAudio(
    std::vector<int16_t> const &samples, AceEmotionState input_emotion_state, std::vector<AnimDataFrame> *out_frames)
{
    std::string clipId;
    std::vector<std::string> blendshapeNames;
    CHECK_NO_ERROR(UploadAudioClip(samples.data(), samples.size(), clipId, blendshapeNames));

    // save the audio clip id to reuse during session
    m_audio_clip_id = clipId;

    const float audioLength = (float)samples.size() / (float)SAMPLE_RATE;
    const double dt = 1.0/FPS;
    for(double t=0;t<audioLength;t += dt) {
        // create empty anim frames at this stage. will update frames individually by on-demand for authoring workflow
        AnimDataFrame animDataFrame;
        animDataFrame.timestamp = t;

        animDataFrame.blend_shape_names = blendshapeNames;
        for(int i=0; i<blendshapeNames.size(); ++i) {
            animDataFrame.blend_shape_weights.push_back(0.0f);
        }

        for(auto const& [name, val] : input_emotion_state.GetParameterMap()) {
            animDataFrame.emotion_state_names.push_back(name);
        }

        out_frames->push_back(animDataFrame);
    }
    return AceClientResult{AceClientStatus::OK};
}

AceClientResult A2FAuthoringClient::FetchFrame(
        float timestamp, AceEmotionState input_emotion_state, AnimDataFrame *out_frame)
{
    if (m_audio_clip_id == "") {
        // not started session
        return AceClientResult{AceClientStatus::OK_NO_MORE_FRAMES, "No more frames!"};
    }
    LOG_DEBUG("Requesting blendshape at timestamp: " << timestamp);

    CHECK_NO_ERROR(GetAvatarFacePose(
        m_audio_clip_id,
        input_emotion_state,
        (float)timestamp,
        out_frame->blend_shape_weights,
        out_frame->emotion_state
    ));

    if (out_frame->blend_shape_weights.size() != out_frame->blend_shape_names.size()) {
        std::string msg = "The number of blendshape weights ";
        msg += out_frame->blend_shape_weights.size();
        msg += " doesn't match the number of blendshape names: ";
        msg += out_frame->blend_shape_names.size();
        LOG_DEBUG(msg);
    }

    return AceClientResult{AceClientStatus::OK};
}

AceClientResult A2FAuthoringClient::UploadAudioClip(
    const int16_t *samples, size_t sample_count, std::string& out_clip_id, std::vector<std::string>& out_blendshape_names)
{
    grpc::ClientContext context;
    if (!m_api_key.empty()) {
      context.AddMetadata("authorization", "Bearer " + m_api_key);
    }
    if (!m_function_id.empty()) {
      context.AddMetadata("function-id", m_function_id);
    }

    AudioClip request;
    auto header = request.mutable_audio_header();
    header->set_audio_format(AudioHeader_AudioFormat_AUDIO_FORMAT_PCM);
    header->set_channel_count(1);
    header->set_samples_per_second(SAMPLE_RATE);
    header->set_bits_per_sample(16);

    const size_t totalBufferSize = sample_count * sizeof(int16_t);
    const uint8_t* audioBufferPtr = reinterpret_cast<const uint8_t*>(samples);
    request.set_content(audioBufferPtr, totalBufferSize);

    AudioClipHandle response;
    grpc::Status status = m_stub->UploadAudioClip(&context, request, &response);
    if (!status.ok()) {
        LOG_ERROR("UploadAudioClip error:  grpc error message: " << status.error_message());
        return AceClientResult{AceClientStatus::ERROR_UNKNOWN, status.error_message()};
    }

    out_clip_id = response.audio_clip_id();
    auto blendshape_names = response.blendshape_names();
    out_blendshape_names = std::vector<std::string>(blendshape_names.begin(), blendshape_names.end());

    return AceClientResult{AceClientStatus::OK};
}

AceClientResult A2FAuthoringClient::GetAvatarFacePose(
    std::string& audio_clip_id,
    AceEmotionState input_emotion_state,
    float timestamp,
    std::vector<float>& out_blendshapes,
    std::vector<float>& out_emotion_states)
{
    grpc::ClientContext context;
    if (!m_api_key.empty()) {
      context.AddMetadata("authorization", "Bearer " + m_api_key);
    }
    if (!m_function_id.empty()) {
      context.AddMetadata("function-id", m_function_id);
    }

    FacePoseRequest request;
    request.set_audio_hash(audio_clip_id);
    request.set_time_stamp(timestamp);
    auto faceParams = request.mutable_face_params();
    auto floatParams = faceParams->mutable_float_params();
    (*floatParams) = m_face_params;
    // authoring service require this field but we didn't expose this param
    (*floatParams)["blinkOffset"] = 0.0f;
    // temporal smoothing must be turned off when requesting authoring service
    (*floatParams)["lowerFaceSmoothing"] = 0.0f;
    // temporal smoothing must be turned off when requesting authoring service
    (*floatParams)["upperFaceSmoothing"] = 0.0f;

    auto emotionParams = request.mutable_emotion_pp_params();
    (*emotionParams) = m_emotion_params;
    // temporal smoothing must be turned off when requesting authoring service
    emotionParams->set_live_blend_coef(0.0f);

    auto bsParams = request.mutable_blendshape_params();
    auto bsMultipliers = bsParams->mutable_bs_weight_multipliers();
    auto bsOffsets = bsParams-> mutable_bs_weight_offsets();
    (*bsMultipliers) = m_blendshape_mutlipliers;
    (*bsOffsets) = m_blendshape_offsets;

    auto preferredEmotions = request.mutable_preferred_emotions();
    preferredEmotions->insert({"amazement", input_emotion_state.amazement});
    preferredEmotions->insert({"anger", input_emotion_state.anger});
    preferredEmotions->insert({"cheekiness", input_emotion_state.cheekiness});
    preferredEmotions->insert({"disgust", input_emotion_state.disgust});
    preferredEmotions->insert({"fear", input_emotion_state.fear});
    preferredEmotions->insert({"grief", input_emotion_state.grief});
    preferredEmotions->insert({"joy", input_emotion_state.joy});
    preferredEmotions->insert({"outofbreath", input_emotion_state.outofbreath});
    preferredEmotions->insert({"pain", input_emotion_state.pain});
    preferredEmotions->insert({"sadness", input_emotion_state.sadness});

    BlendShapeData response;
    grpc::Status status = m_stub->GetAvatarFacePose(&context, request, &response);
    if (!status.ok()) {
        LOG_ERROR("GetAvatarFacePose error: grpc error message: " << status.error_message());
        return AceClientResult{AceClientStatus::ERROR_UNKNOWN, status.error_message()};
    }
    if (response.blendshapes_size() == 0) {
        LOG_ERROR("GetAvatarFacePose error: The server returned zero blendshape weights."
            " This could be caused by an internal server error or incorrect input data.");
        return AceClientResult{AceClientStatus::ERROR_INVALID_INPUT, "The server returned zero blendshape weights"};
    }

    auto blendshapes = response.blendshapes();
    out_blendshapes = std::vector<float>(blendshapes.begin(), blendshapes.end());

    std::vector<float> emotionStates;
    auto emotionMap = response.emotions();
    for(auto const& [name, val] : input_emotion_state.GetParameterMap()) {
        if (emotionMap.find(name) != emotionMap.end()) {
            emotionStates.push_back(emotionMap[name]);
        } else {
            emotionStates.push_back(0.0f);
        }
    }
    out_emotion_states = emotionStates;

    return AceClientResult{AceClientStatus::OK};
}

} // namespace mace
