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
#include "a2f_controller_client.h"

#include <iostream>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "ace_grpc_cpp/nvidia_ace.emotion_aggregate.v1.pb.h"

#include "grpc_client.h"
#include "logger.h"

using nvidia_ace::controller::v1::AudioStream;
using nvidia_ace::controller::v1::AudioStreamHeader;
using nvidia_ace::audio::v1::AudioHeader;
using nvidia_ace::audio::v1::AudioHeader_AudioFormat_AUDIO_FORMAT_PCM;
using nvidia_ace::a2f::v1::FaceParameters;
using nvidia_ace::a2f::v1::BlendShapeParameters;
using nvidia_ace::a2f::v1::AudioWithEmotion;
using nvidia_ace::controller::v1::AnimationDataStream;
using nvidia_ace::emotion_aggregate::v1::EmotionAggregate;
using nvidia_ace::emotion_with_timecode::v1::EmotionWithTimeCode;
using nvidia_ace::status::v1::Status_Code;

typedef std::shared_ptr<grpc::ClientReaderWriterInterface<AudioStream, AnimationDataStream>> AudioAnimationStream;

#define CHECK_TRUE(expr, msg) \
    do { \
        if (!(expr)) { \
            LOG_ERROR("A2FControllerClient: " << msg); \
            return AceClientResult{AceClientStatus::ERROR_UNKNOWN, msg}; \
        } \
    } while(0)

const size_t SAMPLE_RATE = 16000;
const size_t CHUNK_SIZE = SAMPLE_RATE * sizeof(int16_t);


namespace {
// Increase the deadline in the gRPC context before calling stream->Write, effectively setting a timeout for stream writes.
bool WriteWithDeadline(grpc::ClientContext &context, AudioAnimationStream stream, const AudioStream &request) {
    mace::ExtendDeadline(context);
    return stream->Write(request);
}

// Increase the deadline in the gRPC context before calling stream->Read, effectively setting a timeout for stream reads.
bool ReadWithDeadline(grpc::ClientContext &context, AudioAnimationStream stream, AnimationDataStream *response) {
    mace::ExtendDeadline(context);
    return stream->Read(response);
}
}


