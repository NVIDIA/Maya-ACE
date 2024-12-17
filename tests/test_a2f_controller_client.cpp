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

#include "ace_grpc_cpp/nvidia_ace.services.a2f_controller.v1.grpc.pb.h"
#include "ace_grpc_cpp/nvidia_ace.services.a2f_controller.v1_mock.grpc.pb.h"

#include "aceclient/animation.h"
#include "aceclient/parameters.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::SetArgPointee;
using ::testing::WithArg;

using nvidia_ace::controller::v1::AudioStream;
using nvidia_ace::controller::v1::AnimationDataStream;
using nvidia_ace::services::a2f_controller::v1::MockA2FControllerServiceStub;
using mace::A2FControllerClient;

const std::string API_KEY = "myKey";
const std::string FUNCTION_ID = "myFunctionId";


// ==========================
// A2FControllerClientTests
// ==========================

class MockClientReaderWriter : public grpc::ClientReaderWriterInterface<AudioStream, AnimationDataStream> {
public:
  MOCK_METHOD(bool, Write, (const AudioStream& request, grpc::WriteOptions options), (override));
  MOCK_METHOD(bool, Read, (AnimationDataStream* response), (override));
  MOCK_METHOD(bool, WritesDone, (), (override));
  MOCK_METHOD(grpc::Status, Finish, (), (override));
  MOCK_METHOD(void, WaitForInitialMetadata, (), (override));
  MOCK_METHOD(bool, NextMessageSize, (uint32_t *sz), (override));
};

class TestA2FControllerClient : public ::testing::Test
{
};


TEST_F(TestA2FControllerClient, SendAudioSucceeded) {
  const int NUM_BLENDSHAPES = 52;
  std::shared_ptr<MockA2FControllerServiceStub> stub = std::make_shared<MockA2FControllerServiceStub>();
  std::unique_ptr<A2FControllerClient> client(new A2FControllerClient(stub, API_KEY, FUNCTION_ID));
  auto stream = new MockClientReaderWriter();
  EXPECT_CALL(*stub, ProcessAudioStreamRaw(_)).WillOnce(Return(stream));

  AnimationDataStream firstResponse;
  auto responseHeader = firstResponse.mutable_animation_data_stream_header();
  auto skelAnimationHeader = responseHeader->mutable_skel_animation_header();
  for(int i=0;i<NUM_BLENDSHAPES;++i) {
    skelAnimationHeader->add_blend_shapes("face_" + std::to_string(i));
  }

  AnimationDataStream fakeAnimationData;
  auto animationData = fakeAnimationData.mutable_animation_data();
  auto skelAnimation = animationData->mutable_skel_animation();
  auto blendshapeWeights = skelAnimation->add_blend_shape_weights();
  const double expectedTimeCode = 3.14;
  blendshapeWeights->set_time_code(expectedTimeCode);
  for(int i=0;i<NUM_BLENDSHAPES;++i) {
    blendshapeWeights->add_values(0.1f * i);
  }

  EXPECT_CALL(*stream, Read(_))
    .Times(AtLeast(3))
    .WillOnce(testing::DoAll(testing::SetArgPointee<0>(testing::ByRef(firstResponse)), testing::Return(true)))
    .WillOnce(testing::DoAll(testing::SetArgPointee<0>(testing::ByRef(fakeAnimationData)), testing::Return(true)))
    .WillRepeatedly(testing::Return(false));
  EXPECT_CALL(*stream, Write(_, _)).Times(AtLeast(3)).WillRepeatedly(testing::Return(true));
  EXPECT_CALL(*stream, WritesDone()).Times(1).WillRepeatedly(testing::Return(true));
  EXPECT_CALL(*stream, Finish()).Times(1).WillRepeatedly(testing::Return(grpc::Status::OK));

  // Act
  std::vector<int16_t> fake_buffer(16000);
  std::vector<AnimDataFrame> out_frames;
  mace::AceEmotionState emotion_state;
  auto result = client->SendAudio(fake_buffer, emotion_state, &out_frames);

  // Assert
  ASSERT_EQ(result.status, AceClientStatus::OK);
  EXPECT_EQ(out_frames.size(), 1);
  EXPECT_DOUBLE_EQ(out_frames[0].timestamp, expectedTimeCode);
  for(int i=0;i<NUM_BLENDSHAPES;++i) {
    EXPECT_EQ(out_frames[0].blend_shape_names[i], skelAnimationHeader->blend_shapes(i));
    EXPECT_FLOAT_EQ(out_frames[0].blend_shape_weights[i], blendshapeWeights->values(i));
  }
}

