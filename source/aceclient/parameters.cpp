// SPDX-FileCopyrightText: Copyright (c) 2023-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#include <map>
#include <string>

#include "parameters.h"

#define Parameter_Face_SkinStrength "skinStrength"
#define Parameter_Face_UpperFaceStrength "upperFaceStrength"
#define Parameter_Face_LowerFaceStrength "lowerFaceStrength"
#define Parameter_Face_EyelidOpenOffset "eyelidOpenOffset"
#define Parameter_Face_BlinkStrength "blinkStrength"
#define Parameter_Face_LipOpenOffset "lipOpenOffset"
#define Parameter_Face_UpperFaceSmoothing "upperFaceSmoothing"
#define Parameter_Face_LowerFaceSmoothing "lowerFaceSmoothing"
#define Parameter_Face_FaceMaskLevel "faceMaskLevel"
#define Parameter_Face_FaceMaskSoftness "faceMaskSoftness"
#define Parameter_Face_TongueStrength "tongueStrength"
#define Parameter_Face_TongueHeightOffset "tongueHeightOffset"
#define Parameter_Face_TongueDepthOffset "tongueDepthOffset"

namespace mace {
  std::map<char *, float> AceFaceParameters::GetParameterMap() const {
    return {
      // ace grpc api field: struct variable
      {Parameter_Face_SkinStrength, SkinStrength},
      {Parameter_Face_UpperFaceStrength, UpperFaceStrength},
      {Parameter_Face_LowerFaceStrength, LowerFaceStrength},
      {Parameter_Face_EyelidOpenOffset, EyelidOpenOffset},
      {Parameter_Face_BlinkStrength, BlinkStrength},
      {Parameter_Face_LipOpenOffset, LipOpenOffset},
      {Parameter_Face_UpperFaceSmoothing, UpperFaceSmoothing},
      {Parameter_Face_LowerFaceSmoothing, LowerFaceSmoothing},
      {Parameter_Face_FaceMaskLevel, FaceMaskLevel},
      {Parameter_Face_FaceMaskSoftness, FaceMaskSoftness},
      {Parameter_Face_TongueStrength, TongueStrength},
      {Parameter_Face_TongueHeightOffset, TongueHeightOffset},
      {Parameter_Face_TongueDepthOffset, TongueDepthOffset},
    };
  }
  std::map<char *, float> AceEmotionParameters::GetParameterMap() const {
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

  std::map<char *, float> AceEmotionState::GetParameterMap() const {
    return {
      // emotion name: struct variable
      {"amazement", amazement},
      {"anger", anger},
      {"cheekiness", cheekiness},
      {"disgust", disgust},
      {"fear", fear},
      {"grief", grief},
      {"joy", joy},
      {"outofbreath", out_of_breath},
      {"pain", pain},
      {"sadness", sadness},
    };
  }
} // namespace mace
