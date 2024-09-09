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
#include "audio.h"
#include "logger.h"

#include <AudioFile.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <vector>

#pragma warning(disable : 5038)
#pragma warning(disable : 4456)


std::vector<int16_t> get_file_wav_content_int16(const std::string& filename) {
    /*
    A simple wave file reader with resampling - not preserving signal quality.
        - Returns audio samples at 16k samplerate int16 bitdepth.
    */

    // Read 8, 16, 24, 32 bits PCM(int) and 32 bits IEEEFLOAT and map to [-1.0, 1.0] values
    //      NOTE: AudioFile<int> cannot read IEEEFLOAT files.
    AudioFile<float> audio(filename);

    // NOTE: The current A2F service accepts only int16 audio samples.
    std::vector<int16_t> samples;

    // no audio data
    if(audio.getNumChannels() == 0 || audio.getLengthInSeconds() == 0){
        LOG_ERROR("No audio samples from " + filename);
        return {};
    }

    if(audio.getSampleRate() != DefaultSampleRate){
        // TODO: Need a better resampling.
        // NOTE: Temporary resampling (only down-sampling is allowed.)
        if(audio.getSampleRate() < DefaultSampleRate) {
            LOG_ERROR("Source samplerate is too low (< 16000): " + filename);
            return {};
        }

        size_t out_size = audio.samples[0].size() * DefaultSampleRate / audio.getSampleRate();

        int32_t min_resample = 0, max_resample = 0;
        for (size_t i = 0; i < out_size; i++) {
            size_t k = std::min(
                std::max((size_t) 0, i * audio.getSampleRate() / DefaultSampleRate),
                audio.samples[0].size() - 1
            );
            float sample_f = audio.samples[0][k];
            // float sample_f = (audio.samples[0][k] + audio.samples[0][k+1] + audio.samples[0][k+2]) / 3.0f;

            int32_t sample_i = static_cast<int32_t> (std::min(32767.0f, sample_f * 32768.0f));
            samples.push_back(sample_i);

        }
    }
    else {
        for (int i = 0; i < audio.samples[0].size(); i++) {
            samples.push_back(static_cast<int16_t> (std::min(32767.0f, audio.samples[0][i] * 32768.0f)));
        }
    }

    return samples;
}

std::vector<int16_t> convert_float_to_int16(const std::vector<float> &input) {
    /*Convert sample values from float to int16_t
    */
    std::vector<int16_t> output;
    for (float sample: input) {
        output.push_back(static_cast<int16_t> (std::min(32767.0f, sample * 32768.0f)));
    }
    return output;
}

std::vector<float> get_file_wav_content(const std::string& filename, size_t samplerate) {
    AudioFile<float> audio(filename);  // can read PCM and IEEE FLOAT in value range [-1.0, 1.0]

    if(audio.getNumChannels() == 0 || audio.getLengthInSeconds() == 0) return {};

    const auto src_sr = audio.getSampleRate();
    if(src_sr == samplerate) return audio.samples[0];
    if(src_sr < samplerate) return {};  // no upsampling is supported

    const auto original = audio.samples[0];

    return resample(original, samplerate, src_sr);
}

std::vector<float> resample(const std::vector<float>& input, int targetSampleRate, int originalSampleRate) {
    const int multiple = targetSampleRate / originalSampleRate;
    if(multiple * targetSampleRate == originalSampleRate) // multiple of source; 48 khz -> 16k, 24k
    {
        return downsample(input, targetSampleRate, originalSampleRate);
    }

    size_t lcm = std::lcm(originalSampleRate, targetSampleRate);
    if (lcm > 1e10) {
        return {}; // it is too expensive to resample. not supported
    }

    return downsample(upsample(input, lcm, originalSampleRate), targetSampleRate, lcm);
}

std::vector<float> upsample(const std::vector<float>& input, int targetSampleRate, int originalSampleRate) {
    std::vector<float> output;
    float ratio = static_cast<float>(targetSampleRate) / originalSampleRate;

    for (size_t i = 0; i < input.size(); ++i) {
        output.push_back(input[i]);
        if (i < input.size() - 1) {
            float nextSample = input[i + 1];
            for (float t = 1.0f; t < ratio; t += 1.0f) {
                float interpolatedSample = input[i] + (nextSample - input[i]) * (t / ratio);
                output.push_back(interpolatedSample);
            }
        }
    }
    return output;
}

std::vector<float> downsample(const std::vector<float>& input, int targetSampleRate, int originalSampleRate) { //decimate
    std::vector<float> output;
    float ratio = static_cast<float>(originalSampleRate) / targetSampleRate;

    for (size_t i = 0; i < input.size(); i += static_cast<size_t>(ratio)) {
        output.push_back(input[i]);
    }

    return output;
}

double get_sample_position(double seconds, size_t samplerate) {
    return samplerate * seconds;
}

double clamp(double value, double bottom, double top) {
    return std::min(top, std::max(bottom, value));
}
