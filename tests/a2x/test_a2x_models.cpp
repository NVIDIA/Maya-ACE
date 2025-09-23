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

using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::SetArgPointee;
using ::testing::WithArg;

// Mock class for Audio2FaceModelInfo
class MockAudio2FaceModelInfo : public Audio2FaceModelInfo {
public:
    MockAudio2FaceModelInfo() : Audio2FaceModelInfo("") {}
    
    MOCK_METHOD(bool, isValid, (), (const, override));
    MOCK_METHOD(bool, isRegression, (), (const, override));
    MOCK_METHOD(const rapidjson::Document&, getModelDoc, (), (const, override));
};

// Helper function to create JSON document from string
rapidjson::Document createJsonDocument(const std::string& jsonString) {
    rapidjson::Document doc;
    doc.Parse(jsonString.c_str());
    return doc;
}

// ==========================
// A2XModelsTests
// ==========================

class TestAudio2FaceModelInfo : public ::testing::Test
{
protected:
    std::string a2e_model_path;
    std::string a2f_diffusion_model_path;
    std::string a2f_regression_model_path;
    std::string audio_file_path;

    void SetUp() override {
        // Set up test data path
        a2e_model_path = get_test_data_path(A2E_MODEL_PATH);
        a2f_diffusion_model_path = get_test_data_path(A2F_DIFFUSION_MODEL_PATH);
        a2f_regression_model_path = get_test_data_path(A2F_REGRESSION_MODEL_PATH);
        audio_file_path = get_test_data_path(AUDIO_FILE_PATH);
    }
};

TEST_F(TestAudio2FaceModelInfo, Constructor) {
    // Test with regression model
    {
        Audio2FaceModelInfo modelInfo(a2f_regression_model_path);
        ASSERT_TRUE(modelInfo.isValid());
        ASSERT_TRUE(modelInfo.isRegression());
    }
    
    // Test with diffusion model
    {
        Audio2FaceModelInfo modelInfo(a2f_diffusion_model_path);
        ASSERT_TRUE(modelInfo.isValid());
        ASSERT_FALSE(modelInfo.isRegression());
    }
    
    // Test with invalid path
    {
        Audio2FaceModelInfo modelInfo("invalid_path.json");
        ASSERT_FALSE(modelInfo.isValid());
    }
    
    // Test with non-model file
    {
        Audio2FaceModelInfo modelInfo(audio_file_path);
        ASSERT_FALSE(modelInfo.isValid());
    }
}

TEST_F(TestAudio2FaceModelInfo, IsValid) {
    // Valid regression model
    {
        Audio2FaceModelInfo modelInfo(a2f_regression_model_path);
        ASSERT_TRUE(modelInfo.isValid());
    }
    
    // Valid diffusion model
    {
        Audio2FaceModelInfo modelInfo(a2f_diffusion_model_path);
        ASSERT_TRUE(modelInfo.isValid());
    }
    
    // Invalid model path
    {
        Audio2FaceModelInfo modelInfo("nonexistent.json");
        ASSERT_FALSE(modelInfo.isValid());
    }
}

TEST_F(TestAudio2FaceModelInfo, IsRegression) {
    // Regression model should return true
    {
        Audio2FaceModelInfo modelInfo(a2f_regression_model_path);
        ASSERT_TRUE(modelInfo.isRegression());
    }
    
    // Diffusion model should return false
    {
        Audio2FaceModelInfo modelInfo(a2f_diffusion_model_path);
        ASSERT_FALSE(modelInfo.isRegression());
    }
    
    // Invalid model should return false
    {
        Audio2FaceModelInfo modelInfo("invalid.json");
        ASSERT_FALSE(modelInfo.isRegression());
    }
}

