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
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <optional>

#include "aceclient/animation.h"
#include "aceclient/frame_receiver.h"
#include "aceclient/audio.h"
#include "aceclient/logger.h"
#include "aceclient/parameters.h"

#include <gtest/gtest.h>

#pragma warning(disable : 4305)

std::string const test_url = "http://localhost:50051";
std::string const test_audio_path = "./sample_data/audio_4sec_16k_s16le.wav";


class StubAnimationClient : public mace::AnimationClient {
    // dummy class to access protected members
    public:
    StubAnimationClient() : AnimationClient() {};

    using AnimationClient::_url;
    using AnimationClient::_apiKey;
    using AnimationClient::_functionId;

    using AnimationClient::framerate;
    using AnimationClient::lastUpdated;
    using AnimationClient::frames;
    using AnimationClient::faceParameters;
    using AnimationClient::emotionState;
    using AnimationClient::emotionParameters;
    using AnimationClient::blendshapeMultipliers;
    using AnimationClient::blendshapeOffsets;

    using AnimationClient::getValidFrameIndex;
    using AnimationClient::findKeyIndex;
    using AnimationClient::GetNetworkAddress;
    using AnimationClient::isConnectionSecured;
};

namespace {
    class TestAnimationClient: public ::testing::Test {
        protected:
        StubAnimationClient *client;

        TestAnimationClient() {}
        virtual ~TestAnimationClient() {}

        virtual void SetUp() {
            // initialize client
            std::string address = test_url;
            client = new StubAnimationClient();
            client->_url = address;
            client->_apiKey = "_api_key_";
            client->_functionId = "_function_id_";
        }

        virtual void TearDown() {
            delete client;
        }
    };

    TEST_F(TestAnimationClient, TestSetup) {
        EXPECT_EQ(client->_apiKey, "_api_key_");
        EXPECT_EQ(client->_functionId, "_function_id_");
    };

    TEST_F(TestAnimationClient, TestSetBlendshapeMultiplier) {
        client->SetBlendshapeMultiplier("face01", 1.0f);
        EXPECT_EQ(client->blendshapeMultipliers[0].first, "face01");

        client->SetBlendshapeMultiplier("face02", 2.0f);
        EXPECT_EQ(client->blendshapeMultipliers[1].first, "face02");

        client->SetBlendshapeMultiplier("face03", 3.0f);
        EXPECT_EQ(client->blendshapeMultipliers[2].first, "face03");

        client->SetBlendshapeMultiplier("face01", 11.0f);
        EXPECT_EQ(client->blendshapeMultipliers[0].second, 11.0f);
    };

    TEST_F(TestAnimationClient, TestRemoveBlendshapeMultiplier) {
        client->blendshapeMultipliers.push_back(KEY_VALUE("key01", 1.0f));
        client->blendshapeMultipliers.push_back(KEY_VALUE("key02", 2.0f));
        client->blendshapeMultipliers.push_back(KEY_VALUE("key03", 3.0f));

        EXPECT_EQ(client->findKeyIndex("key01", client->blendshapeMultipliers), 0);
        EXPECT_EQ(client->findKeyIndex("key03", client->blendshapeMultipliers), 2);
        EXPECT_EQ(client->findKeyIndex("key0x", client->blendshapeMultipliers), -1);

        EXPECT_EQ(client->RemoveBlendshapeMultiplier("key0x"), -1);
        EXPECT_EQ(client->RemoveBlendshapeMultiplier("key01"), 0);
        EXPECT_EQ(client->RemoveBlendshapeMultiplier("key02"), 0);
        EXPECT_EQ(client->blendshapeMultipliers.size(), 1);
    };

    TEST_F(TestAnimationClient, TestGetBlendshapeMultipliers) {
        client->blendshapeMultipliers.push_back(KEY_VALUE("key01", 1.0f));
        client->blendshapeMultipliers.push_back(KEY_VALUE("key02", 2.0f));
        client->blendshapeMultipliers.push_back(KEY_VALUE("key03", 3.0f));

        auto multipliers = client->GetBlendshapeMultipliers();
        EXPECT_EQ(multipliers.size(), 3);
        EXPECT_EQ(multipliers["key01"], 1.0f);
        EXPECT_EQ(multipliers["key03"], 3.0f);

        auto keys = client->GetBlendshapeMultiplierKeys();
        ASSERT_EQ(keys.size(), 3);
        EXPECT_EQ(keys[2], "key03");

        auto values = client->GetBlendshapeMultiplierValues();
        ASSERT_EQ(values.size(), 3);
        EXPECT_EQ(values[2], 3.0f);
    };