TEST_F(TestA2FControllerClient, ProcessAudioStreamFirstResponseIsNotHeader) {
  std::shared_ptr<MockA2FControllerServiceStub> stub = std::make_shared<MockA2FControllerServiceStub>();
  std::unique_ptr<A2FControllerClient> client(new A2FControllerClient(stub, API_KEY, FUNCTION_ID));
  auto stream = new MockClientReaderWriter();
  EXPECT_CALL(*stub, ProcessAudioStreamRaw(_)).WillOnce(Return(stream));

  AnimationDataStream fakeAnimationData;
  fakeAnimationData.mutable_animation_data();

  EXPECT_CALL(*stream, Read(_))
    .Times(1)
    .WillOnce(testing::DoAll(testing::SetArgPointee<0>(testing::ByRef(fakeAnimationData)), testing::Return(true)));
  EXPECT_CALL(*stream, Write(_, _)).Times(AtLeast(3)).WillRepeatedly(testing::Return(true));
  EXPECT_CALL(*stream, WritesDone()).Times(1).WillRepeatedly(testing::Return(true));
  EXPECT_CALL(*stream, Finish()).Times(1).WillRepeatedly(testing::Return(grpc::Status::OK));

  // Act
  std::vector<int16_t> fake_buffer(16000);
  std::vector<AnimDataFrame> out_frames;
  mace::AceEmotionState emotion_state;
  auto result = client->SendAudio(fake_buffer, emotion_state, &out_frames);

  // Assert
  ASSERT_EQ(result.status, AceClientStatus::ERROR_UNEXPECTED_OUTPUT);
}

TEST_F(TestA2FControllerClient, TestSetup) {
  const std::string test_api_key = "_api_key_";
  const std::string test_function_id = "_function_id_";
  std::shared_ptr<MockA2FControllerServiceStub> stub = std::make_shared<MockA2FControllerServiceStub>();

  // Act
  std::unique_ptr<A2FControllerClient> client(new A2FControllerClient(stub, test_api_key, test_function_id));

  // Assert
  EXPECT_EQ(client->m_api_key, test_api_key);
}

TEST_F(TestA2FControllerClient, TestSetBlendshapeParameters) {
  std::shared_ptr<MockA2FControllerServiceStub> stub = std::make_shared<MockA2FControllerServiceStub>();
  std::unique_ptr<A2FControllerClient> client(new A2FControllerClient(stub, API_KEY, FUNCTION_ID));

  EXPECT_EQ(client->m_blendshape_mutlipliers.size(), 0);
  EXPECT_EQ(client->m_blendshape_offsets.size(), 0);

  // Act
  client->SetBlendshapeMultiplier("__face_02__", 0.2f);
  client->SetBlendshapeOffset("__face_02__", 2.0f);
  EXPECT_EQ(client->m_blendshape_mutlipliers.size(), 1);
  EXPECT_EQ(client->m_blendshape_offsets.size(), 1);

  EXPECT_EQ(client->m_blendshape_mutlipliers["__face_02__"], 0.2f);
  EXPECT_EQ(client->m_blendshape_offsets["__face_02__"], 2.0f);
}

TEST_F(TestA2FControllerClient, TestBuildAudioStreamHeader) {
  std::shared_ptr<MockA2FControllerServiceStub> stub = std::make_shared<MockA2FControllerServiceStub>();
  std::unique_ptr<A2FControllerClient> client(new A2FControllerClient(stub, API_KEY, FUNCTION_ID));

  AudioStreamHeader header;

  // Act
  client->SetBlendshapeMultiplier("_face_03_", 0.3f);
  client->SetBlendshapeOffset("_face_03_", 3.0f);
  client->SetFaceParam("_param_03_", 30.0f);
  client->buildAudioStreamHeader(&header);

  // Assert
  auto multipliers = header.blendshape_params().bs_weight_multipliers();
  EXPECT_EQ(multipliers["_face_03_"], 0.3f);

  auto offsets = header.blendshape_params().bs_weight_offsets();
  EXPECT_EQ(offsets["_face_03_"], 3.0f);

  auto float_params = header.face_params().float_params();
  EXPECT_EQ(float_params["_param_03_"], 30.0f);
}