TEST_F(TestAudio2FaceModelInfo, EmotionNames) {
    // Test regression model emotions
    {
        Audio2FaceModelInfo modelInfo(a2f_regression_model_path);
        auto emotionNames = modelInfo.getEmotionNames();
        ASSERT_GT(emotionNames.size(), 0); // Should have emotions
        // Verify emotion names are not empty
        for (const auto& name : emotionNames) {
            ASSERT_FALSE(name.empty());
        }
    }
    
    // Test diffusion model emotions
    {
        Audio2FaceModelInfo modelInfo(a2f_diffusion_model_path);
        auto emotionNames = modelInfo.getEmotionNames();
        ASSERT_GT(emotionNames.size(), 0); // Should have emotions
    }
    
    // Test invalid model
    {
        Audio2FaceModelInfo modelInfo("invalid.json");
        auto emotionNames = modelInfo.getEmotionNames();
        ASSERT_EQ(emotionNames.size(), 0); // Should be empty
    }
}

TEST_F(TestAudio2FaceModelInfo, EmotionCount) {
    // Test regression model emotion count
    {
        Audio2FaceModelInfo modelInfo(a2f_regression_model_path);
        size_t emotionCount = modelInfo.getEmotionCount();
        ASSERT_GT(emotionCount, 0);
        
        // Verify count matches names
        auto emotionNames = modelInfo.getEmotionNames();
        ASSERT_EQ(emotionCount, emotionNames.size());
    }
    
    // Test diffusion model emotion count
    {
        Audio2FaceModelInfo modelInfo(a2f_diffusion_model_path);
        size_t emotionCount = modelInfo.getEmotionCount();
        ASSERT_GT(emotionCount, 0);
        
        // Verify count matches names
        auto emotionNames = modelInfo.getEmotionNames();
        ASSERT_EQ(emotionCount, emotionNames.size());
    }
    
    // Test invalid model
    {
        Audio2FaceModelInfo modelInfo("invalid.json");
        ASSERT_EQ(modelInfo.getEmotionCount(), -1);
    }
}

TEST_F(TestAudio2FaceModelInfo, IdentityNames) {
    // Regression models don't have identities
    {
        Audio2FaceModelInfo modelInfo(a2f_regression_model_path);
        auto identityNames = modelInfo.getIdentityNames();
        ASSERT_EQ(identityNames.size(), 0); // Should be empty for regression
    }
    
    // Diffusion models should have identities
    {
        Audio2FaceModelInfo modelInfo(a2f_diffusion_model_path);
        auto identityNames = modelInfo.getIdentityNames();
        ASSERT_GT(identityNames.size(), 0); // Should have identities
        // Verify identity names are not empty
        for (const auto& name : identityNames) {
            ASSERT_FALSE(name.empty());
        }
    }
    
    // Invalid model
    {
        Audio2FaceModelInfo modelInfo("invalid.json");
        auto identityNames = modelInfo.getIdentityNames();
        ASSERT_EQ(identityNames.size(), 0);
    }
}

TEST_F(TestAudio2FaceModelInfo, IdentityCount) {
    // Regression models don't have identities
    {
        Audio2FaceModelInfo modelInfo(a2f_regression_model_path);
        ASSERT_EQ(modelInfo.getIdentityCount(), 0);
    }
    
    // Diffusion models should have identities
    {
        Audio2FaceModelInfo modelInfo(a2f_diffusion_model_path);
        size_t identityCount = modelInfo.getIdentityCount();
        ASSERT_GT(identityCount, 0);
        
        // Verify count matches names
        auto identityNames = modelInfo.getIdentityNames();
        ASSERT_EQ(identityCount, identityNames.size());
    }
    
    // Invalid model
    {
        Audio2FaceModelInfo modelInfo("invalid.json");
        ASSERT_EQ(modelInfo.getIdentityCount(), -1);
    }
}

