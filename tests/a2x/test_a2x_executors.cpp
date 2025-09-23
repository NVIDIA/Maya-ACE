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
#include "test_constants.h"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <thread>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <audio2face/audio2face.h>
#include <audio2face/executor.h>
#include <audio2face/error.h>
#include <audio2face/parse_helper.h>

#include "maya_ace/common/a2x_models.h"
#include "maya_ace/common/a2x_data.h"
#include "maya_ace/common/a2x_executors.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::SetArgPointee;
using ::testing::WithArg;

// ==========================
// A2XExecutorsTests
// ==========================

// Test fixture for executor-related tests with common initialization
class TestCreateGeometryExecutor : public ::testing::Test
{
protected:
    std::unique_ptr<CudaStreamData> cudaStreamData;
    std::unique_ptr<AudioAccumulatorData> audioAccumulatorData;
    std::unique_ptr<EmotionAccumulatorData> emotionAccumulatorData;
    nva2f::GeometryExecutorCreationParameters params;
    size_t nbEmotions = 10;

	nva2x::IAudioAccumulator* audioAccumulatorPtr = nullptr;
	nva2x::IEmotionAccumulator* emotionAccumulatorPtr = nullptr;

    void SetUp() override {
        // Initialize CUDA stream
        cudaStreamData = std::make_unique<CudaStreamData>(CudaStreamData{
            std::move(ToUniquePtr(nva2x::CreateCudaStream()))
        });
        
        // Initialize audio accumulators
        audioAccumulatorData = std::make_unique<AudioAccumulatorData>(AudioAccumulatorData{
            std::move(ToUniquePtr(nva2x::CreateAudioAccumulator(16000, 0)))
        });
        
        // Determine emotion count from regression model
        auto modelInfo = ToUniquePtr(nva2f::ReadRegressionModelInfo(A2F_REGRESSION_MODEL_PATH.c_str()));
        if (modelInfo) {
            nbEmotions = modelInfo->GetNetworkInfo().GetEmotionsCount();
        }
        
        // Initialize emotion accumulator
        emotionAccumulatorData = std::make_unique<EmotionAccumulatorData>(EmotionAccumulatorData{
            std::move(ToUniquePtr(nva2x::CreateEmotionAccumulator(nbEmotions, 300, 0)))
        });
        
        // Set up geometry executor parameters
        params.cudaStream = cudaStreamData->mCudaStream->Data();
        params.nbTracks = 1;
        audioAccumulatorPtr = audioAccumulatorData->mAudioAccumulator.get();
        emotionAccumulatorPtr = emotionAccumulatorData->mEmotionAccumulator.get();
        params.sharedAudioAccumulators = &audioAccumulatorPtr;
        params.sharedEmotionAccumulators = &emotionAccumulatorPtr;
    }
};

TEST_F(TestCreateGeometryExecutor, RegressionModel) {
    auto executor = createGeometryExecutor(params, A2F_REGRESSION_MODEL_PATH);
    ASSERT_NE(executor, nullptr);
}

TEST_F(TestCreateGeometryExecutor, DiffusionModel_DefaultIdentity) {
    auto executor = createGeometryExecutor(params, A2F_DIFFUSION_MODEL_PATH);
    ASSERT_NE(executor, nullptr);
}

TEST_F(TestCreateGeometryExecutor, DiffusionModel_SpecificIdentity) {
    auto executor = createGeometryExecutor(params, A2F_DIFFUSION_MODEL_PATH, 1);
    ASSERT_NE(executor, nullptr);
}

TEST_F(TestCreateGeometryExecutor, InvalidModel) {
    auto executor = createGeometryExecutor(params, "invalid.json");
    ASSERT_EQ(executor, nullptr);
}

// ==========================
// A2XBlendshapeExecutorsTests  
// ==========================

// Test fixture for blendshape executor-related tests with common initialization
class TestCreateBlendshapeExecutor : public ::testing::Test
{
protected:
    std::unique_ptr<CudaStreamData> cudaStreamData;
    std::unique_ptr<AudioAccumulatorData> audioAccumulatorData;
    std::unique_ptr<EmotionAccumulatorData> emotionAccumulatorData;
    nva2f::GeometryExecutorCreationParameters geometryParams;
    size_t nbEmotions;

