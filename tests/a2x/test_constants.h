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
#pragma once

#include <string>
#include <filesystem>


// TODO: change this to the correct path when releasing
const std::string A2E_MODEL_PATH = "sample_project/models/audio2emotion-models/audio2emotion-v2.2/model.json";
const std::string A2F_DIFFUSION_MODEL_PATH = "sample_project/models/audio2face-models/audio2face-3d-v3.0/model.json";
const std::string A2F_REGRESSION_MODEL_PATH = "sample_project/models/audio2face-models/audio2face-3d-v2.3-mark/model.json";
const std::string AUDIO_FILE_PATH = "sample_data/audio_4sec_16k_s16le.wav";

inline std::string get_test_data_path(const std::string& relative_path) {
    const auto test_dir = std::filesystem::current_path().u8string();
    const auto u8_path = std::filesystem::u8path(relative_path);

    if (u8_path.is_relative()) {
        return (test_dir / u8_path).generic_string();
    }
    return u8_path.generic_string();
}