TEST_F(TestAudio2FaceModelInfo, AudioBufferLength) {
    // Test regression model buffer length
    {
        Audio2FaceModelInfo modelInfo(a2f_regression_model_path);
        size_t bufferLength = modelInfo.getAudioBufferLength();
        ASSERT_GT(bufferLength, 0); // Should have a valid buffer length
    }
    
    // Test diffusion model buffer length
    {
        Audio2FaceModelInfo modelInfo(a2f_diffusion_model_path);
        size_t bufferLength = modelInfo.getAudioBufferLength();
        ASSERT_GT(bufferLength, 0); // Should have a valid buffer length
    }
    
    // Invalid model
    {
        Audio2FaceModelInfo modelInfo("invalid.json");
        ASSERT_EQ(modelInfo.getAudioBufferLength(), -1);
    }
}

TEST_F(TestAudio2FaceModelInfo, Documents) {
    // Test valid model documents
    {
        Audio2FaceModelInfo modelInfo(a2f_regression_model_path);
        const auto& modelDoc = modelInfo.getModelDoc();
        const auto& networkDoc = modelInfo.getNetworkInfoDoc();
        
        ASSERT_FALSE(modelDoc.IsNull());
        ASSERT_FALSE(networkDoc.IsNull());
        ASSERT_TRUE(modelDoc.HasMember("networkInfoPath"));
    }
    
    // Test invalid model documents
    {
        Audio2FaceModelInfo modelInfo("invalid.json");
        const auto& modelDoc = modelInfo.getModelDoc();
        const auto& networkDoc = modelInfo.getNetworkInfoDoc();
        
        ASSERT_TRUE(modelDoc.IsNull());
        ASSERT_TRUE(networkDoc.IsNull());
    }
}

TEST_F(TestAudio2FaceModelInfo, TestReadJson) {
    // Test reading valid JSON
    {
        auto doc = readJson(a2f_regression_model_path);
        ASSERT_FALSE(doc.IsNull());
        ASSERT_TRUE(doc.IsObject());
    }
    
    // Test reading invalid path
    {
        auto doc = readJson("nonexistent.json");
        ASSERT_TRUE(doc.IsNull());
    }
}

TEST_F(TestAudio2FaceModelInfo, TestReadA2FModelInfo) {
    // true
    auto regModelInfo = ToUniquePtr(
        nva2f::ReadRegressionModelInfo(a2f_regression_model_path.c_str()));
    ASSERT_NE(regModelInfo, nullptr);

    auto diffModelInfo = ToUniquePtr(
        nva2f::ReadDiffusionModelInfo(a2f_diffusion_model_path.c_str()));
    ASSERT_NE(diffModelInfo, nullptr);

    // false
    diffModelInfo = ToUniquePtr(
        nva2f::ReadDiffusionModelInfo(a2f_regression_model_path.c_str()));
    ASSERT_EQ(diffModelInfo, nullptr);

    regModelInfo = ToUniquePtr(
        nva2f::ReadRegressionModelInfo(a2f_diffusion_model_path.c_str()));
    ASSERT_EQ(regModelInfo, nullptr);

    // not found
    regModelInfo = ToUniquePtr(
        nva2f::ReadRegressionModelInfo(a2e_model_path.c_str()));
    ASSERT_EQ(regModelInfo, nullptr);
}

TEST_F(TestAudio2FaceModelInfo, GetBlendshapeSolveExecutorCreationParameters_RegressionModel) {
    // Test with valid regression model
    {
        Audio2FaceModelInfo modelInfo(a2f_regression_model_path);
        auto params = modelInfo.getBlendshapeSolveExecutorCreationParameters();
        
        // Verify we got valid parameters (non-default constructed)
        ASSERT_NE(params.initializationSkinParams, nullptr);
        ASSERT_NE(params.initializationTongueParams, nullptr);

        // The exact verification depends on the structure of BlendshapeSolveExecutorCreationParameters
        // For regression models, identity index should be ignored
        auto params2 = modelInfo.getBlendshapeSolveExecutorCreationParameters(1);
        ASSERT_NE(params2.initializationSkinParams, nullptr);
        ASSERT_NE(params2.initializationTongueParams, nullptr);
    }
    // Test with invalid regression model, but valid blendshape config
    {
        Audio2FaceModelInfo modelInfo(a2f_diffusion_model_path);
        auto params = modelInfo.getBlendshapeSolveExecutorCreationParameters();
        ASSERT_NE(params.initializationSkinParams, nullptr);
        ASSERT_NE(params.initializationTongueParams, nullptr);
    }
}

