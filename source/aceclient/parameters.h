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
#pragma once

#include <iostream>
#include <map>
#include <string>

namespace mace {
  struct AceFaceParameters {
    // map to NvACEA2XParameters
    // default values are from Claire v1
    float SkinStrength = 1.f;
    float UpperFaceStrength = 1.f;
    float LowerFaceStrength = 1.f;
    float EyelidOpenOffset = 0.f;
    float BlinkStrength = 1.f;
    float LipOpenOffset = 0.f;
    float UpperFaceSmoothing = 0.001f;
    float LowerFaceSmoothing = 0.006f;
    float FaceMaskLevel = 0.6f;
    float FaceMaskSoftness = 0.0085f;
    float TongueStrength = 1.3f;
    float TongueHeightOffset = 0.f;
    float TongueDepthOffset = 0.f;

    std::map<char *, float> GetParameterMap() const;
  };

  struct AceEmotionParameters {
    // map to NvACEEmotionParameters
    float emotion_contrast = 1.f;
    float live_blend_coef = 0.7f;
    bool enable_preferred_emotion = true;
    float preferred_emotion_strength = 1.0f;
    float emotion_strength = 0.6f;
    int32_t max_emotions = 6;

    std::map<char *, float> GetParameterMap() const;
  };

  struct AceEmotionState {
    // map to NvACEEmotionState
    float amazement = 0.f;
    float anger = 0.f;
    float cheekiness = 0.f;
    float disgust = 0.f;
    float fear = 0.f;
    float grief = 0.f;
    float joy = 0.f;
    float out_of_breath = 0.f;
    float pain = 0.f;
    float sadness = 0.f;

    std::map<char *, float> GetParameterMap() const;
  };
} // namespace mace
