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
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "grpc++/grpc++.h"

#include "ace_grpc_cpp/nvidia_ace.services.a2f_authoring.v1.grpc.pb.h"
#include "ace_grpc_cpp/nvidia_ace.services.a2f_authoring.v1_mock.grpc.pb.h"

#include "aceclient/animation.h"
#include "aceclient/parameters.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::SetArgPointee;
using ::testing::WithArg;

using nvidia_ace::a2f_authoring::v1::BlendShapeData;
using nvidia_ace::a2f_authoring::v1::AudioClipHandle;
using nvidia_ace::a2f_authoring::v1::FacePoseRequest;
using nvidia_ace::services::a2f_authoring::v1::MockA2FAuthoringServiceStub;
using mace::A2FAuthoringClient;

const size_t FPS = 30;
const std::string API_KEY = "myKey";
const std::string FUNCTION_ID = "myFunctionId";

// ==========================
// A2FAuthoringClientTests
// ==========================

class TestA2FAuthoringClient : public ::testing::Test
{
};

TEST_F(TestA2FAuthoringClient, SendAudioSucceeded) {
    const int FAKE_AUDIO_LENGTH = 1;
    AudioClipHandle fakeUploadClipResponse;
    fakeUploadClipResponse.set_audio_clip_id("some_audio_clip_id");
    BlendShapeData fakeGetPoseResponse;
    for(int i=0;i<52;++i) {
        fakeGetPoseResponse.add_blendshapes(i/52.0f);
    }
    std::shared_ptr<MockA2FAuthoringServiceStub> stub = std::make_shared<MockA2FAuthoringServiceStub>();
    std::unique_ptr<A2FAuthoringClient> client(new A2FAuthoringClient(stub, API_KEY, FUNCTION_ID));
    EXPECT_CALL(*stub, UploadAudioClip(_, _, _)).Times(1)
        .WillOnce(testing::DoAll(
            testing::SetArgPointee<2>(testing::ByRef(fakeUploadClipResponse)),
            testing::Return(grpc::Status::OK)
        ));

    // Act
    std::vector<int16_t> fake_buffer(16000 * FAKE_AUDIO_LENGTH);
    std::vector<AnimDataFrame> out_frames;
    mace::AceEmotionState emotion_state;
    auto result = client->SendAudio(fake_buffer, emotion_state, &out_frames);

    // Assert
    ASSERT_EQ(result.status, AceClientStatus::OK);
    EXPECT_GT(out_frames.size(), FPS * 1);
    const double dt = 1.0/FPS;
    for(int i=0;i<out_frames.size();++i) {
        EXPECT_DOUBLE_EQ(out_frames[i].timestamp, i * dt);
    }
}

TEST_F(TestA2FAuthoringClient, FetchFrameAfterAudioUpload) {
    const int FAKE_AUDIO_LENGTH = 1;
    const int NUM_BLENDSHAPES = 52;
    AudioClipHandle fakeUploadClipResponse;
    fakeUploadClipResponse.set_audio_clip_id("some_audio_clip_id_2");


    BlendShapeData fakeGetPoseResponse;
    for(int i=0;i<NUM_BLENDSHAPES;++i) {
        fakeUploadClipResponse.add_blendshape_names("blendshape_" + std::to_string(i));
        fakeGetPoseResponse.add_blendshapes(i/(float)NUM_BLENDSHAPES);
    }
    std::shared_ptr<MockA2FAuthoringServiceStub> stub = std::make_shared<MockA2FAuthoringServiceStub>();
    std::unique_ptr<A2FAuthoringClient> client(new A2FAuthoringClient(stub, API_KEY, FUNCTION_ID));

    EXPECT_CALL(*stub, UploadAudioClip(_, _, _)).Times(1)
        .WillOnce(testing::DoAll(
            testing::SetArgPointee<2>(testing::ByRef(fakeUploadClipResponse)),
            testing::Return(grpc::Status::OK)
        ));

    EXPECT_CALL(*stub, GetAvatarFacePose(_, testing::Truly([&fakeUploadClipResponse](const FacePoseRequest& actualRequest){
        return actualRequest.audio_hash() == fakeUploadClipResponse.audio_clip_id();
    }), _)).Times(testing::AtLeast(1))
        .WillRepeatedly(testing::DoAll(
            testing::SetArgPointee<2>(testing::ByRef(fakeGetPoseResponse)),
            testing::Return(grpc::Status::OK)
        ));

    // Act
    std::vector<int16_t> fake_buffer(16000 * FAKE_AUDIO_LENGTH);
    std::vector<AnimDataFrame> out_frames;
    mace::AceEmotionState emotion_state;
    client->SendAudio(fake_buffer, emotion_state, &out_frames);

    auto frame = out_frames[0];

    // Assert
    EXPECT_EQ(frame.blend_shape_weights.size(), NUM_BLENDSHAPES);

    // Fetch frame
    auto result = client->FetchFrame(0.0f, emotion_state, &frame);

    // Assert
    ASSERT_EQ(result.status, AceClientStatus::OK);
    EXPECT_GT(out_frames.size(), FPS * 1);
}