TEST_F(TestAudio2FaceModelInfo, GetBlendshapeSolveExecutorCreationParameters_DiffusionModel) {
    // Test with valid diffusion model - default identity
    {
        Audio2FaceModelInfo modelInfo(a2f_diffusion_model_path);
        auto params = modelInfo.getBlendshapeSolveExecutorCreationParameters();
        
        // Default identity index is 0
        ASSERT_NE(params.initializationSkinParams, nullptr);
        ASSERT_NE(params.initializationTongueParams, nullptr);
    }
    // Test with valid diffusion model - specific identity
    {
        Audio2FaceModelInfo modelInfo(a2f_diffusion_model_path);
        auto params1 = modelInfo.getBlendshapeSolveExecutorCreationParameters(0);

        ASSERT_NE(params1.initializationSkinParams, nullptr);
        ASSERT_NE(params1.initializationTongueParams, nullptr);
    }
    // Test with invalid diffusion model, but valid blendshape config
    {
        Audio2FaceModelInfo modelInfo(a2f_regression_model_path);
        auto params1 = modelInfo.getBlendshapeSolveExecutorCreationParameters(0);

        ASSERT_NE(params1.initializationSkinParams, nullptr);
        ASSERT_NE(params1.initializationTongueParams, nullptr);
    }
}

TEST_F(TestAudio2FaceModelInfo, GetBlendshapeSolveExecutorCreationParameters_InvalidModel) {
    // Test with invalid model
    {
        Audio2FaceModelInfo modelInfo("invalid.json");
        auto params = modelInfo.getBlendshapeSolveExecutorCreationParameters();
        
        // Should return empty/default parameters for invalid model
        // The method returns {} when model is not valid
        ASSERT_EQ(params.initializationSkinParams, nullptr);
        ASSERT_EQ(params.initializationTongueParams, nullptr);
    }
    
    // Test with non-existent file
    {
        Audio2FaceModelInfo modelInfo("nonexistent.json");
        auto params = modelInfo.getBlendshapeSolveExecutorCreationParameters();
        
        // Should return empty/default parameters
        ASSERT_EQ(params.initializationSkinParams, nullptr);
        ASSERT_EQ(params.initializationTongueParams, nullptr);
    }
}

TEST_F(TestAudio2FaceModelInfo, GetBlendshapeSolveExecutorCreationParameters_EdgeCases) {
    // Test with model that might not have blendshape info
    {
        // Using audio file path which is not a valid model
        Audio2FaceModelInfo modelInfo(audio_file_path);
        auto params = modelInfo.getBlendshapeSolveExecutorCreationParameters();
        
        // Should return empty/default parameters
        ASSERT_EQ(params.initializationSkinParams, nullptr);
        ASSERT_EQ(params.initializationTongueParams, nullptr);
    }
}

TEST_F(TestAudio2FaceModelInfo, HasBlendshapeInfo_InvalidModel) {
    // Test with invalid model path
    {
        Audio2FaceModelInfo modelInfo("invalid.json");
        ASSERT_FALSE(modelInfo.hasBlendshapeInfo(0));
        ASSERT_FALSE(modelInfo.hasBlendshapeInfo(1));
    }
    
    // Test with non-model file
    {
        Audio2FaceModelInfo modelInfo(audio_file_path);
        ASSERT_FALSE(modelInfo.hasBlendshapeInfo(0));
    }
}