namespace mace {

A2FControllerClient::A2FControllerClient(
    std::shared_ptr<grpc::Channel> channel, std::string api_key, std::string function_id
) : m_stub(A2FControllerService::NewStub(channel).release()), m_api_key(api_key), m_function_id(function_id) {}

A2FControllerClient::A2FControllerClient(
    std::shared_ptr<A2FControllerService::StubInterface> stub, std::string api_key, std::string function_id
) : m_stub(stub), m_api_key(api_key), m_function_id(function_id) {}

void A2FControllerClient::SetFaceParam(const std::string key, float val) {
    m_face_params[key] = val;
}

void A2FControllerClient::SetEmotionPostProcessingParams(EmotionPostProcessingParameters &param) {
    m_emotion_params = param;
}

void A2FControllerClient::SetBlendshapeMultiplier(const std::string key, float value) {
    m_blendshape_mutlipliers[key] = value;
}

void A2FControllerClient::SetBlendshapeOffset(const std::string key, float value) {
    m_blendshape_offsets[key] = value;
}

AceClientResult A2FControllerClient::SendAudio(
    std::vector<int16_t> const &samples, AceEmotionState input_emotion_state, std::vector<AnimDataFrame> *out_frames)
{
    grpc::ClientContext context;
    if (!m_api_key.empty()) {
      context.AddMetadata("authorization", "Bearer " + m_api_key);
    }
    if (!m_function_id.empty()) {
      context.AddMetadata("function-id", m_function_id);
    }
    AudioAnimationStream stream(m_stub->ProcessAudioStream(&context));

    LOG_DEBUG("A2FControllerClient: SendAudio Start");
    {
        // send header
        AudioStream message;
        AudioStreamHeader* stream_header = message.mutable_audio_stream_header();

        buildAudioStreamHeader(stream_header);

        CHECK_TRUE(WriteWithDeadline(context, stream, message), "Unable to write AudioStreamHeader.");
        LOG_DEBUG("A2FControllerClient: AudioStreamHeader has been written");
        LOG_DEBUG(stream_header);
    }
    {
        // send audio buffer
        const size_t totalBufferSize = samples.size() * sizeof(int16_t); // total buffer size in bytes
        const uint8_t* audioBufferPtr = reinterpret_cast<const uint8_t*>(samples.data());
        // Cutting the audio into reasonable sized chunks.
        for(size_t offset = 0; offset < totalBufferSize; offset += CHUNK_SIZE) {
            const size_t currentChunkSize = (offset + CHUNK_SIZE <= totalBufferSize) ? CHUNK_SIZE : (totalBufferSize - offset);
            AudioStream message;
            auto audioWithEmotion = message.mutable_audio_with_emotion();
            audioWithEmotion->set_audio_buffer(audioBufferPtr + offset, currentChunkSize);

            auto emotionWithTimeCode = audioWithEmotion->add_emotions();
            // time_code is set to the start timestamp of each chunk
            // in the future, if we have emotion key frame enabled,
            // we can cut the chunk according to the keyframe timestamp or a max chunk size
            emotionWithTimeCode->set_time_code((float)offset/(float)SAMPLE_RATE);
            auto emotion = emotionWithTimeCode->mutable_emotion();
            emotion->insert({"amazement", input_emotion_state.amazement});
            emotion->insert({"anger", input_emotion_state.anger});
            emotion->insert({"cheekiness", input_emotion_state.cheekiness});
            emotion->insert({"disgust", input_emotion_state.disgust});
            emotion->insert({"fear", input_emotion_state.fear});
            emotion->insert({"grief", input_emotion_state.grief});
            emotion->insert({"joy", input_emotion_state.joy});
            emotion->insert({"outofbreath", input_emotion_state.outofbreath});
            emotion->insert({"pain", input_emotion_state.pain});
            emotion->insert({"sadness", input_emotion_state.sadness});

            CHECK_TRUE(WriteWithDeadline(context, stream, message), "Unable to write AudioWithEmotion.");
        }
        LOG_DEBUG("A2FControllerClient: AudioWithEmotion has been written");
    }
    {
        // send end marker
        AudioStream message;
        message.mutable_end_of_audio();
        CHECK_TRUE(WriteWithDeadline(context, stream, message), "Unable to write EndOfAudio.");
        LOG_DEBUG("A2FControllerClient: EndOfAudio has been written");
    }
    CHECK_TRUE(stream->WritesDone(), "WritesDone failed.");

    // read response
    LOG_DEBUG("A2FControllerClient: Start to read response");
    AnimationDataStream response;

    // the header must be sent first
    LOG_DEBUG("A2FControllerClient: Reading response header");
    CHECK_TRUE(ReadWithDeadline(context, stream, &response), "Unable to read the response header");
    if (!response.has_animation_data_stream_header()) {
        // handle error
        std::string errorMessage = "Response header is not sent as the first response message";
        LOG_ERROR(errorMessage);
        stream->Finish();
        return AceClientResult{AceClientStatus::ERROR_UNEXPECTED_OUTPUT, errorMessage};
    }

    // parse response header
    LOG_DEBUG("A2FControllerClient: Received AnimationDataStreamHeader");
    std::vector<std::string> blendshape_names;
    auto responseHeader = response.animation_data_stream_header();
    if (responseHeader.has_skel_animation_header()) {
        auto skelAnimationHeader = responseHeader.skel_animation_header();
        blendshape_names.clear();
        for(int i=0;i<skelAnimationHeader.blend_shapes_size();++i) {
            blendshape_names.push_back(skelAnimationHeader.blend_shapes(i));
        }
    }

    // read response until end status is received
    while(ReadWithDeadline(context, stream, &response)) {
        if (response.has_animation_data()) {
            LOG_DEBUG("A2FControllerClient: Received AnimationData");
            std::vector<float> emotion_state;

            auto animationData = response.animation_data();
            // parse metadata (emotino_aggregation)
            auto metadata = animationData.metadata();
            const std::string EMOTION_AGGREGATE_KEY = "emotion_aggregate";
            if (metadata.find(EMOTION_AGGREGATE_KEY) != metadata.end()) {
                const google::protobuf::Any& any_value = metadata.at(EMOTION_AGGREGATE_KEY);
                if (any_value.Is<EmotionAggregate>()) {
                    EmotionAggregate emotionAggregate;
                    any_value.UnpackTo(&emotionAggregate);

                    if (emotionAggregate.a2f_smoothed_output_size() > 0) {
                        auto emotion = emotionAggregate.a2f_smoothed_output(0).emotion();
                        for(auto const& [name, val] : input_emotion_state.GetParameterMap()) {
                            if (emotion.find(name) != emotion.end()) {
                                emotion_state.push_back(emotion[name]);
                            } else {
                                emotion_state.push_back(0.0f);
                            }
                        }
                    }
                } else {
                    LOG_ERROR(
                        "A2FControllerClient: Expected EmotionAggregate in metadata[" << EMOTION_AGGREGATE_KEY
                        << " but got: " << any_value.type_url());
                }
            } else {
                // not all animation_data frame contains the emotion in metadata.
                LOG_DEBUG("A2FControllerClient: Key '" << EMOTION_AGGREGATE_KEY << "' is not found in metadata.");
            }

            if (animationData.has_audio()) {
                // the input audio buffer will be echo back from the server.
            }
            if (animationData.has_skel_animation()) {
                auto skelAnimation = animationData.skel_animation();
                // from the experiment there's only one element in this blend_shape_weights array.
                for(auto blend_shape_weights : skelAnimation.blend_shape_weights()) {
                    AnimDataFrame animDataFrame;
                    animDataFrame.timestamp = blend_shape_weights.time_code();
                    animDataFrame.blend_shape_names = blendshape_names;
                    for(auto const& [name, val] : input_emotion_state.GetParameterMap()) {
                        animDataFrame.emotion_state_names.push_back(name);
                    }
                    animDataFrame.emotion_state = emotion_state;
                    animDataFrame.blend_shape_weights = std::vector<float>(
                        blend_shape_weights.values().begin(), blend_shape_weights.values().end());
                    out_frames->push_back(animDataFrame);
                    LOG_DEBUG("A2FControllerClient: timestamp: " << animDataFrame.timestamp);
                }
            }
        } else if (response.has_event()) {
            LOG_DEBUG("A2FControllerClient: Received Event");
        } else if (response.has_status()) {
            // The status must be sent last and may be sent in between.
            // the comment in the proto definition didn't say how to check for the end of the response stream.
            // assuming the stream is terminated by ERROR or SUCCESS status code. INFO and WARNING might be sent in between.
            auto status = response.status();
            if (status.code() == Status_Code::Status_Code_ERROR) {
                LOG_ERROR("A2FControllerClient: Received Error Status Code: " << status.message());
                stream->Finish();
                return AceClientResult{AceClientStatus::ERROR_UNEXPECTED_OUTPUT, status.message()};
            }
            if (status.code() == Status_Code::Status_Code_SUCCESS) {
                LOG_DEBUG("A2FControllerClient: Received Success Status: " << status.message());
                break;
            }
            LOG_INFO("A2FControllerClient: Received Status (code: "
                << status.code()
                << ", message: "
                << status.message()
                << ")"
            );
        } else {
            // This should not happen. Likely theres a new type of message gets added. Please check the AnimationDataStream proto definition.
            LOG_DEBUG("A2FControllerClient: Received Unknown Response");
            return AceClientResult{AceClientStatus::ERROR_UNEXPECTED_OUTPUT, "Response contains unknown status"};
        }
    }
    grpc::Status status = stream->Finish();
    if (!status.ok()) {
        LOG_DEBUG("A2FControllerClient: Bidi streaming RPC failed: " << status.error_message());
        return AceClientResult{AceClientStatus::ERROR_CONNECTION, status.error_message()};
    }
    LOG_DEBUG("A2FControllerClient: SendAudio End");
    return AceClientResult{AceClientStatus::OK};
}

AceClientResult A2FControllerClient::FetchFrame(
        float timestamp,
        AceEmotionState input_emotion_state,
        AnimDataFrame *out_frame
)
{
    // do nothing
    return {AceClientStatus::OK};
}

void A2FControllerClient::buildAudioStreamHeader(AudioStreamHeader* stream_header) {
    // Fill the information of AudioStreamHeader

    AudioHeader* header = stream_header->mutable_audio_header();
    header->set_audio_format(AudioHeader_AudioFormat_AUDIO_FORMAT_PCM);
    header->set_channel_count(1);
    header->set_samples_per_second(SAMPLE_RATE);
    header->set_bits_per_sample(16);

    FaceParameters* faceParams = stream_header->mutable_face_params();
    auto* floatParams = faceParams->mutable_float_params();
    (*floatParams) = m_face_params;

    EmotionPostProcessingParameters* emotionParams = stream_header->mutable_emotion_post_processing_params();
    (*emotionParams) = m_emotion_params;

    BlendShapeParameters* bsParams = stream_header->mutable_blendshape_params();
    auto* bsMultipliers = bsParams->mutable_bs_weight_multipliers();
    auto* bsOffsets = bsParams-> mutable_bs_weight_offsets();
    (*bsMultipliers) = m_blendshape_mutlipliers;
    (*bsOffsets) = m_blendshape_offsets;

    return;
}

} // namespace mace
