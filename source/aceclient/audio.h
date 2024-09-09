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

#include <cmath>
#include <limits>
#include <string>
#include <vector>

constexpr size_t DefaultSampleRate = 16000;
constexpr size_t DefaultBufferLength = 8320;
constexpr size_t DefaultBufferOffset = 4160;
constexpr size_t Int16Max = std::numeric_limits<int16_t>::max(); // = 32767 or max(32767, val * 32768)
constexpr size_t Int16Scale = 1 << 15; // 32768

std::vector<int16_t> get_file_wav_content_int16(const std::string& filename);  // deprecated
std::vector<int16_t> convert_float_to_int16(const std::vector<float> &input);
std::vector<float> resample(const std::vector<float>& input, int targetSampleRate, int originalSampleRate);
std::vector<float> upsample(const std::vector<float>& input, int targetSampleRate, int originalSampleRate);
std::vector<float> downsample(const std::vector<float>& input, int targetSampleRate, int originalSampleRate);
std::vector<float> get_file_wav_content(const std::string& filename, size_t samplerate=DefaultSampleRate);
double get_sample_position(double seconds, size_t samplerate=DefaultSampleRate);
double clamp(double value, double bottom, double top);
