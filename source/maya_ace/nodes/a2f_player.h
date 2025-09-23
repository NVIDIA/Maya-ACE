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

// TODO: include individual header files for each module
#include "ace_player.h" 

#include <audio2x/audio_accumulator.h>
#include <audio2emotion/audio2emotion.h>
#include <audio2face/audio2face.h>
#include <audio2face/executor.h>
#include "common/timeslider.h"
#include "common/a2x_data.h"
#include "a2x_plugin_data.h"


class A2FAnimationPlayer : public MPxNode {
public:
    A2FAnimationPlayer();
    ~A2FAnimationPlayer() override;

    static void* creator();
    static MStatus initialize();
    void getCacheSetup(
        const MEvaluationNode & evalNode, 
        MNodeCacheDisablingInfo & disablingInfo, 
        MNodeCacheSetupInfo & cacheSetupInfo, 
        MObjectArray & monitoredAttributes) const override;
    MStatus compute(const MPlug&, MDataBlock&) override;

    void postConstructor() override;
    void setDeleted(bool flag);
    bool isDeleted() { return m_isDeleted; }

    static MTypeId id;
    static const char* typeName;
 
    static MObject dummyInput; // This is added to avoid having internal attributes that doesn't depend on any input attributes

    // Time Inputs
    static MObject time;

    // Audio Inputs
    static MObject audioParams;
    static MObject audiofile;
    static MObject audioOffset;
    static MObject audioStart;
    static MObject audioEnd;

    // Model Configs
    static MObject modelConfigs;
    static MObject a2eModelPath;
    static MObject a2fModelPath;
    static MObject identityIndex;
    static MObject useGpuBlendshapeSolver;

    // Geometry Inputs
    static MObject faceGeometryOrig;
    static MObject tongueGeometryOrig;

    // Emotion Parameters
    static MObject emotionParams;
    static MObject emotionStrength;
    static MObject emotionContrast;
    static MObject maxEmotions;
    static MObject liveBlendCoef;
    static MObject enablePreferredEmotion;
    static MObject preferredEmotionStrength;
    static MObject preferredEmotions;

    // Face parameters
    static MObject faceParams;
    static MObject lowerFaceSmoothing;
    static MObject upperFaceSmoothing;
    static MObject lowerFaceStrength;
    static MObject upperFaceStrength;
    static MObject faceMaskLevel;
    static MObject faceMaskSoftness;
    static MObject skinStrength;
    static MObject eyelidOpenOffset;
    static MObject lipOpenOffset;

    // Tongue parameters
    static MObject tongueParams;
    static MObject tongueStrength;
    static MObject tongueHeightOffset;
    static MObject tongueDepthOffset;

    // Blendshape parameters
    static MObject blendshapeParams;
    static MObject blendshapeFaceMultipliers;
    static MObject blendshapeFaceOffsets;
    static MObject blendshapeTongueMultipliers;
    static MObject blendshapeTongueOffsets;

    // Outputs
    static MObject isQuickEstimate;

    static MObject outputAnimationResults;
    static MObject outputWeights;
    static MObject faceWeightsCount;
    static MObject tongueWeightsCount;

    static MObject outputInferenceResults;
    static MObject faceGeometry;
    static MObject tongueGeometry;
    static MObject jawTransform;
    static MObject rightEyeRotationX;
    static MObject rightEyeRotationY;
    static MObject rightEyeRotationZ;
    static MObject rightEyeRotation;
    static MObject leftEyeRotationX;
    static MObject leftEyeRotationY;
    static MObject leftEyeRotationZ;
    static MObject leftEyeRotation;

    static MObject outputEmotionStates;
    static MObject outputEmotions;