TEST_F(TestAudio2FaceModelInfo, HasBlendshapeInfo_ValidRegressionStructure) {
    const std::string validRegressionJson = R"({
        "networkInfoPath": "network_info.json",
        "blendshapePaths": {
            "skin": {
                "config": "bs_skin_config.json",
                "data": "bs_skin.npz"
            },
            "tongue": {
                "config": "bs_tongue_config.json", 
                "data": "bs_tongue.npz"
            }
        }
    })";
    
    const std::string validNetworkInfoJson = R"({
        "id": {
            "type": "regression"
        },
        "params": {
            "explicit_emotions": ["happy", "sad", "angry"]
        },
        "audio_params": {
            "buffer_len": 1024
        }
    })";
    
    // Write temporary files
    std::string tempModelPath = "temp_regression_model.json";
    std::string tempNetworkInfoPath = "network_info.json";
    
    std::ofstream modelFile(tempModelPath);
    modelFile << validRegressionJson;
    modelFile.close();
    
    std::ofstream networkFile(tempNetworkInfoPath);
    networkFile << validNetworkInfoJson;
    networkFile.close();
    
    Audio2FaceModelInfo modelInfo(tempModelPath);
    ASSERT_TRUE(modelInfo.isValid());
    ASSERT_TRUE(modelInfo.isRegression());
    ASSERT_TRUE(modelInfo.hasBlendshapeInfo(0));
    
    // Cleanup
    std::remove(tempModelPath.c_str());
    std::remove(tempNetworkInfoPath.c_str());
}

TEST_F(TestAudio2FaceModelInfo, HasBlendshapeInfo_ValidDiffusionStructure) {
    const std::string validDiffusionJson = R"({
        "networkInfoPath": "network_info.json",
        "blendshapePaths": [
            {
                "skin": {
                    "config": "bs_skin_config_Claire.json",
                    "data": "bs_skin_Claire.npz"
                },
                "tongue": {
                    "config": "bs_tongue_config_Claire.json",
                    "data": "bs_tongue_Claire.npz"
                }
            },
            {
                "skin": {
                    "config": "bs_skin_config_James.json",
                    "data": "bs_skin_James.npz"
                },
                "tongue": {
                    "config": "bs_tongue_config_James.json",
                    "data": "bs_tongue_James.npz"
                }
            }
        ]
    })";
    
    const std::string validDiffusionNetworkInfoJson = R"({
        "id": {
            "type": "diffusion"
        },
        "params": {
            "emotions": ["happy", "sad", "angry"],
            "identities": ["Claire", "James"]
        },
        "audio_params": {
            "buffer_len": 1024
        }
    })";
    
    // Write temporary files
    std::string tempModelPath = "temp_diffusion_model.json";
    std::string tempNetworkInfoPath = "network_info.json";
    
    std::ofstream modelFile(tempModelPath);
    modelFile << validDiffusionJson;
    modelFile.close();
    
    std::ofstream networkFile(tempNetworkInfoPath);
    networkFile << validDiffusionNetworkInfoJson;
    networkFile.close();
    
    Audio2FaceModelInfo modelInfo(tempModelPath);
    ASSERT_TRUE(modelInfo.isValid());
    ASSERT_FALSE(modelInfo.isRegression());
    ASSERT_TRUE(modelInfo.hasBlendshapeInfo(0)); // Claire
    ASSERT_TRUE(modelInfo.hasBlendshapeInfo(1)); // James
    ASSERT_FALSE(modelInfo.hasBlendshapeInfo(2)); // Out of bounds
    ASSERT_FALSE(modelInfo.hasBlendshapeInfo(-1)); // Invalid index
    
    // Cleanup
    std::remove(tempModelPath.c_str());
    std::remove(tempNetworkInfoPath.c_str());
}

