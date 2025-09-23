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

#include "a2x_models.h"

#include <optional>
#include <string>
#include <filesystem>
#include <fstream>
#include <memory>
#include <utility>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include <audio2face/audio2face.h>


// Constructor
Audio2FaceModelInfo::Audio2FaceModelInfo(const std::string& modelPath) : modelPath(modelPath) {
    // Load model.json
    modelDoc = readJson(modelPath);
    
    auto convert_path = [&modelPath](const std::string& jsonPath) -> std::string {
        const auto u8Path = std::filesystem::u8path(jsonPath);
        if (u8Path.is_relative()) {
            return (std::filesystem::u8path(modelPath).parent_path() / u8Path).string();
        }
        else {
            return u8Path.string();
        }
    };
    
    // Load networkInfo.json if model.json was loaded successfully
    if (modelDoc.IsObject() && modelDoc.HasMember("networkInfoPath")) {
        // likely, regression model
        const auto networkInfoPath = modelDoc["networkInfoPath"].GetString();
        networkInfoDoc = readJson(convert_path(networkInfoPath));
    }
}

rapidjson::Document readJson(const std::string& modelPath) {
    std::ifstream ifs(modelPath);
    if (ifs.fail()) { 
        rapidjson::Document doc;
        return doc; // Return empty document on failure
    }
    
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document doc;
    doc.ParseStream(isw);

    return doc;
}

// Destructor
Audio2FaceModelInfo::~Audio2FaceModelInfo() {
    // rapidjson::Document destructor will handle cleanup
}

bool Audio2FaceModelInfo::isValid() const {
    if (modelDoc.IsNull() || networkInfoDoc.IsNull()) { 
        return false;
    }

    if (!networkInfoDoc.HasMember("id")) { return false; }

    const auto& id = networkInfoDoc["id"].GetObject();
    if (!id.HasMember("type")) { return false; }

    return true;
}

bool Audio2FaceModelInfo::isRegression() const {
    if (!isValid()) { return false; }
    
    const auto& id = networkInfoDoc["id"].GetObject();

    return std::string(id["type"].GetString()) == "regression";
}

std::vector<std::string> Audio2FaceModelInfo::getEmotionNames() const {
    std::vector<std::string> emotionNames;
    if (!isValid()) { return emotionNames; }
    // NOTE: reading through ModelInfo class takes seconds.
    //     To make it faster, we read from networkInfo.json instead.
    // if (isRegression()) {
    //     auto modelInfo = ToUniquePtr(nva2f::ReadRegressionModelInfo(modelPath.c_str()));
    //     if (!modelInfo) { return {}; }
    //     for(int i = 0 ; i < modelInfo->GetNetworkInfo().GetEmotionsCount(); ++i) {
    //         emotionNames.push_back(modelInfo->GetNetworkInfo().GetEmotionName(i));
    //     }
    // } else {
    //     auto modelInfo = ToUniquePtr(nva2f::ReadDiffusionModelInfo(modelPath.c_str()));
    //     if (!modelInfo) { return {}; }
    //     for(int i = 0 ; i < modelInfo->GetNetworkInfo().GetEmotionsCount(); ++i) {
    //         emotionNames.push_back(modelInfo->GetNetworkInfo().GetEmotionName(i));
    //     }
    // }
    const auto& doc = getNetworkInfoDoc();
    if (!doc.HasMember("params")) {
        return {};
    }
    const auto& params = doc["params"].GetObject();
    if (params.HasMember("explicit_emotions")) {
        // v2.x regression model
        const auto& names = params["explicit_emotions"].GetArray();
        for(int i = 0 ; i < names.Size(); ++i) {
            emotionNames.push_back(std::string(names[i].GetString()));
        }
    }
    else if (params.HasMember("emotions")) {
        // v3.0+ diffusion model
        const auto& names = params["emotions"].GetArray();
        for(int i = 0 ; i < names.Size(); ++i) {
            emotionNames.push_back(std::string(names[i].GetString()));
        }
    }
    return emotionNames;
}

int Audio2FaceModelInfo::getEmotionCount() const {
    if (!isValid()) { return -1; }

    const auto& emotions = getEmotionNames();
    return emotions.size();
}

std::vector<std::string> Audio2FaceModelInfo::getIdentityNames() const {
    std::vector<std::string> identityNames;
    
    if (!isValid()) { return {}; }

    if (isRegression()) { return {}; }
    
    // NOTE: reading through ModelInfo class takes seconds. 
    //     To make it faster, we read from networkInfo.json instead.
    // auto modelInfo = ToUniquePtr(nva2f::ReadDiffusionModelInfo(modelPath.c_str()));
    // if (!modelInfo) {
    //     return {};
    // }
    // for(int i = 0; i < modelInfo->GetNetworkInfo().GetIdentityLength(); ++i) {
    //     identityNames.push_back(modelInfo->GetNetworkInfo().GetIdentityName(i));
    // }
    const auto& doc = getNetworkInfoDoc();
    if (!doc.HasMember("params")) {
        return {};
    }
    const auto& params = doc["params"].GetObject();
    if (!params.HasMember("identities")) {
        return {};
    }
    const auto& names = params["identities"].GetArray();
    for(int i = 0 ; i < names.Size(); ++i) {
        identityNames.push_back(std::string(names[i].GetString()));
    }
    return identityNames;
}

int Audio2FaceModelInfo::getIdentityCount() const {
    if (!isValid()) { return -1; }

    if (isRegression()) { return 0; }

    const auto& names = getIdentityNames();
    return names.size();
}