    // Internal Attributes
    static MObject faceGeometryOrigCache;
    static MObject tongueGeometryOrigCache;
    static MObject audioAccumulatorAttr;
    static MObject emotionAccumulatorAttr;
    static MObject cudaStreamAttr;
    static MObject a2eExecutorAttr;
    static MObject a2fExecutorAttr;
    static MObject accumulatedAudioAccumulatorAttr;
    // containing emotion/geometry/blendshape results. They can come from background evaluation or quick estimate.
    static MObject executorResultsAttr; 
    static MObject quickEstimateBufferAttr; // allocate the buffer in advance
    static MObject backgroundEvaluationBufferAttr; // allocate the buffer in advance
    static MObject backgroundEvaluationCtxAttr; // background evaluation thread and related synchronization primitives

private:
    // Allocate & initialize internal attributes
    MStatus computeInitAudioAccumulator(const MPlug&, MDataBlock&);
    MStatus computeInitCudaStream(const MPlug&, MDataBlock&);
    MStatus computeInitBackgroundEvaluationCtx(const MPlug&, MDataBlock&);
    MStatus computeInitFaceMesh(const MPlug&, MDataBlock&);
    MStatus computeInitTongueMesh(const MPlug&, MDataBlock&);
    MStatus computeInitEmotionAccumulator(const MPlug&, MDataBlock&);
    MStatus computeInitA2EExecutor(const MPlug&, MDataBlock&);
    MStatus computeInitA2FExecutor(const MPlug&, MDataBlock&);
    MStatus computeInitAccumulatedAudioAccumulator(const MPlug&, MDataBlock&);
    MStatus computeInitQuickEstimateBuffer(const MPlug&, MDataBlock&);
    MStatus computeInitBackgroundEvaluationBuffer(const MPlug&, MDataBlock&);
    MStatus computeInitExecutorResults(const MPlug&, MDataBlock&);

    // Compute for output attributes
    MStatus computeGeometryResults(const MPlug&, MDataBlock&);
    MStatus computeBlendshapeSolveResults(const MPlug&, MDataBlock&);
    MStatus computeEmotionStates(const MPlug&, MDataBlock&);
    MStatus computeIsQuickEstimate(const MPlug&, MDataBlock&);

    // Helper functions
    MStatus allocateExecutorResultsData(MDataBlock& block, ExecutorResultsData& executorResultsData, std::size_t nbFrames = 0);
    MStatus updateA2EExecutorParameters(nva2e::IEmotionInteractiveExecutor& emotionExecutor, MDataBlock& block);
    MStatus runA2EExecutorOffline(
        nva2x::IAudioAccumulator& audioAccumulator, 
        nva2e::IEmotionInteractiveExecutor& emotionExecutor, 
        nva2x::IEmotionAccumulator& emotionAccumulator);
    MStatus updateA2FExecutorParameters(nva2f::IFaceInteractiveExecutor& faceExecutor, MDataBlock& block);
    MStatus runA2FExecutorOffline(
        nva2x::ICudaStream& cudaStream,
        nva2x::IAudioAccumulator& audioAccumulator,
        nva2x::IEmotionAccumulator& emotionAccumulator,
        nva2f::IFaceInteractiveExecutor& faceExecutor,
        ExecutorResultsData& executorResultsData,
        double targetTime = -1,
        bool quickEstimate = false);
    long computeFrameIndex(MDataBlock&, std::size_t, std::size_t, std::size_t, std::size_t, MStatus*);
    MStatus updateGeometryParameterDefaults(MDataBlock& block, nva2f::IGeometryInteractiveExecutor* geometryExecutor);
    MStatus createBlendshapeAliases(
        MDataBlock& block, 
        const std::vector<std::string>& facePoseNames,
        const std::vector<std::string>& tonguePoseNames
    );
    MStatus resetBlendshapeMultipliersAndOffsets(
        MDataBlock& block,
        UniquePtr<nva2f::IBlendshapeInteractiveExecutor>& blendshapeSolveExecutor
    );

    // Background evaluation
    A2XParameters getParameters(MDataBlock& block);
    bool isCacheStale(MDataBlock& block, MStatus* status = nullptr);
    MStatus cancelBackgroundEvaluation(BackgroundEvaluationData& bgData, MDataBlock block);
    MStatus backgroundEvaluationThreadFunc(
        nva2x::ICudaStream* cudaStream,
        nva2x::IAudioAccumulator* staticAudioAccumulator,
        nva2x::IEmotionAccumulator* emotionAccumulator,
        nva2e::IEmotionInteractiveExecutor* emotionExecutor,
        nva2f::IFaceInteractiveExecutor* faceExecutor,
        BackgroundEvaluationData* bgData,
        ExecutorResultsData* executorResultsData
    );

    bool m_isDeleted = false;
    TimeSliderProgress::CustomDrawID m_customDrawContext = TimeSliderProgress::kInvalidContext;
};