    TEST_F(TestAnimationClient, TestSetBlendshapeOffset) {
        client->SetBlendshapeOffset("face01", 1.0f);
        EXPECT_EQ(client->blendshapeOffsets[0].first, "face01");

        client->SetBlendshapeOffset("face02", 2.0f);
        EXPECT_EQ(client->blendshapeOffsets[1].first, "face02");

        client->SetBlendshapeOffset("face03", 3.0f);
        EXPECT_EQ(client->blendshapeOffsets[2].first, "face03");

        client->SetBlendshapeOffset("face01", 11.0f);
        EXPECT_EQ(client->blendshapeOffsets[0].second, 11.0f);
    };

    TEST_F(TestAnimationClient, TestRemoveBlendshapeOffset) {
        client->blendshapeOffsets.push_back(KEY_VALUE("key01", 1.0f));
        client->blendshapeOffsets.push_back(KEY_VALUE("key02", 2.0f));
        client->blendshapeOffsets.push_back(KEY_VALUE("key03", 3.0f));

        EXPECT_EQ(client->findKeyIndex("key01", client->blendshapeOffsets), 0);
        EXPECT_EQ(client->findKeyIndex("key03", client->blendshapeOffsets), 2);
        EXPECT_EQ(client->findKeyIndex("key0x", client->blendshapeOffsets), -1);

        EXPECT_EQ(client->RemoveBlendshapeOffset("key0x"), -1);
        EXPECT_EQ(client->RemoveBlendshapeOffset("key01"), 0);
        EXPECT_EQ(client->RemoveBlendshapeOffset("key02"), 0);
        EXPECT_EQ(client->blendshapeOffsets.size(), 1);
    };

    TEST_F(TestAnimationClient, TestGetBlendshapeOffsets) {
        client->blendshapeOffsets.push_back(KEY_VALUE("key01", 1.0f));
        client->blendshapeOffsets.push_back(KEY_VALUE("key02", 2.0f));
        client->blendshapeOffsets.push_back(KEY_VALUE("key03", 3.0f));

        auto multipliers = client->GetBlendshapeOffsets();
        EXPECT_EQ(multipliers.size(), 3);
        EXPECT_EQ(multipliers["key01"], 1.0f);
        EXPECT_EQ(multipliers["key03"], 3.0f);

        auto keys = client->GetBlendshapeOffsetKeys();
        ASSERT_EQ(keys.size(), 3);
        EXPECT_EQ(keys[2], "key03");

        auto values = client->GetBlendshapeOffsetValues();
        ASSERT_EQ(values.size(), 3);
        EXPECT_EQ(values[2], 3.0f);
    };
}

TEST(TestClient, TestSetUrl) {
    mace::AnimationClient animclient;

    ASSERT_EQ(animclient.GetUrl(), "https://grpc.nvcf.nvidia.com:443");
    ASSERT_EQ(animclient.SetUrl("http://localhost:999"), AceClientStatus::OK);
    ASSERT_EQ(animclient.GetUrl(), "http://localhost:999");
    ASSERT_EQ(animclient.SetUrl("https://localhost:999"), AceClientStatus::OK);
    ASSERT_EQ(animclient.GetUrl(), "https://localhost:999");

    ASSERT_EQ(animclient.SetUrl("localhost:999"), AceClientStatus::ERROR_INVALID_INPUT);
}

TEST(TestClient, TestApiKey) {
    mace::AnimationClient client;

    client.SetAPIKey("__api_key__");
    ASSERT_EQ(client.GetAPIKey(), "__api_key__");

    client.SetAPIKey("$__TEST_VAR__");
    ASSERT_EQ(client.GetAPIKey(), "");
}

