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

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include <audio2face/audio2face.h>

#include "a2x_data.h"


class Audio2FaceModelInfo {
    public:
        Audio2FaceModelInfo(const std::string& modelPath);
        virtual ~Audio2FaceModelInfo();

        virtual bool isValid() const;
        virtual bool isRegression() const;
        bool hasBlendshapeInfo(int identityIndex=0) const;
        std::vector<std::string> getEmotionNames() const;
        int getEmotionCount() const;
        std::vector<std::string> getIdentityNames() const;
        int getIdentityCount() const;
        int getAudioBufferLength() const;

        // TODO: deprecate this.
        nva2f::BlendshapeSolveExecutorCreationParameters 
            getBlendshapeSolveExecutorCreationParameters(int identityIndex=0) const;

        virtual const rapidjson::Document& getModelDoc() const;
        virtual const rapidjson::Document& getNetworkInfoDoc() const;

    private:
        std::string modelPath;
        rapidjson::Document modelDoc;
        rapidjson::Document networkInfoDoc;
};

bool isClassifierModel(const std::string& modelPath);

rapidjson::Document readJson(const std::string& modelPath);