TEST_F(TestAudio2FaceModelInfo, HasBlendshapeInfo_MissingBlendshapePaths) {
    const std::string missingBlendshapeJson = R"({
        "networkInfoPath": "network_info.json"
    })";
    
    const std::string networkInfoJson = R"({
        "id": {
            "type": "regression"
        },
        "params": {
            "explicit_emotions": ["happy"]
        },
        "audio_params": {
            "buffer_len": 1024
        }
    })";
    
    std::string tempModelPath = "temp_missing_bs_model.json";
    std::string tempNetworkInfoPath = "network_info.json";
    
    std::ofstream modelFile(tempModelPath);
    modelFile << missingBlendshapeJson;
    modelFile.close();
    
    std::ofstream networkFile(tempNetworkInfoPath);
    networkFile << networkInfoJson;
    networkFile.close();
    
    Audio2FaceModelInfo modelInfo(tempModelPath);
    ASSERT_TRUE(modelInfo.isValid());
    ASSERT_FALSE(modelInfo.hasBlendshapeInfo(0)); // Should be false due to missing blendshapePaths
    
    // Cleanup
    std::remove(tempModelPath.c_str());
    std::remove(tempNetworkInfoPath.c_str());
}

TEST_F(TestAudio2FaceModelInfo, HasBlendshapeInfo_MissingSkin) {
    // Create mock object
    MockAudio2FaceModelInfo mockModelInfo;
    
    // Create JSON document for model with missing skin
    const std::string missingSkinJson = R"({
        "networkInfoPath": "network_info.json",
        "blendshapePaths": {
            "tongue": {
                "config": "bs_tongue_config.json",
                "data": "bs_tongue.npz"
            }
        }
    })";
    
    auto modelDoc = createJsonDocument(missingSkinJson);
    
    // Set up expectations
    EXPECT_CALL(mockModelInfo, isValid())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(mockModelInfo, isRegression())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(mockModelInfo, getModelDoc())
        .WillRepeatedly(testing::ReturnRef(modelDoc));
    
    // Test the behavior
    ASSERT_FALSE(mockModelInfo.hasBlendshapeInfo(0)); // Should be false due to missing skin
}

TEST_F(TestAudio2FaceModelInfo, HasBlendshapeInfo_MissingConfig) {
    // Create mock object
    MockAudio2FaceModelInfo mockModelInfo;
    
    // Create JSON document for model with missing config
    const std::string missingConfigJson = R"({
        "networkInfoPath": "network_info.json",
        "blendshapePaths": {
            "skin": {
                "data": "bs_skin.npz"
            },
            "tongue": {
                "config": "bs_tongue_config.json",
                "data": "bs_tongue.npz"
            }
        }
    })";
    
    auto modelDoc = createJsonDocument(missingConfigJson);
    
    // Set up expectations
    EXPECT_CALL(mockModelInfo, isValid())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(mockModelInfo, isRegression())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(mockModelInfo, getModelDoc())
        .WillRepeatedly(testing::ReturnRef(modelDoc));
    
    // Test the behavior
    ASSERT_FALSE(mockModelInfo.hasBlendshapeInfo(0)); // Should be false due to missing config
}

TEST_F(TestAudio2FaceModelInfo, HasBlendshapeInfo_WrongStructureType) {
    // Create mock object
    MockAudio2FaceModelInfo mockModelInfo;
    
    // Create JSON document for regression model with array structure (should be object)
    const std::string wrongStructureJson = R"({
        "networkInfoPath": "network_info.json",
        "blendshapePaths": [
            {
                "skin": {
                    "config": "bs_skin_config.json",
                    "data": "bs_skin.npz"
                },
                "tongue": {
                    "config": "bs_tongue_config.json",
                    "data": "bs_tongue.npz"
                }
            }
        ]
    })";
    
    auto modelDoc = createJsonDocument(wrongStructureJson);
    
    // Set up expectations
    EXPECT_CALL(mockModelInfo, isValid())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(mockModelInfo, isRegression())
        .WillRepeatedly(Return(true)); // Regression model should expect object, not array
    EXPECT_CALL(mockModelInfo, getModelDoc())
        .WillRepeatedly(testing::ReturnRef(modelDoc));
    
    // Test the behavior
    ASSERT_FALSE(mockModelInfo.hasBlendshapeInfo(0)); // Should be false due to wrong structure type
}