TEST(TestClient, TestGetBlendshapes) {
    mace::AnimationClient client;

    AnimDataFrame frame1;
    frame1.blend_shape_names = {"a", "b"};
    frame1.blend_shape_weights = {0.0, 0.0};
    AnimDataFrame frame2;
    frame2.blend_shape_names = {"a", "b"};
    frame2.blend_shape_weights = {0.1, 0.2};
    AnimDataFrame frame3;
    frame3.blend_shape_names = {"a", "b"};
    frame3.blend_shape_weights = {0.3, 0.4};

    client.AddFrame(frame1);
    client.AddFrame(frame2);
    client.AddFrame(frame3);

    // check blendshapes
    ASSERT_EQ(client.GetBlendshapeNames(), frame1.blend_shape_names);
    ASSERT_EQ(client.GetBlendshapeWeights(0.0f), frame1.blend_shape_weights);
    ASSERT_EQ(client.GetBlendshapeWeights(1.0f), frame3.blend_shape_weights);
    ASSERT_EQ(client.GetBlendshapeWeights(1.0f, mace::Cycle), frame1.blend_shape_weights);
    ASSERT_EQ(client.GetBlendshapeWeights((size_t)1), frame2.blend_shape_weights);

    std::vector<float> expected = {0.03, 0.06};
    ASSERT_EQ(client.GetBlendshapeWeights(0.01f), expected);
}

TEST(TestClient, TestFrameAccesses) {
    mace::AnimationClient client;
    /*
    struct AnimDataFrame {
    public:
        AnimDataFrame(const NvACEAnimDataFrame* frame, AnimDataContext context_in);
        AnimDataFrame(AnimDataContext context_in);

        std::vector<std::string> blend_shape_names;
        std::vector<float> blend_shape_weights;
        std::vector<float> audio_samples;
        double timestamp;
        AceClientStatus status;
        AnimDataContext context = nullptr;
    };
    */
    AnimDataFrame frame1;
    AnimDataFrame frame2;
    frame2.blend_shape_names = {"a", "b"};
    frame2.blend_shape_weights = {0.1, 0.2};
    AnimDataFrame frame3;
    frame3.blend_shape_names = {"a", "b"};
    frame3.blend_shape_weights = {0.3, 0.4};
    AnimDataFrame frame4;
    frame4.blend_shape_names = {"a", "b"};
    frame4.blend_shape_weights = {0.5, 0.6};
    AnimDataFrame frame5;
    frame5.blend_shape_names = {"a", "b"};
    frame5.blend_shape_weights = {0.7, 0.8};

    // check frames
    ASSERT_FALSE(client.HasAnimation((size_t)0));
    ASSERT_FALSE(client.HasAnimation(0.0f));
    ASSERT_EQ(client.ReplaceFrame(0, frame1), -1);

    // add 2 frames
    ASSERT_EQ(client.AddFrame(frame1), 1);
    ASSERT_EQ(client.AddFrame(frame2), 2);

    // check frames
    ASSERT_TRUE(client.HasAnimation(0.0f));
    ASSERT_FALSE(client.HasAnimation(1.0f));
    ASSERT_TRUE(client.HasAnimation((size_t)1));
    ASSERT_EQ(client.GetFramesCount(), 2);
    ASSERT_EQ(client.GetFrame(1).blend_shape_weights, frame2.blend_shape_weights);

    // modify frames
    ASSERT_EQ(client.RemoveFrames(0, 1), 1);
    ASSERT_EQ(client.GetFramesCount(), 1);
    ASSERT_EQ(client.GetFrame(0).blend_shape_weights, frame2.blend_shape_weights);

    // insert at the end
    ASSERT_EQ(client.InsertFrame(1, frame3), 2);
    ASSERT_EQ(client.GetFrame(1).blend_shape_weights, frame3.blend_shape_weights);

    // insert in the middle
    ASSERT_EQ(client.InsertFrame(1, frame4), 3);
    ASSERT_EQ(client.GetFrame(1).blend_shape_weights, frame4.blend_shape_weights);

    // replace in the middle
    ASSERT_EQ(client.ReplaceFrame(1, frame5), 3);
    ASSERT_EQ(client.GetFrame(1).blend_shape_weights, frame5.blend_shape_weights);
}

TEST(TestClient, TestFaceParameters) {
    mace::AnimationClient animclient;

    mace::AceFaceParameters external;
    mace::AceFaceParameters internal1 = animclient.GetFaceParameters();

    // client has default parameters
    ASSERT_EQ(internal1.SkinStrength, external.SkinStrength);
    ASSERT_NE(&internal1, &external);

    // Get returns a copied data
    internal1.SkinStrength = 0.5f;
    mace::AceFaceParameters internal2 = animclient.GetFaceParameters();
    ASSERT_EQ(internal2.SkinStrength, external.SkinStrength);

    // Set updates internal data
    animclient.SetFaceParameters(internal1);
    mace::AceFaceParameters internal3 = animclient.GetFaceParameters();
    ASSERT_EQ(internal3.SkinStrength, 0.5f);
}

