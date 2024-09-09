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
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <numeric>

#include "aceclient/audio.h"

#include <gtest/gtest.h>


TEST(TestAudio, TestIntScale) {
    ASSERT_EQ(Int16Max, 32767);
}

TEST(TestAudio, TestClamp) {
    ASSERT_EQ(clamp(0.5f, 0.0f, 1.0f), 0.5f);
    ASSERT_EQ(clamp(0.5f, 0.6f, 1.0f), 0.6f);
    ASSERT_EQ(clamp(0.5f, 0.0f, 0.4f), 0.4f);
}

TEST(TestAudio, TestGetSamplePosition) {
    ASSERT_EQ(get_sample_position(1.0f), 16000);
    ASSERT_EQ(get_sample_position(0.5f), 8000);
}

TEST(TestAudio, TestResample) {
    std::vector<float> source;

    // fill source with values like 0.0, 0.1, 0.2, ..., 0.9
    for (int i = 0; i < 100; i++ ) {
        source.push_back((i % 10) / 10.0f);
    }

    std::vector<float> resampled_03 = resample(source, 3, 10);
    ASSERT_EQ(resampled_03.size(), 30);
    ASSERT_EQ(resampled_03[2], 2.0f / 3.0f);
    ASSERT_EQ(resampled_03[1], 1.0f / 3.0f);

    std::vector<float> resampled_04 = resample(source, 10, 25);
    ASSERT_EQ(resampled_04.size(), 40);
    ASSERT_EQ(resampled_04[2], 0.5f);
    ASSERT_EQ(resampled_04[1], 0.25f);

    std::vector<float> resampled_05 = resample(source, 5, 10);
    ASSERT_EQ(resampled_05.size(), 50);
    ASSERT_EQ(resampled_05[2], 0.4f);
    ASSERT_EQ(resampled_05[1], 0.2f);
}

TEST(TestAudio, TestUpsample) {
    std::vector<float> source;

    // fill source with values like 0.0, 0.1, 0.2, ..., 0.9
    for (int i = 0; i < 100; i++ ) {
        source.push_back((i % 10) / 10.0f);
    }

    std::vector<float> resampled_2 = upsample(source, 2, 1);
    ASSERT_EQ(resampled_2.size(), 199);
    ASSERT_EQ(resampled_2[1], 0.05f);

    std::vector<float> resampled_20 = upsample(source, 20, 10);
    ASSERT_EQ(resampled_20.size(), 199);
    ASSERT_EQ(resampled_20[1], 0.05f);

    std::vector<float> resampled_30 = upsample(source, 30, 10);
    ASSERT_EQ(resampled_30.size(), 298);
    ASSERT_EQ(resampled_30[1], 0.1f / 3.0f);

    std::vector<float> resampled_25 = upsample(source, 25, 10);
    ASSERT_EQ(resampled_25.size(), 298);
    ASSERT_LE(resampled_25[1] - 0.04f, 1e-6);  // 0.04f != 0.04f
}

TEST(TestAudio, TestDownsample) {
    std::vector<float> source;

    // fill source with values like 0.0, 0.1, 0.2, ..., 0.9
    for (int i = 0; i < 100; i++ ) {
        source.push_back((i % 10) / 10.0f);
    }
    std::vector<float> resampled_05 = downsample(source, 5, 10);
    ASSERT_EQ(resampled_05.size(), 50);
    ASSERT_EQ(resampled_05[1], 0.2f);

    std::vector<float> resampled_03 = downsample(source, 3, 10);
    ASSERT_EQ(resampled_03.size(), 34);
    ASSERT_EQ(resampled_03[1], 0.3f);
}

TEST(TestAudio, TestReadWavFile16kFloat32Legacy) {
    /*Source wav file spec:
        - Duration: 00:00:03.77, bitrate: 512 kb/s
        - pcm_f32le ([3][0][0][0] / 0x0003), 16000 Hz, 1 channels, flt, 512 kb/s
        - total 60371 samples
    */
    std::filesystem::path input_path("./sample_data/audio_4sec_16k_f32.wav");
    std::vector<int16_t> samples = get_file_wav_content_int16(input_path.u8string());

    double mean_samples = 0.0;
    for (int i = 0; i < samples.size(); i++ ) {
        mean_samples += samples[i] / (double) samples.size();
    }
    ASSERT_LE(mean_samples, 1000);
    ASSERT_GE(mean_samples, -1000);

    ASSERT_EQ(samples.size(), 60371);

    int max_val = *std::max_element(samples.begin(), samples.end());
    int min_val = *std::min_element(samples.begin(), samples.end());
    ASSERT_GE(max_val, 1);
    ASSERT_LE(min_val, -1);
}