TEST_F(TestAudio2FaceModelInfo, HasBlendshapeInfo_EmptyArray) {
    // Create mock object
    MockAudio2FaceModelInfo mockModelInfo;
    
    // Create JSON document for diffusion model with empty array
    const std::string emptyArrayJson = R"({
        "networkInfoPath": "network_info.json",
        "blendshapePaths": []
    })";
    
    auto modelDoc = createJsonDocument(emptyArrayJson);
    
    // Set up expectations
    EXPECT_CALL(mockModelInfo, isValid())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(mockModelInfo, isRegression())
        .WillRepeatedly(Return(false)); // Diffusion model
    EXPECT_CALL(mockModelInfo, getModelDoc())
        .WillRepeatedly(testing::ReturnRef(modelDoc));
    
    // Test the behavior
    ASSERT_FALSE(mockModelInfo.hasBlendshapeInfo(0)); // Should be false due to empty array
}

TEST_F(TestAudio2FaceModelInfo, HasBlendshapeInfo_ValidRegressionModel) {
    // Create mock object
    MockAudio2FaceModelInfo mockModelInfo;
    
    // Create JSON document for valid regression model
    const std::string validRegressionJson = R"({
        "networkInfoPath": "network_info.json",
        "blendshapePaths": {
            "skin": {
                "config": "bs_skin_config.json",
                "data": "bs_skin.npz"
            },
            "tongue": {
                "config": "bs_tongue_config.json", 
                "data": "bs_tongue.npz"
            }
        }
    })";
    
    auto modelDoc = createJsonDocument(validRegressionJson);
    
    // Set up expectations
    EXPECT_CALL(mockModelInfo, isValid())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(mockModelInfo, isRegression())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(mockModelInfo, getModelDoc())
        .WillRepeatedly(testing::ReturnRef(modelDoc));
    
    // Test the behavior - should return true for valid structure
    ASSERT_TRUE(mockModelInfo.hasBlendshapeInfo(0));
    ASSERT_TRUE(mockModelInfo.hasBlendshapeInfo(1)); // Identity index ignored for regression
}

TEST_F(TestAudio2FaceModelInfo, HasBlendshapeInfo_ValidDiffusionModel) {
    // Create mock object
    MockAudio2FaceModelInfo mockModelInfo;
    
    // Create JSON document for valid diffusion model
    const std::string validDiffusionJson = R"({
        "networkInfoPath": "network_info.json",
        "blendshapePaths": [
            {
                "skin": {
                    "config": "bs_skin_config_Claire.json",
                    "data": "bs_skin_Claire.npz"
                },
                "tongue": {
                    "config": "bs_tongue_config_Claire.json",
                    "data": "bs_tongue_Claire.npz"
                }
            },
            {
                "skin": {
                    "config": "bs_skin_config_James.json",
                    "data": "bs_skin_James.npz"
                },
                "tongue": {
                    "config": "bs_tongue_config_James.json",
                    "data": "bs_tongue_James.npz"
                }
            }
        ]
    })";
    
    auto modelDoc = createJsonDocument(validDiffusionJson);
    
    // Set up expectations
    EXPECT_CALL(mockModelInfo, isValid())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(mockModelInfo, isRegression())
        .WillRepeatedly(Return(false)); // Diffusion model
    EXPECT_CALL(mockModelInfo, getModelDoc())
        .WillRepeatedly(testing::ReturnRef(modelDoc));
    
    // Test the behavior
    ASSERT_TRUE(mockModelInfo.hasBlendshapeInfo(0)); // Claire - valid index
    ASSERT_TRUE(mockModelInfo.hasBlendshapeInfo(1)); // James - valid index
    ASSERT_FALSE(mockModelInfo.hasBlendshapeInfo(2)); // Out of bounds
    ASSERT_FALSE(mockModelInfo.hasBlendshapeInfo(-1)); // Invalid index
}
