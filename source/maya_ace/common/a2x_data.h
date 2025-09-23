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

#include <atomic>
#include <condition_variable>
#include <memory>
#include <thread>
#include <vector>

#include <audio2emotion/audio2emotion.h>
#include <audio2face/audio2face.h>
#include <audio2face/parse_helper.h>
#include <audio2x/tensor.h>
#include <audio2x/error.h>


struct Destroyer {
    template <typename T>
    void operator()(T* obj) const {
        if (obj) {
            obj->Destroy();
        }
    }
};

template <typename T>
using UniquePtr = std::unique_ptr<T, Destroyer>;

template <typename T>
UniquePtr<T> ToUniquePtr(T* ptr) { return UniquePtr<T>(ptr); }

template <typename T>
using SharedPtr = std::shared_ptr<T>;

template <typename T>
SharedPtr<T> ToSharedPtr(UniquePtr<T> p) {
    return SharedPtr<T>(p.release(), [](T* p){ p->Destroy();});
}

struct GeometryExecutorResultsData {
    UniquePtr<nva2x::IHostTensorFloat> resultsBuffer = nullptr;
    std::size_t faceSize = 0;
    std::size_t tongueSize = 0;
    std::size_t jawSize = 0;
    std::size_t eyesSize = 0;
};

struct BlendshapeExecutorResultsData {
    UniquePtr<nva2x::IHostTensorFloat> resultsBuffer = nullptr;
    std::size_t faceWeightsCount = 0;
    std::size_t tongueWeightsCount = 0;
};

struct ExecutorResultsData {
    std::size_t nbFrames = 0;
    std::size_t nbActualFrames = 0; // might be smaller than nbFrames if early terminated
    std::size_t frameRateNum = 0;
    std::size_t frameRateDenom = 0;
    GeometryExecutorResultsData geometryExecutorResultsData;
    BlendshapeExecutorResultsData blendshapeExecutorResultsData;
    bool isQuickEstimate;
};

struct AudioAccumulatorData {
    UniquePtr<nva2x::IAudioAccumulator> mAudioAccumulator;
};

struct EmotionAccumulatorData {
    UniquePtr<nva2x::IEmotionAccumulator> mEmotionAccumulator;

    // These are used for outputting the emotion results.
    UniquePtr<nva2x::IDeviceTensorFloat> mEmotionDeviceTensor; // for sampling from the accumulator
    UniquePtr<nva2x::IHostTensorFloat> mEmotionHostTensor; // for read back
};

struct CudaStreamData {
    UniquePtr<nva2x::ICudaStream> mCudaStream;
};

struct A2EExecutorData {
    UniquePtr<nva2e::IEmotionInteractiveExecutor> mEmotionInteractiveExecutor;
};

struct A2FExecutorData {
    // Store the base interface pointer
    UniquePtr<nva2f::IBlendshapeInteractiveExecutor> mBlendshapeInteractiveExecutor = nullptr;
    UniquePtr<nva2f::IGeometryInteractiveExecutor> mGeometryInteractiveExecutor = nullptr;

    std::size_t mAudioBufferLength = 0;
    
    // Common method that works with base interface
    std::size_t getStartSampleIndex(std::size_t sampleIndex) const {
        // Use base interface method if available
        const std::size_t chunkSize = mAudioBufferLength;
        const auto sampleRate = getSamplingRate();
        std::size_t frameRateNum, frameRateDenom;
        getFrameRate(frameRateNum, frameRateDenom);
        // Ideally startSampleIndex = sampleIndex - chunkSize / 2
        // And startSampleIndex + sampleRate * nbFramesFromStart / fps == sampleIndex
        // But this won't always be true due to rounding, so we need to find the nearest nbFramesFromStart
        if(sampleRate == 0)
        {
            return 0;
        }
        std::size_t nbFramesFromStart = ((chunkSize / 2) * frameRateNum / frameRateDenom + sampleRate - 1) / sampleRate;
        std::size_t nbSamplesFromStart = nbFramesFromStart * sampleRate * frameRateDenom / frameRateNum;
        return sampleIndex > nbSamplesFromStart ? sampleIndex - nbSamplesFromStart : 0;
    }

