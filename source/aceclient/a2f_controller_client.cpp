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

#include <iostream>

#include "logger.h"
#include "a2f_controller_client.h"
#include "ace_grpc_cpp/nvidia_ace.emotion_aggregate.v1.pb.h"

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

using grpc::health::v1::HealthCheckRequest;
using grpc::health::v1::HealthCheckResponse;

#define CHECK_TRUE(expr, msg) \
    do { \
        if (!(expr)) { \
            LOG_ERROR("A2FControllerClient: " << msg); \
            return AceClientStatus::ERROR_UNKNOWN; \
        } \
    } while(0)

const int TIMEOUT_SEC = 3;
const size_t SAMPLE_RATE = 16000;
const size_t CHUNK_SIZE = SAMPLE_RATE * sizeof(int16_t);
const std::vector<std::string> EMOTION_STATE_NAMES = {
    "amazement",
    "anger",
    "cheekiness",
    "disgust",
    "fear",
    "grief",
    "joy",
    "outofbreath",
    "pain",
    "sadness",
};

namespace {

// Delay the deadline in the gRPC context.
void SetContextDeadline(grpc::ClientContext &context) {
    context.set_deadline(
        std::chrono::system_clock::now() + std::chrono::seconds(TIMEOUT_SEC));
}

// Increase the deadline in the gRPC context before calling stream->Write, effectively setting a timeout for stream writes.
bool WriteWithDeadline(grpc::ClientContext &context,
    std::shared_ptr<grpc::ClientReaderWriterInterface<AudioStream, AnimationDataStream>> stream,
    const AudioStream &request) {
    SetContextDeadline(context);
    return stream->Write(request);
}

// Increase the deadline in the gRPC context before calling stream->Read, effectively setting a timeout for stream reads.
bool ReadWithDeadline(grpc::ClientContext &context,
    std::shared_ptr<grpc::ClientReaderWriterInterface<AudioStream, AnimationDataStream>> stream,
    AnimationDataStream *response) {
    SetContextDeadline(context);
    return stream->Read(response);
}

}