TEST(TestAudio, TestReadWavFile16kFloat32) {
    /*Source wav file spec:
        - Duration: 00:00:03.77, bitrate: 512 kb/s
        - pcm_f32le ([3][0][0][0] / 0x0003), 16000 Hz, 1 channels, flt, 512 kb/s
        - total 60371 samples
    */
    std::filesystem::path input_path("./sample_data/audio_4sec_16k_f32.wav");
    std::vector<float> samples_f = get_file_wav_content(input_path.u8string());
    std::vector<int16_t> samples = convert_float_to_int16(samples_f);

    double mean_samples = 0.0;
    for (int i = 0; i < samples.size(); i++ ) {
        mean_samples += samples[i] / (double) samples.size();
    }
    ASSERT_LE(mean_samples, 1000);
    ASSERT_GE(mean_samples, -1000);

    ASSERT_EQ(samples_f.size(), 60371);
    ASSERT_EQ(samples.size(), 60371);

    int max_val = *std::max_element(samples.begin(), samples.end());
    int min_val = *std::min_element(samples.begin(), samples.end());
    ASSERT_GE(max_val, 1);
    ASSERT_LE(min_val, -1);
}

TEST(TestAudio, TestReadWavFile16kInt16Legacy) {
    /*Source wav file spec:
        - Duration: 00:00:04.00, bitrate: 256 kb/s
        - pcm_s16le ([1][0][0][0] / 0x0001), 16000 Hz, 1 channels, s16, 256 kb/s
        - total 64000 samples
    */
    std::filesystem::path input_path("./sample_data/audio_4sec_16k_s16le.wav");
    std::vector<int16_t> samples = get_file_wav_content_int16(input_path.u8string());

    double mean_samples = 0.0;
    for (int i = 0; i < samples.size(); i++ ) {
        mean_samples += samples[i] / (double) samples.size();
    }
    ASSERT_LE(mean_samples, 1000);
    ASSERT_GE(mean_samples, -1000);

    ASSERT_EQ(samples.size(), 64000);

    int max_val = *std::max_element(samples.begin(), samples.end());
    int min_val = *std::min_element(samples.begin(), samples.end());
    ASSERT_GE(max_val, 1);
    ASSERT_LE(min_val, -1);
}

TEST(TestAudio, TestReadWavFile16kInt16) {
    /*Source wav file spec:
        - Duration: 00:00:04.00, bitrate: 256 kb/s
        - pcm_s16le ([1][0][0][0] / 0x0001), 16000 Hz, 1 channels, s16, 256 kb/s
        - total 64000 samples
    */
    std::filesystem::path input_path("./sample_data/audio_4sec_16k_s16le.wav");
    std::vector<float> samples_f = get_file_wav_content(input_path.u8string());
    std::vector<int16_t> samples = convert_float_to_int16(samples_f);

    double mean_samples = 0.0;
    for (int i = 0; i < samples.size(); i++ ) {
        mean_samples += samples[i] / (double) samples.size();
    }
    ASSERT_LE(mean_samples, 1000);
    ASSERT_GE(mean_samples, -1000);

    ASSERT_EQ(samples_f.size(), 64000);
    ASSERT_EQ(samples.size(), 64000);

    int max_val = *std::max_element(samples.begin(), samples.end());
    int min_val = *std::min_element(samples.begin(), samples.end());
    ASSERT_GE(max_val, 1);
    ASSERT_LE(min_val, -1);
}

TEST(TestAudio, TestReadWavFile48kInt16Legacy) {
    /*Source wav file spec:
        - Duration: 00:00:05.75, bitrate: 768 kb/s
        - pcm_s16le ([1][0][0][0] / 0x0001), 48000 Hz, 1 channels, s16, 768 kb/s
        - total 275925 samples
    */
    std::filesystem::path input_path("./sample_data/audio_6sec_48k_s16le.wav");
    std::vector<int16_t> samples = get_file_wav_content_int16(input_path.u8string());

    double mean_samples = 0.0;
    for (int i = 0; i < samples.size(); i++ ) {
        mean_samples += samples[i] / (double) samples.size();
    }
    ASSERT_LE(mean_samples, 1000);
    ASSERT_GE(mean_samples, -1000);

    ASSERT_EQ(samples.size(), 91975);

    int max_val = *std::max_element(samples.begin(), samples.end());
    int min_val = *std::min_element(samples.begin(), samples.end());
    ASSERT_GE(max_val, 1);
    ASSERT_LE(min_val, -1);
}

TEST(TestAudio, TestReadWavFile48kInt16) {
    /*Source wav file spec:
        - Duration: 00:00:05.75, bitrate: 768 kb/s
        - pcm_s16le ([1][0][0][0] / 0x0001), 48000 Hz, 1 channels, s16, 768 kb/s
        - total 275925 samples
    */
    std::filesystem::path input_path("./sample_data/audio_6sec_48k_s16le.wav");
    std::vector<float> samples_f = get_file_wav_content(input_path.u8string());
    std::vector<int16_t> samples = convert_float_to_int16(samples_f);

    double mean_samples = 0.0;
    for (int i = 0; i < samples.size(); i++ ) {
        mean_samples += samples[i] / (double) samples.size();
    }
    ASSERT_LE(mean_samples, 1000);
    ASSERT_GE(mean_samples, -1000);

    ASSERT_EQ(samples_f.size(), 91975);
    ASSERT_EQ(samples.size(), 91975);

    int max_val = *std::max_element(samples.begin(), samples.end());
    int min_val = *std::min_element(samples.begin(), samples.end());
    ASSERT_GE(max_val, 1);
    ASSERT_LE(min_val, -1);
}
