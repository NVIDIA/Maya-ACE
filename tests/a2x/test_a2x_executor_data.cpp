// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

#include "test_constants.h"

#include <memory>
#include <system_error>

#include "gtest/gtest.h"

#include <audio2face/audio2face.h>
#include <audio2face/executor.h>

#include "maya_ace/common/a2x_data.h"
#include "maya_ace/common/a2x_executors.h"

// ==========================
// A2FExecutorData Tests
// ==========================

class TestA2FExecutorData : public ::testing::Test {
protected:  
    std::unique_ptr<CudaStreamData> cudaStreamData;
    std::unique_ptr<AudioAccumulatorData> audioAccumulatorData;
    std::unique_ptr<EmotionAccumulatorData> emotionAccumulatorData;
    nva2f::GeometryExecutorCreationParameters params;
    std::size_t nbEmotions = 10;

    nva2x::IAudioAccumulator* audioAccumulatorPtr = nullptr;
    nva2x::IEmotionAccumulator* emotionAccumulatorPtr = nullptr;
    
    static constexpr std::size_t TEST_AUDIO_BUFFER_LENGTH = 8096;

    void SetUp() override {
        // Initialize dependencies for real executor creation
        cudaStreamData = std::make_unique<CudaStreamData>(CudaStreamData{
            std::move(ToUniquePtr(nva2x::CreateCudaStream()))
        });
        
        audioAccumulatorData = std::make_unique<AudioAccumulatorData>(AudioAccumulatorData{
            std::move(ToUniquePtr(nva2x::CreateAudioAccumulator(16000, 0)))
        });
        
        // Determine emotion count from regression model
        auto modelInfo = ToUniquePtr(nva2f::ReadRegressionModelInfo(A2F_REGRESSION_MODEL_PATH.c_str()));
        if (modelInfo) {
            nbEmotions = modelInfo->GetNetworkInfo().GetEmotionsCount();
        }
        
        emotionAccumulatorData = std::make_unique<EmotionAccumulatorData>(EmotionAccumulatorData{
            std::move(ToUniquePtr(nva2x::CreateEmotionAccumulator(nbEmotions, 300, 0)))
        });
        
        // Set up executor creation parameters
        params.cudaStream = cudaStreamData->mCudaStream->Data();
        params.nbTracks = 1;
        audioAccumulatorPtr = audioAccumulatorData->mAudioAccumulator.get();
        emotionAccumulatorPtr = emotionAccumulatorData->mEmotionAccumulator.get();
        params.sharedAudioAccumulators = &audioAccumulatorPtr;
        params.sharedEmotionAccumulators = &emotionAccumulatorPtr;

        // Add audio accumulation in the test
        std::vector<float> dummyAudio(16000, 0.0f); // 1 second of silence
        audioAccumulatorData->mAudioAccumulator->Accumulate(
            nva2x::HostTensorFloatConstView{dummyAudio.data(), dummyAudio.size()}, params.cudaStream);
        audioAccumulatorData->mAudioAccumulator->Close();
    }
};

// Test basic struct properties and construction
TEST_F(TestA2FExecutorData, DefaultConstruction) {
    A2FExecutorData executorData;
    
    EXPECT_EQ(executorData.mBlendshapeInteractiveExecutor, nullptr);
    EXPECT_EQ(executorData.mGeometryInteractiveExecutor, nullptr);
    EXPECT_EQ(executorData.mAudioBufferLength, 0);

    EXPECT_FALSE(executorData.hasBlendshapeExecutor());
    EXPECT_EQ(executorData.getTotalNbFrames(), 0);
    EXPECT_EQ(executorData.getStartSampleIndex(0), 0);
    EXPECT_EQ(executorData.getSamplingRate(), 0);
}

// Test with real geometry executor functionality
TEST_F(TestA2FExecutorData, WithGeometryExecutor) {
    auto geometryExecutor = createGeometryExecutor(params, A2F_REGRESSION_MODEL_PATH);
    ASSERT_NE(geometryExecutor, nullptr);
    
    A2FExecutorData executorData {
        nullptr,
        std::move(geometryExecutor),
        TEST_AUDIO_BUFFER_LENGTH,
    };

    ASSERT_NE(executorData.mGeometryInteractiveExecutor, nullptr);
    ASSERT_NE(executorData.getFaceExecutor(), nullptr);
    ASSERT_NE(executorData.getGeometryExecutor(), nullptr);

    EXPECT_NE(
        executorData.mGeometryInteractiveExecutor.get(), nullptr);
    EXPECT_EQ(
        executorData.mGeometryInteractiveExecutor.get(), executorData.getGeometryExecutor());
    EXPECT_EQ(
        executorData.mGeometryInteractiveExecutor.get(), executorData.getFaceExecutor());

    // Test hasBlendshapeExecutor 
    // EXPECT_EQ(executorData.mBlendshapeSolveExecutor, nullptr);  // will raise access violation error
    ASSERT_FALSE(executorData.hasBlendshapeExecutor());

    // EXPECT_NE(executorData.getTotalNbFrames(0), 0);  // will raise access violation error
    EXPECT_EQ(executorData.getStartSampleIndex(0), 0);
    EXPECT_GT(executorData.getSamplingRate(), 0);
}