namespace mace {

AceClientStatus A2FControllerHealthCheck(
    std::shared_ptr<Health::StubInterface> stub, std::string apiKey, std::string functionId) {
    grpc::ClientContext context;
    context.set_deadline(
        std::chrono::system_clock::now() + std::chrono::seconds(TIMEOUT_SEC));

    if (!apiKey.empty()) {
      context.AddMetadata("authorization", "Bearer " + apiKey);
    }
    if (!functionId.empty()) {
      context.AddMetadata("function-id", functionId);
    }

    LOG_DEBUG("A2FControllerHealthCheck: Checking...");
    HealthCheckRequest request;
    HealthCheckResponse response;
    grpc::Status status = stub->Check(&context, request, &response);
    if (!status.ok()) {
        std::string errorMessage = status.error_message();
        LOG_ERROR("A2FControllerHealthCheck: Failed. code: " << status.error_code() << ", reason: " << errorMessage);
        // The error code did not accurately reflect the actual cause of the failure; we need to parse the error message to identify the correct issue.
        if (errorMessage.find("Unauthenticated") != std::string::npos ||
            errorMessage.find("no authorization was passed") != std::string::npos) {
            // failed to open stateful work request: rpc error: code = Unauthenticated desc = invalid response from UAM
            // or
            // no authorization was passed in the metadata
            return AceClientStatus::ERROR_UNAUTHENTICATED;
        } else if (errorMessage.find("SSL_ERROR_SSL") != std::string::npos) {
            // failed to connect to all addresses; last error: UNKNOWN: ipv4:xxx.xxx.xxx.xxx:port: Ssl handshake failed: SSL_ERROR_SSL: error:FFFFFFFF:SSL routines::wrong version number
            return AceClientStatus::ERROR_SSL_HANDSHAKE;
        } else if (errorMessage.find("Deadline Exceeded") != std::string::npos) {
            // Deadline Exceeded
            // Possible causes:
            // - bad network connection
            // - invalid server port
            // - connecting to a https server using http:// protocol
            return AceClientStatus::ERROR_CONNECTION;
        } else if (errorMessage.find("Cloud credits expired") != std::string::npos) {
            // failed to open stateful work request: rpc error: code = Unknown desc = Account 'xxx': Cloud credits expired - Please contact NVIDIA representatives
            return AceClientStatus::ERROR_CREDITS_EXPIRED;
        } else if (errorMessage.find("DNS resolution failed") != std::string::npos) {
            // Failed. code: 14, reason: DNS resolution failed for domain.not.exist.com:443: C-ares status is not ARES_SUCCESS qtype=AAAA name=domain.not.exist.com is_balancer=0: Domain name not found
            return AceClientStatus::ERROR_DNS_RESOLUTION;
        } else if (errorMessage.find("Connection refused") != std::string::npos) {
            // Failed. code: 14, reason: failed to connect to all addresses; last error: UNAVAILABLE: ipv4:127.0.0.127:4434: Connection refused
            return AceClientStatus::ERROR_CONNECTION;
        }
        // The following is a list of known error types. However, the current error messages are vague and lack sufficient detail.
        // The server should be updated to provide more informative error messages instead of a generic internal server error.
        // - Invalid functionId:
        //   Failed. code: 13, reason: failed to open stateful work request: rpc error: code = Internal desc = There was a server error trying to handle an exception
        return AceClientStatus::ERROR_UNKNOWN;
    }
    LOG_DEBUG("A2FControllerHealthCheck: Ok!");
    return AceClientStatus::OK;
}

AceClientStatus A2FControllerHealthCheck(
    std::shared_ptr<grpc::Channel> channel, std::string apiKey, std::string functionId) {
    std::shared_ptr<Health::StubInterface> stub = Health::NewStub(channel);
    return A2FControllerHealthCheck(stub, apiKey, functionId);
}


A2FControllerClient::A2FControllerClient(std::shared_ptr<grpc::Channel> channel, std::string apiKey, std::string functionId)
      : m_stub(A2FControllerService::NewStub(channel).release()), m_apiKey(apiKey), m_functionId(functionId) {}

A2FControllerClient::A2FControllerClient(std::shared_ptr<A2FControllerService::StubInterface> stub, std::string apiKey, std::string functionId)
      : m_stub(stub), m_apiKey(apiKey), m_functionId(functionId) {}

void A2FControllerClient::SetFaceParam(const char *key, float val) {
    m_faceParameters[key] = val;
}

void A2FControllerClient::SetEmotionPostProcessingParams(EmotionPostProcessingParameters &param) {
    m_emotionPostProcessingParameters = param;
}

void A2FControllerClient::SetBlendshapeMultiplier(const char *key, float value) {
    m_blendshapeMultipliers[key] = value;
}

void A2FControllerClient::SetBlendshapeOffset(const char *key, float value) {
    m_blendshapeOffsets[key] = value;
}

AceClientStatus A2FControllerClient::ProcessAudioStream(
    const int16_t *samples, size_t sample_count, AceEmotionState input_emotion_state, std::vector<AnimDataFrame> *out_frames) {
    grpc::ClientContext context;
    if (!m_apiKey.empty()) {
      context.AddMetadata("authorization", "Bearer " + m_apiKey);
    }
    if (!m_functionId.empty()) {
      context.AddMetadata("function-id", m_functionId);
    }
    std::shared_ptr<grpc::ClientReaderWriterInterface<AudioStream, AnimationDataStream>> stream(m_stub->ProcessAudioStream(&context));

    LOG_DEBUG("A2FControllerClient: ProcessAudioStream Start");
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
        const size_t totalBufferSize = sample_count * sizeof(int16_t); // total buffer size in bytes
        const uint8_t* audioBufferPtr = reinterpret_cast<const uint8_t*>(samples);
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
            emotion->insert({"out_of_breath", input_emotion_state.out_of_breath});
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
        LOG_ERROR("Response header is not sent as the first response message");
        stream->Finish();
        return AceClientStatus::ERROR_UNEXPECTED_OUTPUT;
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
                        for (auto name : EMOTION_STATE_NAMES) {
                            if (emotion.find(name) != emotion.end()) {
                                emotion_state.push_back(emotion[name]);
                            } else {
                                emotion_state.push_back(0.0f);
                            }
                        }
                    }
                } else {
                    LOG_ERROR("A2FControllerClient: Expected EmotionAggregate in metadata[" << EMOTION_AGGREGATE_KEY << " but got: " << any_value.type_url());
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
                    animDataFrame.emotion_state_names = EMOTION_STATE_NAMES;
                    animDataFrame.emotion_state = emotion_state;
                    animDataFrame.blend_shape_weights = std::vector<float>(blend_shape_weights.values().begin(), blend_shape_weights.values().end());
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
                return AceClientStatus::ERROR_UNEXPECTED_OUTPUT;
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
            return AceClientStatus::ERROR_UNEXPECTED_OUTPUT;
        }
    }
    grpc::Status status = stream->Finish();
    if (!status.ok()) {
        LOG_DEBUG("A2FControllerClient: Bidi streaming RPC failed: " << status.error_message());
        return AceClientStatus::ERROR_CONNECTION;
    }
    LOG_DEBUG("A2FControllerClient: ProcessAudioStream End");
    return AceClientStatus::OK;
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
    (*floatParams) = m_faceParameters;

    EmotionPostProcessingParameters* emotionParams = stream_header->mutable_emotion_post_processing_params();
    (*emotionParams) = m_emotionPostProcessingParameters;

    BlendShapeParameters* bsParams = stream_header->mutable_blendshape_params();
    auto* bsMultipliers = bsParams->mutable_bs_weight_multipliers();
    auto* bsOffsets = bsParams-> mutable_bs_weight_offsets();
    (*bsMultipliers) = m_blendshapeMultipliers;
    (*bsOffsets) = m_blendshapeOffsets;

    return;
}

} // namespace mace