    bool hasBlendshapeExecutor() const {
        return mBlendshapeInteractiveExecutor && mBlendshapeInteractiveExecutor.get() != nullptr;
    }

    nva2f::IFaceInteractiveExecutor* getFaceExecutor() const {
        if (hasBlendshapeExecutor()) {
            return mBlendshapeInteractiveExecutor.get();
        }
        if (mGeometryInteractiveExecutor) {
            return mGeometryInteractiveExecutor.get();
        }
        return nullptr;
    }

    nva2f::IGeometryInteractiveExecutor* getGeometryExecutor() const {
        if (hasBlendshapeExecutor()) {
            const auto& blendshapeExecutor = *mBlendshapeInteractiveExecutor;
            nva2f::IGeometryInteractiveExecutor* geometryExecutor = nullptr;
            auto status = nva2f::GetInteractiveExecutorGeometryExecutor(blendshapeExecutor, &geometryExecutor);
            if (!status) {
                return nullptr;
            }
            return geometryExecutor;
        }
        return mGeometryInteractiveExecutor.get();
    }

    const std::size_t getSamplingRate() const {
        if (!hasBlendshapeExecutor() && !mGeometryInteractiveExecutor) {
            return 0;
        }
        auto* executor = getFaceExecutor();
        return executor->GetSamplingRate();
    }

    void getFrameRate(std::size_t& numerator, std::size_t& denominator) const {
        if (!hasBlendshapeExecutor() && !mGeometryInteractiveExecutor) {
            numerator = 0;
            denominator = 1;
            return;
        }
        auto* executor = getFaceExecutor();
        executor->GetFrameRate(numerator, denominator);
    }

    const std::size_t getTotalNbFrames() const {
        if (!hasBlendshapeExecutor() && !mGeometryInteractiveExecutor) {
            return 0;
        }
        auto* executor = getFaceExecutor();
        return executor->GetTotalNbFrames();
    }
};

struct CachedBlendshapeParams {
    std::vector<float> mFaceMultipliers;
    std::vector<float> mFaceOffsets;
    std::vector<float> mTongueMultipliers;
    std::vector<float> mTongueOffsets;
};

struct A2XParameters {
    nva2e::PostProcessParams emotion;
    nva2f::AnimatorSkinParams face;
    nva2f::AnimatorTongueParams tongue;

    std::vector<float> _preferredEmotionsCopy;

    void setPreferredEmotions(const std::vector<float>& values) {
        this->emotion.preferredEmotion = {values.data(), values.size()};
        // NOTE: duplicate values for simple operations.
        this->_preferredEmotionsCopy = values;
    }

    bool operator==(const A2XParameters& other) const {
        // Could be optimized by using hash but it's currently fast enough
        return emotion.emotionStrength == other.emotion.emotionStrength &&
               emotion.emotionContrast == other.emotion.emotionContrast &&
               emotion.maxEmotions == other.emotion.maxEmotions &&
               emotion.liveBlendCoef == other.emotion.liveBlendCoef &&
               emotion.enablePreferredEmotion == other.emotion.enablePreferredEmotion &&
               emotion.preferredEmotionStrength == other.emotion.preferredEmotionStrength &&
               _preferredEmotionsCopy == other._preferredEmotionsCopy &&
               // TODO: compare beginning emotion when it's exposed in AceAnimationPlayer

               face.lowerFaceSmoothing == other.face.lowerFaceSmoothing &&
               face.upperFaceSmoothing == other.face.upperFaceSmoothing &&
               face.lowerFaceStrength == other.face.lowerFaceStrength &&
               face.upperFaceStrength == other.face.upperFaceStrength &&
               face.faceMaskLevel == other.face.faceMaskLevel &&
               face.faceMaskSoftness == other.face.faceMaskSoftness &&
               face.skinStrength == other.face.skinStrength &&
               face.eyelidOpenOffset == other.face.eyelidOpenOffset &&
               face.lipOpenOffset == other.face.lipOpenOffset &&

               tongue.tongueStrength == other.tongue.tongueStrength &&
               tongue.tongueHeightOffset == other.tongue.tongueHeightOffset &&
               tongue.tongueDepthOffset == other.tongue.tongueDepthOffset;
    }

    bool operator!=(const A2XParameters& other) const {
        return !(*this == other);
    }
};
