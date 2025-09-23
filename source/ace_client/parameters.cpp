// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#include <algorithm>
#include <map>
#include <string>

#include "parameters.h"

namespace mace
{

std::map<std::string, float> AceFaceParameters::GetParameterMap() const {
    return {
        // ace grpc api field: struct variable
        {"skinStrength", SkinStrength},
        {"upperFaceStrength", UpperFaceStrength},
        {"lowerFaceStrength", LowerFaceStrength},
        {"eyelidOpenOffset", EyelidOpenOffset},
        {"blinkStrength", BlinkStrength},
        {"lipOpenOffset", LipOpenOffset},
        {"upperFaceSmoothing", UpperFaceSmoothing},
        {"lowerFaceSmoothing", LowerFaceSmoothing},
        {"faceMaskLevel", FaceMaskLevel},
        {"faceMaskSoftness", FaceMaskSoftness},
        {"tongueStrength", TongueStrength},
        {"tongueHeightOffset", TongueHeightOffset},
        {"tongueDepthOffset", TongueDepthOffset},
    };
}

std::map<std::string, float> AceEmotionParameters::GetParameterMap() const {
    return {
        // ace grpc api field: struct variable
        {"emotion_contrast", emotion_contrast},
        {"live_blend_coef", live_blend_coef},
        {"emotion_strength", emotion_strength},
        {"max_emotions", (float)max_emotions},
        {"enable_preferred_emotion", (float)enable_preferred_emotion},
        {"preferred_emotion_strength", preferred_emotion_strength},
    };
}

std::map<std::string, float> AceEmotionState::GetParameterMap() const {
    return {
        // emotion name: struct variable
        {"amazement", amazement},
        {"anger", anger},
        {"cheekiness", cheekiness},
        {"disgust", disgust},
        {"fear", fear},
        {"grief", grief},
        {"joy", joy},
        {"outofbreath", outofbreath},
        {"pain", pain},
        {"sadness", sadness},
    };
}

std::vector<std::string> AceParameters::ListNames() const {
    std::vector<std::string> names;
    for(auto const& [name, val] : GetParameterMap()) {
        names.push_back(name);
    }
    return names;
}

} // namespace mace
