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
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <optional>

#include "ace_client/a2f_client_interface.h"
#include "ace_client/animation.h"
#include "ace_client/frame_receiver.h"
#include "ace_client/audio.h"
#include "ace_client/logger.h"
#include "ace_client/parameters.h"
#include "ace_client/grpc_client.h"

#include <gtest/gtest.h>

#pragma warning(disable : 4305)

extern std::string TEST_URL; // defined and populated in main.cpp
std::string const test_audio_path = "./sample_data/audio_4sec_16k_s16le.wav";


class MockNetworkedAnimation : public mace::NetworkedAnimation {
    // dummy class to access protected members
    public:
    MockNetworkedAnimation() : NetworkedAnimation() {};

    using NetworkedAnimation::m_address;
    using NetworkedAnimation::m_api_key;
    using NetworkedAnimation::m_function_id;

    using NetworkedAnimation::m_framerate;
    using NetworkedAnimation::m_last_updated;
    using NetworkedAnimation::m_frames;
    using NetworkedAnimation::m_face_params;
    using NetworkedAnimation::m_emotion_state;
    using NetworkedAnimation::m_emotion_params;
    using NetworkedAnimation::m_blendshape_multipliers;
    using NetworkedAnimation::m_blendshape_offsets;

    using NetworkedAnimation::GetValidFrameIndex;
    using NetworkedAnimation::SendAudioWithParameters;
};

namespace {
    class TestNetworkedAnimation: public ::testing::Test {
        protected:
        MockNetworkedAnimation *client;

        TestNetworkedAnimation() {}
        virtual ~TestNetworkedAnimation() {}

        virtual void SetUp() {
            // initialize client
            std::string address = TEST_URL;
            client = new MockNetworkedAnimation();
            client->m_address = address;
            client->m_api_key = "_api_key_";
            client->m_function_id = "_function_id_";
        }

        virtual void TearDown() {
            delete client;
        }
    };

    TEST_F(TestNetworkedAnimation, TestSetup) {
        EXPECT_EQ(client->m_api_key, "_api_key_");
        EXPECT_EQ(client->m_function_id, "_function_id_");
    };

    TEST_F(TestNetworkedAnimation, TestSetBlendshapeMultiplier) {
        client->SetBlendshapeMultiplier("face01", 1.0f);
        EXPECT_EQ(client->m_blendshape_multipliers[0].first, "face01");

        client->SetBlendshapeMultiplier("face02", 2.0f);
        EXPECT_EQ(client->m_blendshape_multipliers[1].first, "face02");

        client->SetBlendshapeMultiplier("face03", 3.0f);
        EXPECT_EQ(client->m_blendshape_multipliers[2].first, "face03");

        client->SetBlendshapeMultiplier("face01", 11.0f);
        EXPECT_EQ(client->m_blendshape_multipliers[0].second, 11.0f);
    };

    TEST_F(TestNetworkedAnimation, TestRemoveBlendshapeMultiplier) {
        client->m_blendshape_multipliers.push_back(KEY_VALUE("key01", 1.0f));
        client->m_blendshape_multipliers.push_back(KEY_VALUE("key02", 2.0f));
        client->m_blendshape_multipliers.push_back(KEY_VALUE("key03", 3.0f));

        EXPECT_EQ(mace::FindPairIndex("key01", client->m_blendshape_multipliers), 0);
        EXPECT_EQ(mace::FindPairIndex("key03", client->m_blendshape_multipliers), 2);
        EXPECT_EQ(mace::FindPairIndex("key0x", client->m_blendshape_multipliers), -1);

        EXPECT_EQ(client->RemoveBlendshapeMultiplier("key0x"), -1);
        EXPECT_EQ(client->RemoveBlendshapeMultiplier("key01"), 0);
        EXPECT_EQ(client->RemoveBlendshapeMultiplier("key02"), 0);
        EXPECT_EQ(client->m_blendshape_multipliers.size(), 1);
    };