int Audio2FaceModelInfo::getAudioBufferLength() const {
    if (!isValid()) { return -1; }
    
    // NOTE: reading through ModelInfo class takes seconds. 
    //     To make it faster, we read from networkInfo.json instead.
    // if (isRegression()) {
    //     auto modelInfo = ToUniquePtr(nva2f::ReadRegressionModelInfo(modelPath.c_str()));
    //     if (!modelInfo) {
    //         return -1;
    //     }
    //     return modelInfo->GetNetworkInfo().GetNetworkInfo().bufferLength;
    // } else {    
    //     auto modelInfo = ToUniquePtr(nva2f::ReadDiffusionModelInfo(modelPath.c_str()));
    //     if (!modelInfo) {
    //         return -1;
    //     }
    //     return modelInfo->GetNetworkInfo().GetNetworkInfo().bufferLength;
    // }
    const auto& doc = getNetworkInfoDoc();
    if (!doc.HasMember("audio_params")) {
        return -1;
    }
    const auto& audioParams = doc["audio_params"].GetObject();
    return audioParams["buffer_len"].GetInt();
}

const rapidjson::Document& Audio2FaceModelInfo::getModelDoc() const {
    return modelDoc;
}

const rapidjson::Document& Audio2FaceModelInfo::getNetworkInfoDoc() const {
    return networkInfoDoc;
}

nva2f::BlendshapeSolveExecutorCreationParameters 
    Audio2FaceModelInfo::getBlendshapeSolveExecutorCreationParameters(int identityIndex) const
{
    // Get blendshape parameters
    if (!isValid()) { return {}; }

    bool hasBlendshapeModelInfo = false;
    nva2f::BlendshapeSolveExecutorCreationParameters blendshapeParams;
    if (isRegression()) {
        auto blendshapeModelInfo = ToUniquePtr(
            nva2f::ReadRegressionBlendshapeSolveModelInfo(modelPath.c_str()));
        if (!blendshapeModelInfo) {
            return {};
        }
        blendshapeParams = blendshapeModelInfo->GetExecutorCreationParameters(
            nva2f::IGeometryExecutor::ExecutionOption::All);
        hasBlendshapeModelInfo = true;
    } else {
        auto blendshapeModelInfo = ToUniquePtr(
            nva2f::ReadDiffusionBlendshapeSolveModelInfo(modelPath.c_str()));
        if (!blendshapeModelInfo) {
            return {};
        }
        blendshapeParams = blendshapeModelInfo->GetExecutorCreationParameters(
            nva2f::IGeometryExecutor::ExecutionOption::All, identityIndex);
        hasBlendshapeModelInfo = true;
    }
    if (!hasBlendshapeModelInfo) {
        return {};
    }
    return blendshapeParams;
}

bool Audio2FaceModelInfo::hasBlendshapeInfo(int identityIndex) const {
    // Check if this model.json has blendshape info.
    // It does not guarantee the information is correct and the data is valid.
    // It is only used to check if the model has blendshape info.
    if (!isValid()) { return false; }

    const auto& doc = getModelDoc();

    if (!doc.HasMember("blendshapePaths")) {
        return false;
    }

    if (isRegression()) {
        // For regression models, blendshapePaths is an object (identityIndex is ignored)
        if (!doc["blendshapePaths"].IsObject()) {
            return false;
        }
        
        const auto& blendshapePaths = doc["blendshapePaths"].GetObject();
        
        // Check that both skin and tongue exist
        if (!blendshapePaths.HasMember("skin") || !blendshapePaths.HasMember("tongue")) {
            return false;
        }
        
        // Check skin hierarchy
        const auto& skin = blendshapePaths["skin"];
        if (!skin.IsObject() || !skin.HasMember("config") || !skin.HasMember("data")) {
            return false;
        }
        
        // Check tongue hierarchy  
        const auto& tongue = blendshapePaths["tongue"];
        if (!tongue.IsObject() || !tongue.HasMember("config") || !tongue.HasMember("data")) {
            return false;
        }
        
        return true;
    } else {
        // For diffusion models, blendshapePaths is an array
        if (!doc["blendshapePaths"].IsArray()) {
            return false;
        }
        
        const auto& blendshapePathsArray = doc["blendshapePaths"].GetArray();
        
        // Check if the requested identity index is valid
        if (identityIndex < 0 || identityIndex >= static_cast<int>(blendshapePathsArray.Size())) {
            return false;
        }
        
        // Check the specific identity at the given index
        const auto& identityBlendshapePaths = blendshapePathsArray[identityIndex];
        
        if (!identityBlendshapePaths.IsObject()) {
            return false;
        }
        
        const auto& blendshapePaths = identityBlendshapePaths.GetObject();
        
        // Check that both skin and tongue exist
        if (!blendshapePaths.HasMember("skin") || !blendshapePaths.HasMember("tongue")) {
            return false;
        }
        
        // Check skin hierarchy
        const auto& skin = blendshapePaths["skin"];
        if (!skin.IsObject() || !skin.HasMember("config") || !skin.HasMember("data")) {
            return false;
        }
        
        // Check tongue hierarchy
        const auto& tongue = blendshapePaths["tongue"];
        if (!tongue.IsObject() || !tongue.HasMember("config") || !tongue.HasMember("data")) {
            return false;
        }
        
        return true;
    }
}

bool isClassifierModel(const std::string& modelPath) {
    const auto& doc = readJson(modelPath);
    return doc.HasMember("networkPath");
}