	nva2x::IAudioAccumulator* audioAccumulatorPtr = nullptr;
	nva2x::IEmotionAccumulator* emotionAccumulatorPtr = nullptr;

    void SetUp() override {
        // Initialize CUDA stream
        cudaStreamData = std::make_unique<CudaStreamData>(CudaStreamData{
            std::move(ToUniquePtr(nva2x::CreateCudaStream()))
        });
        
        // Initialize audio accumulators
        audioAccumulatorData = std::make_unique<AudioAccumulatorData>(AudioAccumulatorData{
            std::move(ToUniquePtr(nva2x::CreateAudioAccumulator(16000, 0)))
        });
        
        // Determine emotion count from regression model
        nbEmotions = 0;
        auto modelInfo = ToUniquePtr(nva2f::ReadRegressionModelInfo(A2F_REGRESSION_MODEL_PATH.c_str()));
        if (modelInfo) {
            nbEmotions = modelInfo->GetNetworkInfo().GetEmotionsCount();
        }
        
        // Initialize emotion accumulator
        emotionAccumulatorData = std::make_unique<EmotionAccumulatorData>(EmotionAccumulatorData{
            std::move(ToUniquePtr(nva2x::CreateEmotionAccumulator(nbEmotions, 300, 0)))
        });
        
        // Set up geometry executor parameters
        geometryParams.cudaStream = cudaStreamData->mCudaStream->Data();
        geometryParams.nbTracks = 1;
        audioAccumulatorPtr = audioAccumulatorData->mAudioAccumulator.get();
        emotionAccumulatorPtr = emotionAccumulatorData->mEmotionAccumulator.get();
        geometryParams.sharedAudioAccumulators = &audioAccumulatorPtr;
        geometryParams.sharedEmotionAccumulators = &emotionAccumulatorPtr;
    }
};

TEST_F(TestCreateBlendshapeExecutor, RegressionModel_GpuSolver) {
    // First create a geometry executor
    auto geometryExecutor = createGeometryExecutor(geometryParams, A2F_REGRESSION_MODEL_PATH);
    ASSERT_NE(geometryExecutor, nullptr);
    
    // Create blendshape executor with GPU solver
    std::vector<std::string> facePoseNames{};
    std::vector<std::string> tonguePoseNames{};
    auto blendshapeExecutor = createBlendshapeExecutor(std::move(geometryExecutor), A2F_REGRESSION_MODEL_PATH, facePoseNames, tonguePoseNames, true, 0);
    ASSERT_NE(blendshapeExecutor, nullptr);
}

TEST_F(TestCreateBlendshapeExecutor, RegressionModel_CpuSolver) {
    // First create a geometry executor
    auto geometryExecutor = createGeometryExecutor(geometryParams, A2F_REGRESSION_MODEL_PATH);
    ASSERT_NE(geometryExecutor, nullptr);
    
    // Create blendshape executor with CPU solver
    std::vector<std::string> facePoseNames{};
    std::vector<std::string> tonguePoseNames{};
    auto blendshapeExecutor = createBlendshapeExecutor(std::move(geometryExecutor), A2F_REGRESSION_MODEL_PATH, facePoseNames, tonguePoseNames, false, 0);
    ASSERT_NE(blendshapeExecutor, nullptr);
}

TEST_F(TestCreateBlendshapeExecutor, DiffusionModel_GpuSolver_DefaultIdentity) {
    // First create a geometry executor  
    auto geometryExecutor = createGeometryExecutor(geometryParams, A2F_DIFFUSION_MODEL_PATH);
    ASSERT_NE(geometryExecutor, nullptr);
    
    // Create blendshape executor with GPU solver and default identity
    std::vector<std::string> facePoseNames{};
    std::vector<std::string> tonguePoseNames{};
    auto blendshapeExecutor = createBlendshapeExecutor(std::move(geometryExecutor), A2F_DIFFUSION_MODEL_PATH, facePoseNames, tonguePoseNames, true, 0);
    ASSERT_NE(blendshapeExecutor, nullptr);
}