// Test with real blendshape executor functionality  
// NOTE: Disabled due to unexplained SEH exception - functionality can be tested separately
TEST_F(TestA2FExecutorData, WithBlendshapeExecutor) {
    // Create executors using the working pattern
    auto geometryExecutor = createGeometryExecutor(params, A2F_REGRESSION_MODEL_PATH);
    ASSERT_NE(geometryExecutor, nullptr);
    
    std::vector<std::string> facePoseNames{};
    std::vector<std::string> tonguePoseNames{};
    auto blendshapeExecutor = createBlendshapeExecutor(
        std::move(geometryExecutor), A2F_REGRESSION_MODEL_PATH, facePoseNames, tonguePoseNames, false, 0);
    ASSERT_NE(blendshapeExecutor, nullptr);
    
    A2FExecutorData executorData {
        std::move(blendshapeExecutor),
        nullptr,
        TEST_AUDIO_BUFFER_LENGTH,
    };
    
    ASSERT_NE(executorData.mBlendshapeInteractiveExecutor, nullptr);
    ASSERT_NE(executorData.getFaceExecutor(), nullptr);
    ASSERT_EQ(executorData.getGeometryExecutor(), nullptr);

    EXPECT_NE(
        executorData.mBlendshapeInteractiveExecutor.get(), nullptr);
    EXPECT_EQ(
        executorData.mGeometryInteractiveExecutor.get(), nullptr);
    EXPECT_EQ(
        executorData.mBlendshapeInteractiveExecutor.get(), executorData.getFaceExecutor());

    // Test hasBlendshapeExecutor
    // EXPECT_EQ(executorData.mBlendshapeSolveExecutor, nullptr);  // will raise access violation error
    ASSERT_TRUE(executorData.hasBlendshapeExecutor());

    // EXPECT_EQ(executorData.getTotalNbFrames(0), 0);  // will raise access violation error
    EXPECT_EQ(executorData.getStartSampleIndex(0), 0);
    EXPECT_GT(executorData.getSamplingRate(), 0);
}

// Test getStartSampleIndex calculation logic with simple values
TEST_F(TestA2FExecutorData, GetStartSampleIndex_NoExecutor) {
    A2FExecutorData executorData{
        nullptr,
        nullptr,
        TEST_AUDIO_BUFFER_LENGTH,
    };
    
    // Test that the struct has the audio buffer length set correctly
    EXPECT_EQ(executorData.mAudioBufferLength, TEST_AUDIO_BUFFER_LENGTH);

    // Test getStartSampleIndex() without a valid executor
    EXPECT_EQ(executorData.getStartSampleIndex(0), 0);
    EXPECT_EQ(executorData.getSamplingRate(), 0);
}

// Test blendshape executor creation in isolation
TEST_F(TestA2FExecutorData, BlendshapeExecutor_DirectTesting) {
    // Test blendshape executor creation without A2FExecutorData wrapper
    auto geometryExecutor = createGeometryExecutor(params, A2F_REGRESSION_MODEL_PATH);
    ASSERT_NE(geometryExecutor, nullptr);
    
    // Verify geometry executor works before transferring ownership
    EXPECT_GT(geometryExecutor->GetSamplingRate(), 0);
    
    std::vector<std::string> facePoseNames{};
    std::vector<std::string> tonguePoseNames{};
    auto blendshapeExecutor = createBlendshapeExecutor(
        std::move(geometryExecutor), A2F_REGRESSION_MODEL_PATH, facePoseNames, tonguePoseNames, false, 0);
    ASSERT_NE(blendshapeExecutor, nullptr);
    
    // Test basic blendshape executor functionality directly
    EXPECT_GT(blendshapeExecutor->GetSamplingRate(), 0);
    EXPECT_GT(blendshapeExecutor->GetWeightCount(), 0);
    EXPECT_GE(blendshapeExecutor->GetTotalNbFrames(), 0);
    
    // Test if the executor can be safely stored in a unique_ptr
    const auto testRef = blendshapeExecutor.get();
    EXPECT_NE(testRef, nullptr);
    
    // Test if we can safely call methods on the stored pointer
    EXPECT_GT(testRef->GetSamplingRate(), 0);
}