TEST(TestClient, TestEmotionParameters) {
    mace::AnimationClient animclient;

    mace::AceEmotionParameters external;
    mace::AceEmotionParameters internal1 = animclient.GetEmotionParameters();

    // client has default parameters
    ASSERT_EQ(internal1.emotion_contrast, external.emotion_contrast);
    ASSERT_NE(&internal1, &external);

    // Get returns a copied data
    internal1.emotion_strength = 0.3f;
    mace::AceEmotionParameters internal2 = animclient.GetEmotionParameters();
    ASSERT_EQ(internal2.emotion_strength, external.emotion_strength);

    // Set updates internal data
    animclient.SetEmotionParameters(internal1);
    mace::AceEmotionParameters internal3 = animclient.GetEmotionParameters();
    ASSERT_EQ(internal3.emotion_strength, 0.3f);
}

TEST(TestClient, TestEmotionState) {
    mace::AnimationClient animclient;

    mace::AceEmotionState external;
    mace::AceEmotionState internal1 = animclient.GetEmotionState();

    // client has default parameters
    ASSERT_EQ(internal1.amazement, external.amazement);
    ASSERT_NE(&internal1, &external);

    // Get returns a copied data
    internal1.amazement = 0.3f;
    mace::AceEmotionState internal2 = animclient.GetEmotionState();
    ASSERT_EQ(internal2.amazement, external.amazement);

    // Set updates internal data
    animclient.SetEmotionState(internal1);
    mace::AceEmotionState internal3 = animclient.GetEmotionState();
    ASSERT_EQ(internal3.amazement, 0.3f);
}

TEST(TestClient, TestUpdateAnimationFrames) {
    /*Full request animation pipeline test; connect, session, send audio, and receive frames.
    */
    mace::AnimationClient client;
    AceClientStatus result;

    std::vector<int16_t> samples;
    for (int i = 0; i < 8320; i++) {
        samples.push_back(0);
    }
    client.SetUrl(test_url);

    ASSERT_EQ(client.GetLastUpdated(), 0);
    result = client.UpdateAnimation(samples);

    ASSERT_EQ(result, AceClientStatus::OK);
    ASSERT_GE(client.GetFramesCount(), 1);
    std::cout << "Frames received: " << client.GetFramesCount() << std::endl;

    ASSERT_GE(client.GetLastUpdated(), 1);
}

TEST(TestClient, TestRequestAnimation1) {
    /*Full request animation pipeline test; connect, session, send audio, and receive frames.
    */
    mace::AnimationClient animclient;
    AceClientStatus result;
    std::vector<AnimDataFrame> frames;

    std::filesystem::path input_path(test_audio_path);
    std::vector<int16_t> samples = get_file_wav_content_int16(input_path.u8string());
    int max_sample = *max_element(samples.begin(), samples.end());
    int min_sample = *min_element(samples.begin(), samples.end());
    std::cout << "[Test] Sending " << samples.size() << " samples";
    std::cout << " with values in range of [" << max_sample << ", " << min_sample << "]" << std::endl;
    animclient.SetUrl(test_url);

    result = animclient.RequestAnimation(samples, &frames);
    ASSERT_EQ(result, AceClientStatus::OK);
    ASSERT_GE(frames.size(), 1);
    std::cout << "Frames received: " << frames.size() << std::endl;

    for (const AnimDataFrame& frame : frames) {
      ASSERT_EQ(52, frame.blend_shape_weights.size());
    }
}

TEST(TestClient, TestRequestAnimation2) {
    /* 2nd test to check if FrameReceiver or acl works with repeated calls in global.
    */
    mace::AnimationClient animclient;
    AceClientStatus result;
    std::vector<AnimDataFrame> frames;

    std::vector<int16_t> samples;
    for (int i = 0; i < 8320; i++) {
        samples.push_back(0);
    }
    animclient.SetUrl(test_url);

    result = animclient.RequestAnimation(samples, &frames);
    ASSERT_EQ(result, AceClientStatus::OK);
    ASSERT_GE(frames.size(), 1);

    for (const AnimDataFrame& frame : frames) {
      ASSERT_EQ(52, frame.blend_shape_weights.size());
    }
}