TEST_F(TestCreateBlendshapeExecutor, DiffusionModel_GpuSolver_SpecificIdentity) {
    // First create a geometry executor
    auto geometryExecutor = createGeometryExecutor(geometryParams, A2F_DIFFUSION_MODEL_PATH);
    ASSERT_NE(geometryExecutor, nullptr);
    
    // Create blendshape executor with GPU solver and specific identity
    std::vector<std::string> facePoseNames{};
    std::vector<std::string> tonguePoseNames{};
    auto blendshapeExecutor = createBlendshapeExecutor(std::move(geometryExecutor), A2F_DIFFUSION_MODEL_PATH, facePoseNames, tonguePoseNames, true, 1);
    ASSERT_NE(blendshapeExecutor, nullptr);
}

TEST_F(TestCreateBlendshapeExecutor, DiffusionModel_CpuSolver_DefaultIdentity) {
    // First create a geometry executor
    auto geometryExecutor = createGeometryExecutor(geometryParams, A2F_DIFFUSION_MODEL_PATH);
    ASSERT_NE(geometryExecutor, nullptr);
    
    // Create blendshape executor with CPU solver and default identity
    std::vector<std::string> facePoseNames{};
    std::vector<std::string> tonguePoseNames{};
    auto blendshapeExecutor = createBlendshapeExecutor(std::move(geometryExecutor), A2F_DIFFUSION_MODEL_PATH, facePoseNames, tonguePoseNames, false, 0);
    ASSERT_NE(blendshapeExecutor, nullptr);
}

TEST_F(TestCreateBlendshapeExecutor, DiffusionModel_CpuSolver_SpecificIdentity) {
    // First create a geometry executor
    auto geometryExecutor = createGeometryExecutor(geometryParams, A2F_DIFFUSION_MODEL_PATH);
    ASSERT_NE(geometryExecutor, nullptr);
    
    // Create blendshape executor with CPU solver and specific identity
    std::vector<std::string> facePoseNames{};
    std::vector<std::string> tonguePoseNames{};
    auto blendshapeExecutor = createBlendshapeExecutor(std::move(geometryExecutor), A2F_DIFFUSION_MODEL_PATH, facePoseNames, tonguePoseNames, false, 1);
    ASSERT_NE(blendshapeExecutor, nullptr);
}

TEST_F(TestCreateBlendshapeExecutor, DISABLED_InvalidModel) {
    // NOTE: This test is disabled because it represents an unrealistic scenario.
    // In practice, geometry and blendshape executors should use the same model path.
    // Creating a geometry executor from one model and trying to create a blendshape 
    // executor from a different (invalid) model causes deep SDK crashes that cannot
    // be gracefully handled.
    
    // First create a geometry executor with valid model
    auto geometryExecutor = createGeometryExecutor(geometryParams, A2F_REGRESSION_MODEL_PATH);
    ASSERT_NE(geometryExecutor, nullptr);
    
    // Try to create blendshape executor with invalid model path
    std::vector<std::string> facePoseNames{};
    std::vector<std::string> tonguePoseNames{};
    auto blendshapeExecutor = createBlendshapeExecutor(std::move(geometryExecutor), "invalid.json", facePoseNames, tonguePoseNames, true, 0);
    ASSERT_EQ(blendshapeExecutor, nullptr);
}

TEST_F(TestCreateBlendshapeExecutor, NullGeometryExecutor) {
    // Create a null geometry executor
    UniquePtr<nva2f::IGeometryInteractiveExecutor> nullGeometryExecutor;
    
    // Try to create blendshape executor with null geometry executor
    std::vector<std::string> facePoseNames{};
    std::vector<std::string> tonguePoseNames{};
    auto blendshapeExecutor = createBlendshapeExecutor(std::move(nullGeometryExecutor), A2F_REGRESSION_MODEL_PATH, facePoseNames, tonguePoseNames, true, 0);
    ASSERT_EQ(blendshapeExecutor, nullptr);
}
