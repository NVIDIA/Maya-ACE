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
#pragma once

#include <optional>
#include <string>
#include <filesystem>
#include <fstream>
#include <memory>
#include <utility>
#include <variant>

#include <audio2face/audio2face.h>

#include "a2x_data.h"
#include "a2x_models.h"

// Helper functions for safe casting
template<typename T>
T* getFaceExecutorAs(nva2f::IFaceInteractiveExecutor* executor) {
    return dynamic_cast<T*>(executor);
}

template<typename T>
const T* getFaceExecutorAs(const nva2f::IFaceInteractiveExecutor* executor) {
    return dynamic_cast<const T*>(executor);
}

// Helper function to create geometry executor
inline UniquePtr<nva2f::IGeometryInteractiveExecutor> createGeometryExecutor(
    const nva2f::GeometryExecutorCreationParameters& params,
    const std::string& modelPath,
    int identityIndex=0
) 
{
    UniquePtr<nva2f::IGeometryInteractiveExecutor> geometryExecutor;

    auto modelInfo = Audio2FaceModelInfo(modelPath);
    if (!modelInfo.isValid()) {
        return nullptr;
    }   

    auto isRegressionModel = modelInfo.isRegression();
    
    if (isRegressionModel) {
        auto newGeometryModelInfo = ToUniquePtr(nva2f::ReadRegressionModelInfo(modelPath.c_str()));
        if (!newGeometryModelInfo) {
            return nullptr;
        }
        // TODO: Improve hardcoded frame rate and denominator
        auto regressionParams = newGeometryModelInfo->GetExecutorCreationParameters(
            nva2f::IGeometryExecutor::ExecutionOption::All, 
            /*FrameRateNumerator=*/60,
            /*FrameRateDenominator=*/1
        );
        // use the maximum possible batch size (represented by 0) to speed up inference of single track
        geometryExecutor.reset(nva2f::CreateRegressionGeometryInteractiveExecutor(params, regressionParams, /*batchSize=*/0));
    } else {
        auto newGeometryModelInfo = ToUniquePtr(nva2f::ReadDiffusionModelInfo(modelPath.c_str()));
        if (!newGeometryModelInfo) {
            return nullptr;
        }
        // Setting constantNoise to true to match the behavior in ACE
        auto diffusionParams = newGeometryModelInfo->GetExecutorCreationParameters(
            nva2f::IGeometryExecutor::ExecutionOption::All, identityIndex, /*constantNoise=*/true);
        // use shortest possible nbInferencesForPreview to speed up preview
        // 0: always restart inference from the start (slow for preview)
        // 1: artifacts in preview is noticeable at the boundary between two inferences (frames 15+30n)
        // 2: artifacts is low enough to be acceptable
        geometryExecutor.reset(nva2f::CreateDiffusionGeometryInteractiveExecutor(params, diffusionParams, /*nbInferencesForPreview=*/2));
    }
    
    return std::move(geometryExecutor);
}

// Helper function to create blendshape executor
inline UniquePtr<nva2f::IBlendshapeInteractiveExecutor> createBlendshapeExecutor(
    UniquePtr<nva2f::IGeometryInteractiveExecutor> geometryExecutor,
    const std::string& modelPath,
    std::vector<std::string>& facePoseNames,
    std::vector<std::string>& tonguePoseNames,
    bool useGpuBlendshapeSolver=true,
    int identityIndex=0
) 
{
    if (!geometryExecutor) { return nullptr; }

    Audio2FaceModelInfo modelInfo(modelPath);
    if (!modelInfo.isValid()) {
        return nullptr;
    }
    auto isRegressionModel = modelInfo.isRegression();

    UniquePtr<nva2f::IBlendshapeInteractiveExecutor> blendshapeExecutor;
    nva2f::BlendshapeSolveExecutorCreationParameters blendshapeParams;
    std::variant<UniquePtr<nva2f::IRegressionModel::IBlendshapeSolveModelInfo>, 
        UniquePtr<nva2f::IDiffusionModel::IBlendshapeSolveModelInfo>> blendshapeModelInfo;
    
    if (isRegressionModel) {
        auto newBlendshapeModelInfo = ToUniquePtr(
            nva2f::ReadRegressionBlendshapeSolveModelInfo(modelPath.c_str()));
        if (!newBlendshapeModelInfo) {
            return nullptr;
        }
        blendshapeParams = newBlendshapeModelInfo->GetExecutorCreationParameters(
            nva2f::IGeometryExecutor::ExecutionOption::All);
        blendshapeModelInfo = std::move(newBlendshapeModelInfo); // make its lifetime longer than this scope
    } else {
        auto newBlendshapeModelInfo = ToUniquePtr(
            nva2f::ReadDiffusionBlendshapeSolveModelInfo(modelPath.c_str()));
        if (!newBlendshapeModelInfo) {
            return nullptr;
        }
        blendshapeParams = newBlendshapeModelInfo->GetExecutorCreationParameters(
            nva2f::IGeometryExecutor::ExecutionOption::All, identityIndex);
        blendshapeModelInfo = std::move(newBlendshapeModelInfo); // make its lifetime longer than this scope
    }

    facePoseNames = std::vector<std::string>(
        blendshapeParams.initializationSkinParams->data.poseNames,
        blendshapeParams.initializationSkinParams->data.poseNames + blendshapeParams.initializationSkinParams->data.poseNamesSize
    );

    tonguePoseNames = std::vector<std::string>(
        blendshapeParams.initializationTongueParams->data.poseNames,
        blendshapeParams.initializationTongueParams->data.poseNames + blendshapeParams.initializationTongueParams->data.poseNamesSize
    );

    if (useGpuBlendshapeSolver) {
        nva2f::DeviceBlendshapeSolveExecutorCreationParameters deviceParams;
        deviceParams.initializationSkinParams = blendshapeParams.initializationSkinParams;
        deviceParams.initializationTongueParams = blendshapeParams.initializationTongueParams;
        blendshapeExecutor.reset(nva2f::CreateDeviceBlendshapeSolveInteractiveExecutor(geometryExecutor.release(), deviceParams));
    } else {
        nva2f::HostBlendshapeSolveExecutorCreationParameters hostParams;
        hostParams.initializationSkinParams = blendshapeParams.initializationSkinParams;
        hostParams.initializationTongueParams = blendshapeParams.initializationTongueParams;
        hostParams.sharedJobRunner = nullptr;
        blendshapeExecutor.reset(nva2f::CreateHostBlendshapeSolveInteractiveExecutor(geometryExecutor.release(), hostParams));
    }
    return std::move(blendshapeExecutor);
}
