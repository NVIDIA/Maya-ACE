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

#include <memory>

#include <audio2emotion/audio2emotion.h>
#include <audio2face/audio2face.h>
#include <audio2x/tensor.h>

#include <maya/MDataBlock.h>
#include <maya/MGlobal.h>
#include <maya/MFnPluginData.h>
#include <maya/MFnAttribute.h>
#include <maya/MTime.h>
#include <maya/MPxData.h>

#include "common/a2x_data.h"
#include "a2x_ids.h"


// Can't pass strings as non-type template parameters, so trick it using a function returning a string.
template <unsigned int Tid>
const char* getMayaDataName();

template <typename T, unsigned int Tid>
struct MayaData : public MPxData {
    static MTypeId Id() { return Tid; }
    static MString Name() { return getMayaDataName<Tid>(); }

    static void* creator() { return new MayaData(); }

    MTypeId typeId() const override { return Id(); }
    MString name() const override { return Name(); }

    void copy(const MPxData& src) override {
        if (src.typeId() == Id()) {
            mContent = static_cast<const MayaData&>(src).mContent;
        }
    }

    using content_type = T;
    content_type mContent;
};

struct BackgroundEvaluationData {
    std::thread mBackgroundThread;
    std::condition_variable mStartedCV;
    std::mutex mStartedMtx;
    std::atomic<bool> mBackgroundThreadRunning;
    std::atomic<bool> mBackgroundThreadFinished;

    // for checking if we should restart the background thread
    A2XParameters mPrevParameters;
    bool mPrevParametersSame;
    MTime mPrevTime;

    void stop() {
        // Signal background thread to stop
        mBackgroundThreadRunning.store(false, std::memory_order_release);
        mStartedCV.notify_all();
        // Wait for background thread to finish if it's running
        if (mBackgroundThread.joinable()) {
            mBackgroundThread.join();
        }
    }

    ~BackgroundEvaluationData() {
        stop();
    }
};

template <> inline const char* getMayaDataName<ID_ExecutorResultsData>() { return "ExecutorResultsData"; }
using MayaExecutorResultsData = MayaData<std::shared_ptr<ExecutorResultsData>, ID_ExecutorResultsData>;

template <> inline const char* getMayaDataName<ID_AudioAccumulatorData>() { return "AudioAccumulatorData"; }
using MayaAudioAccumulatorData = MayaData<std::shared_ptr<AudioAccumulatorData>, ID_AudioAccumulatorData>;

template <> inline const char* getMayaDataName<ID_EmotionAccumulatorData>() { return "EmotionAccumulatorData"; }
using MayaEmotionAccumulatorData = MayaData<std::shared_ptr<EmotionAccumulatorData>, ID_EmotionAccumulatorData>;

template <> inline const char* getMayaDataName<ID_CudaStreamData>() { return "CudaStreamData"; }
using MayaCudaStreamData = MayaData<std::shared_ptr<CudaStreamData>, ID_CudaStreamData>;

template <> inline const char* getMayaDataName<ID_A2EExecutorData>() { return "A2EExecutorData"; }
using MayaA2EExecutorData = MayaData<std::shared_ptr<A2EExecutorData>, ID_A2EExecutorData>;

template <> inline const char* getMayaDataName<ID_A2FExecutorData>() { return "A2FExecutorData"; }
using MayaA2FExecutorData = MayaData<std::shared_ptr<A2FExecutorData>, ID_A2FExecutorData>;

template <> inline const char* getMayaDataName<ID_BackgroundEvaluationData>() { return "BackgroundEvaluationData"; }
using MayaBackgroundEvaluationData = MayaData<std::shared_ptr<BackgroundEvaluationData>, ID_BackgroundEvaluationData>;

template <typename DataType>
typename DataType::content_type* GetPluginData(const MDataHandle& dataHandle) {
    auto pluginData = dataHandle.asPluginData();
    if (!pluginData || pluginData->typeId() != DataType::Id()) {
        return nullptr;
    }
    return &static_cast<DataType*>(pluginData)->mContent;
}

template <typename DataType>
typename DataType::content_type* GetPluginData(MDataBlock& block, const MObject& attribute) {
    MStatus status;
    MDataHandle dataHandle = block.inputValue(attribute, &status);
    CHECK_MSTATUS_AND_RETURN(status, nullptr);
    return GetPluginData<DataType>(dataHandle);
}

template <typename DataType>
typename DataType::content_type* GetPluginData(MDataBlock& block, const MPlug& plug) {
    MStatus status;
    MDataHandle dataHandle = block.inputValue(plug, &status);
    CHECK_MSTATUS_AND_RETURN(status, nullptr);
    return GetPluginData<DataType>(dataHandle);
}

template <typename DataType>
MStatus SetPluginData(MDataBlock& block, const MPlug& outputPlug, typename DataType::content_type content) {
    MStatus status;
    MFnPluginData fnDataCreator;

    fnDataCreator.create(DataType::Id(), &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    DataType* newData = static_cast<DataType*>(fnDataCreator.data(&status));
    CHECK_MSTATUS_AND_RETURN_IT(status);
    if (!newData) {
        return MStatus::kFailure;
    }

    newData->mContent = std::move(content);

    MDataHandle outputHandle = block.outputValue(outputPlug, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    outputHandle.set(newData);
    block.setClean(outputPlug);

    return MStatus::kSuccess;
}