    TEST_F(TestNetworkedAnimation, TestGetBlendshapeMultipliers) {
        client->m_blendshape_multipliers.push_back(KEY_VALUE("key01", 1.0f));
        client->m_blendshape_multipliers.push_back(KEY_VALUE("key02", 2.0f));
        client->m_blendshape_multipliers.push_back(KEY_VALUE("key03", 3.0f));

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

    TEST_F(TestNetworkedAnimation, TestSetBlendshapeOffset) {
        client->SetBlendshapeOffset("face01", 1.0f);
        EXPECT_EQ(client->m_blendshape_offsets[0].first, "face01");

        client->SetBlendshapeOffset("face02", 2.0f);
        EXPECT_EQ(client->m_blendshape_offsets[1].first, "face02");

        client->SetBlendshapeOffset("face03", 3.0f);
        EXPECT_EQ(client->m_blendshape_offsets[2].first, "face03");

        client->SetBlendshapeOffset("face01", 11.0f);
        EXPECT_EQ(client->m_blendshape_offsets[0].second, 11.0f);
    };

    TEST_F(TestNetworkedAnimation, TestRemoveBlendshapeOffset) {
        client->m_blendshape_offsets.push_back(KEY_VALUE("key01", 1.0f));
        client->m_blendshape_offsets.push_back(KEY_VALUE("key02", 2.0f));
        client->m_blendshape_offsets.push_back(KEY_VALUE("key03", 3.0f));

        EXPECT_EQ(mace::FindPairIndex("key01", client->m_blendshape_offsets), 0);
        EXPECT_EQ(mace::FindPairIndex("key03", client->m_blendshape_offsets), 2);
        EXPECT_EQ(mace::FindPairIndex("key0x", client->m_blendshape_offsets), -1);

        EXPECT_EQ(client->RemoveBlendshapeOffset("key0x"), -1);
        EXPECT_EQ(client->RemoveBlendshapeOffset("key01"), 0);
        EXPECT_EQ(client->RemoveBlendshapeOffset("key02"), 0);
        EXPECT_EQ(client->m_blendshape_offsets.size(), 1);
    };

    TEST_F(TestNetworkedAnimation, TestGetBlendshapeOffsets) {
        client->m_blendshape_offsets.push_back(KEY_VALUE("key01", 1.0f));
        client->m_blendshape_offsets.push_back(KEY_VALUE("key02", 2.0f));
        client->m_blendshape_offsets.push_back(KEY_VALUE("key03", 3.0f));

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

TEST(TestNetworkedAnimation2, TestSetUrl) {
    mace::NetworkedAnimation anim;

    EXPECT_EQ(anim.GetAddress(), "https://grpc.nvcf.nvidia.com:443");
    anim.SetAddress("http://localhost:999");
    EXPECT_EQ(anim.GetAddress(), "http://localhost:999");
}

TEST(TestNetworkedAnimation2, TestApiKey) {
    mace::NetworkedAnimation client;

    client.SetAPIKey("__api_key__");
    ASSERT_EQ(client.GetAPIKey(), "__api_key__");

    client.SetAPIKey("$__TEST_VAR__");
    ASSERT_EQ(client.GetAPIKey(), "");
}

TEST(TestNetworkedAnimation2, TestFunctionId) {
    mace::NetworkedAnimation client;

    client.SetFunctionId("__function_id__");
    ASSERT_EQ(client.GetFunctionId(), "__function_id__");

    client.SetFunctionId("$__TEST_VAR__");
    ASSERT_EQ(client.GetFunctionId(), "");
}

TEST(TestNetworkedAnimation2, TestGetBlendshapes) {
    mace::NetworkedAnimation client;

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

TEST(TestNetworkedAnimation2, TestFrameAccesses) {
    mace::NetworkedAnimation client;
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

TEST(TestNetworkedAnimation2, TestFaceParameters) {
    mace::NetworkedAnimation anim;

    mace::AceFaceParameters external;
    mace::AceFaceParameters internal1 = anim.GetFaceParameters();

    // client has default parameters
    ASSERT_EQ(internal1.SkinStrength, external.SkinStrength);
    ASSERT_NE(&internal1, &external);

    // Get returns a copied data
    internal1.SkinStrength = 0.5f;
    mace::AceFaceParameters internal2 = anim.GetFaceParameters();
    ASSERT_EQ(internal2.SkinStrength, external.SkinStrength);

    // Set updates internal data
    anim.SetFaceParameters(internal1);
    mace::AceFaceParameters internal3 = anim.GetFaceParameters();
    ASSERT_EQ(internal3.SkinStrength, 0.5f);
}

TEST(TestNetworkedAnimation2, TestEmotionParameters) {
    mace::NetworkedAnimation anim;

    mace::AceEmotionParameters external;
    mace::AceEmotionParameters internal1 = anim.GetEmotionParameters();

    // client has default parameters
    ASSERT_EQ(internal1.emotion_contrast, external.emotion_contrast);
    ASSERT_NE(&internal1, &external);

    // Get returns a copied data
    internal1.emotion_strength = 0.3f;
    mace::AceEmotionParameters internal2 = anim.GetEmotionParameters();
    ASSERT_EQ(internal2.emotion_strength, external.emotion_strength);

    // Set updates internal data
    anim.SetEmotionParameters(internal1);
    mace::AceEmotionParameters internal3 = anim.GetEmotionParameters();
    ASSERT_EQ(internal3.emotion_strength, 0.3f);
}

TEST(TestNetworkedAnimation2, TestEmotionState) {
    mace::NetworkedAnimation anim;

    mace::AceEmotionState external;
    mace::AceEmotionState internal1 = anim.GetEmotionState();

    // client has default parameters
    ASSERT_EQ(internal1.amazement, external.amazement);
    ASSERT_NE(&internal1, &external);

    // Get returns a copied data
    internal1.amazement = 0.3f;
    mace::AceEmotionState internal2 = anim.GetEmotionState();
    ASSERT_EQ(internal2.amazement, external.amazement);

    // Set updates internal data
    anim.SetEmotionState(internal1);
    mace::AceEmotionState internal3 = anim.GetEmotionState();
    ASSERT_EQ(internal3.amazement, 0.3f);
}

TEST(TestNetworkedAnimation2, TestUpdateNetworkedAnimation) {
    /*Full request animation pipeline test; connect, session, send audio, and receive frames.
    */
    mace::NetworkedAnimation client;

    std::vector<int16_t> samples;
    for (int i = 0; i < 8320; i++) {
        samples.push_back(0);
    }
    client.SetAddress(TEST_URL);

    ASSERT_EQ(client.GetLastUpdated(), 0);
    auto result = client.ConnectAndSendAudio(samples);

    ASSERT_EQ(result.status, AceClientStatus::OK);
    ASSERT_GE(client.GetFramesCount(), 1);
    std::cout << "Frames received: " << client.GetFramesCount() << std::endl;

    ASSERT_GE(client.GetLastUpdated(), 1);
}

TEST(TestNetworkedAnimation2, TestRequestAnimation1) {
    /*Full request animation pipeline test; connect, session, send audio, and receive frames.
    */
    class TestNetworkedAnimation : public mace::NetworkedAnimation {
    // dummy class to access protected members
    public:
        TestNetworkedAnimation() : NetworkedAnimation() {};
        using NetworkedAnimation::m_frames;
    };
    TestNetworkedAnimation anim;

    std::filesystem::path input_path(test_audio_path);
    std::vector<int16_t> samples = get_file_wav_content_int16(input_path.u8string());
    int max_sample = *max_element(samples.begin(), samples.end());
    int min_sample = *min_element(samples.begin(), samples.end());
    std::cout << "[Test] Sending " << samples.size() << " samples";
    std::cout << " with values in range of [" << max_sample << ", " << min_sample << "]" << std::endl;

    anim.SetAddress(TEST_URL);

    auto result = anim.ConnectAndSendAudio(samples);

    ASSERT_EQ(result.status, AceClientStatus::OK);
    ASSERT_GE(anim.m_frames.size(), 1);
    std::cout << "Frames received: " << anim.m_frames.size() << std::endl;

    for (const AnimDataFrame& frame : anim.m_frames) {
      ASSERT_EQ(52, frame.blend_shape_weights.size());
    }
}

TEST(TestNetworkedAnimation2, TestRequestAnimation2) {
    /* 2nd test to check if FrameReceiver works with repeated calls in global.
    */
    class TestNetworkedAnimation : public mace::NetworkedAnimation {
    // dummy class to access protected members
    public:
        TestNetworkedAnimation() : NetworkedAnimation() {};
        using NetworkedAnimation::m_frames;
    };
    TestNetworkedAnimation anim;

    std::vector<int16_t> samples;
    for (int i = 0; i < 8320; i++) {
        samples.push_back(0);
    }
    anim.SetAddress(TEST_URL);

    auto result = anim.ConnectAndSendAudio(samples);

    ASSERT_EQ(result.status, AceClientStatus::OK);
    ASSERT_GE(anim.m_frames.size(), 1);
    std::cout << "Frames received: " << anim.m_frames.size() << std::endl;
}
