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
#include "a2f_player.h"

#include <algorithm>
#include <fstream>
#include <variant>
#include <vector>

#include <maya/MGlobal.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatMatrix.h>
#include <maya/MComputation.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MNodeMessage.h>

#include <audio2emotion/audio2emotion.h>
#include <audio2face/audio2face.h>
#include <audio2face/executor.h>
#include <audio2x/error.h>
#include <audio2x/tensor.h>

#include "common/maya_utils.h"
#include "common/a2x_models.h"
#include "common/a2x_data.h"
#include "common/a2x_executors.h"

#include "a2x_plugin_data.h"
#include "profiler.h"

using mace_maya::GetArrayInput;
using mace_maya::SetOutput;
using mace_maya::SetArrayOutput;
using mace_maya::SetAttributeDefault;
using mace_maya::RemoveArrayAliases;
using mace_maya::CreateArrayAliases;


A2FAnimationPlayer::A2FAnimationPlayer() = default;

A2FAnimationPlayer::~A2FAnimationPlayer() = default;

void A2FAnimationPlayer::getCacheSetup(
    const MEvaluationNode & evalNode, 
    MNodeCacheDisablingInfo & disablingInfo, 
    MNodeCacheSetupInfo & cacheSetupInfo, 
    MObjectArray & monitoredAttributes) const {
    disablingInfo.setCacheDisabled(true);
    disablingInfo.setReason(A2FAnimationPlayer::typeName + MString(" doesn't support built-in cache playback."));
}

void A2FAnimationPlayer::setDeleted(bool doDelete) {
    if (m_isDeleted == doDelete) {
        return;
    }
    if (doDelete) {
        TimeSliderProgress::deregisterCustomDraw(m_customDrawContext);
        m_customDrawContext = TimeSliderProgress::kInvalidContext;
        m_isDeleted = true;
    } else {
        m_customDrawContext = TimeSliderProgress::registerCustomDraw();
        m_isDeleted = false;
    }
}

void A2FAnimationPlayer::postConstructor() {
    //MFnDependencyNode nodeFn(thisMObject());
    //MGlobal::displayInfo(MString("postConstructor") + nodeFn.name());
    m_customDrawContext = TimeSliderProgress::registerCustomDraw();
    m_isDeleted = false;
}

MStatus A2FAnimationPlayer::compute(const MPlug& plug, MDataBlock& block) {

    MStatus result = MS::kFailure;

    // Visualize processing. Also enables cancelling the computation.
    MComputation computation;

    computation.beginComputation();
    if (plug == audioAccumulatorAttr) {
        result = computeInitAudioAccumulator(plug, block);
    } else if (plug == cudaStreamAttr) {
        result = computeInitCudaStream(plug, block);
    } else if (plug == backgroundEvaluationCtxAttr) {
        result = computeInitBackgroundEvaluationCtx(plug, block);
    } else if (plug == faceGeometryOrigCache) {
        result = computeInitFaceMesh(plug, block);
    } else if (plug == tongueGeometryOrigCache) {
        result = computeInitTongueMesh(plug, block);
    } else if (plug == emotionAccumulatorAttr) {
        result = computeInitEmotionAccumulator(plug, block);
    } else if (plug == a2eExecutorAttr) {
        result = computeInitA2EExecutor(plug, block);
    } else if (plug == a2fExecutorAttr) {
        result = computeInitA2FExecutor(plug, block);
    } else if (plug == accumulatedAudioAccumulatorAttr) {
        result = computeInitAccumulatedAudioAccumulator(plug, block);
    } else if (plug == quickEstimateBufferAttr) {
        result = computeInitQuickEstimateBuffer(plug, block);
    } else if (plug == backgroundEvaluationBufferAttr) {
        result = computeInitBackgroundEvaluationBuffer(plug, block);
    } else if (plug == executorResultsAttr) {
        result = computeInitExecutorResults(plug, block);
    } else if(plug == outputInferenceResults ||
        plug == faceGeometry ||
        plug == tongueGeometry ||
        plug == jawTransform ||
        plug == rightEyeRotation ||
        plug == leftEyeRotation) {
        // Mesh output attributes are evaluated even when not connected.
        // Hence this additional check to avoid unnecessary computation.
        MPlug faceGeometryPlug(thisMObject(), faceGeometry);
        MPlug tongueGeometryPlug(thisMObject(), tongueGeometry);
        if (faceGeometryPlug.isConnected() || 
            tongueGeometryPlug.isConnected() ||
            plug == jawTransform ||
            plug == rightEyeRotation ||
            plug == leftEyeRotation) {
            result = computeGeometryResults(plug, block);
        }
    } else if(plug == outputAnimationResults ||
        plug == outputWeights ||
        plug == faceWeightsCount ||
        plug == tongueWeightsCount) {
        result = computeBlendshapeSolveResults(plug, block);
    } else if (plug == outputEmotionStates || 
        plug == outputEmotions) {
        result = computeEmotionStates(plug, block);
    } else if (plug == isQuickEstimate) {
        result = computeIsQuickEstimate(plug, block);
    }
    computation.endComputation();

    return result;
}

MStatus A2FAnimationPlayer::cancelBackgroundEvaluation(BackgroundEvaluationData& bgData, MDataBlock block) {
    if (bgData.mBackgroundThreadRunning.load(std::memory_order_acquire)) {
        bgData.mBackgroundThreadRunning.store(false, std::memory_order_release);
        // Only interrupt if the background thread is running
        // Otherwise, audio accumulator might not have been closed yet
        const auto a2eExecutorDataPtr = GetPluginData<MayaA2EExecutorData>(block, a2eExecutorAttr);
        if (a2eExecutorDataPtr && *a2eExecutorDataPtr) {
            // Sometimes, this is called before the a2e executor is initialized
            auto& a2eExecutorData = **a2eExecutorDataPtr;
            auto& a2eExecutor = *a2eExecutorData.mEmotionInteractiveExecutor;
            A2F_CHECK_WARNING_FAILURE(!a2eExecutor.Interrupt(), "Failed to interrupt a2e executor");
        }
        const auto a2fExecutorDataPtr = GetPluginData<MayaA2FExecutorData>(block, a2fExecutorAttr);
        if (a2fExecutorDataPtr && *a2fExecutorDataPtr) {
            auto& a2fExecutorData = **a2fExecutorDataPtr;
            auto& a2fExecutor = *a2fExecutorData.getFaceExecutor();
            A2F_CHECK_WARNING_FAILURE(!a2fExecutor.Interrupt(), "Failed to interrupt a2f executor");
        }
    }
    bgData.mStartedCV.notify_one();
    if (bgData.mBackgroundThread.joinable()) {
        bgData.mBackgroundThread.join();
    }
    bgData.mBackgroundThreadFinished.store(false, std::memory_order_release);

    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::computeInitAudioAccumulator(const MPlug& plug, MDataBlock& block) {
    // Wipe previous results so that it's empty in case of failure.
    MStatus status = SetPluginData<MayaAudioAccumulatorData>(
        block, MPlug(thisMObject(), audioAccumulatorAttr), {}
        );
    CHECK_MSTATUS_AND_RETURN_IT(status);
    
    AudioAccumulatorData audioAccumulatorData {
        std::move(ToUniquePtr(nva2x::CreateAudioAccumulator(16000, 0)))
    };

    status = SetPluginData<MayaAudioAccumulatorData>(
        block, 
        MPlug(thisMObject(), audioAccumulatorAttr), 
        std::make_shared<AudioAccumulatorData>(std::move(audioAccumulatorData))
        );
    CHECK_MSTATUS_AND_RETURN_IT(status);
    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::computeInitCudaStream(const MPlug& plug, MDataBlock& block) {
    // Wipe previous results so that it's empty in case of failure.
    MStatus status = SetPluginData<MayaCudaStreamData>(
        block, MPlug(thisMObject(), cudaStreamAttr), {}
        );
    CHECK_MSTATUS_AND_RETURN_IT(status);

    CudaStreamData cudaStreamData {
        std::move(ToUniquePtr(nva2x::CreateCudaStream()))
    };

    status = SetPluginData<MayaCudaStreamData>(
        block, 
        MPlug(thisMObject(), cudaStreamAttr), 
        std::make_shared<CudaStreamData>(std::move(cudaStreamData))
        );
    CHECK_MSTATUS_AND_RETURN_IT(status);
    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::computeInitBackgroundEvaluationCtx(const MPlug& plug, MDataBlock& block) {
    // Wipe previous results so that it's empty in case of failure.
    MStatus status = SetPluginData<MayaBackgroundEvaluationData>(
        block, MPlug(thisMObject(), backgroundEvaluationCtxAttr), {}
        );
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = SetPluginData<MayaBackgroundEvaluationData>(
        block, 
        MPlug(thisMObject(), backgroundEvaluationCtxAttr), 
        std::make_shared<BackgroundEvaluationData>()
        );
    CHECK_MSTATUS_AND_RETURN_IT(status);
    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::computeInitFaceMesh(const MPlug& plug, MDataBlock& block) {
    MStatus status;
    MDataHandle faceGeometryOrigHandle = block.inputValue(faceGeometryOrig, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MObject faceGeometryOrigMesh = faceGeometryOrigHandle.asMesh();
    A2F_CHECK_WARNING_FAILURE(!faceGeometryOrigMesh.isNull(), "no input face mesh");

    MDataHandle outputFaceGeometryHandle = block.outputValue(faceGeometryOrigCache);
    MFnMeshData dataCreator;
    MObject outputFaceData = dataCreator.create();
    
    MFnMesh fnFaceGeometryOrigMesh(faceGeometryOrigMesh);
    fnFaceGeometryOrigMesh.copy(faceGeometryOrigMesh, outputFaceData);

    block.outputValue(faceGeometryOrigCache).set(outputFaceData);
    block.outputValue(faceGeometryOrigCache).setClean();

    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::computeInitTongueMesh(const MPlug& plug, MDataBlock& block) {
    MStatus status;
    MDataHandle tongueGeometryOrigHandle = block.inputValue(tongueGeometryOrig, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MObject tongueGeometryOrigMesh = tongueGeometryOrigHandle.asMesh();
    A2F_CHECK_WARNING_FAILURE(!tongueGeometryOrigMesh.isNull(), "no input tongue mesh");

    MDataHandle outputTongueGeometryHandle = block.outputValue(tongueGeometryOrigCache);
    MFnMeshData dataCreator;
    MObject outputTongueData = dataCreator.create();
    
    MFnMesh fnTongueGeometryOrigMesh(tongueGeometryOrigMesh);
    fnTongueGeometryOrigMesh.copy(tongueGeometryOrigMesh, outputTongueData);

    block.outputValue(tongueGeometryOrigCache).set(outputTongueData);
    block.outputValue(tongueGeometryOrigCache).setClean();

    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::computeInitEmotionAccumulator(const MPlug& plug, MDataBlock& block) {
    // Wipe previous results so that it's empty in case of failure.
    MStatus status = SetPluginData<MayaEmotionAccumulatorData>(
        block, MPlug(thisMObject(), emotionAccumulatorAttr), {}
        );
    CHECK_MSTATUS_AND_RETURN_IT(status);

    const maceProfilingScope scope("localAnimationPlayer_computeInitEmotionAccumulator", thisMObject());

    auto backgroundEvaluationDataPtr = GetPluginData<MayaBackgroundEvaluationData>(block, backgroundEvaluationCtxAttr);
    if (!backgroundEvaluationDataPtr || !*backgroundEvaluationDataPtr) { return MStatus::kFailure; }
    auto& backgroundEvaluationData = **backgroundEvaluationDataPtr;

    if (block.context().isNormal()) {
        CHECK_MSTATUS_AND_RETURN_IT(cancelBackgroundEvaluation(backgroundEvaluationData, block));
    }

    auto cudaStreamDataPtr = GetPluginData<MayaCudaStreamData>(block, cudaStreamAttr);
    if (!cudaStreamDataPtr || !*cudaStreamDataPtr) { return MStatus::kFailure; }
    auto& cudaStreamData = **cudaStreamDataPtr;

    auto modelPathValue = mace_maya::GetAttributeAsPath(block, a2fModelPath, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    A2F_CHECK_WARNING_FAILURE(std::filesystem::exists(modelPathValue), "A2F model path does not exist.");

    Audio2FaceModelInfo modelInfo(modelPathValue.string());
    A2F_CHECK_WARNING_FAILURE(modelInfo.isValid(), "Unable to identify a2f model type");
    bool isRegressionModel = modelInfo.isRegression();

    std::size_t nbEmotions = 0;
    nbEmotions = modelInfo.getEmotionCount();

    auto emotionAccumulator = ToUniquePtr(nva2x::CreateEmotionAccumulator(nbEmotions, 300, 0));
    std::vector<float> zeroEmotion(nbEmotions, 0.0f);
    A2F_CHECK_WARNING_FAILURE(!emotionAccumulator->Accumulate(0, nva2x::HostTensorFloatConstView(zeroEmotion.data(), zeroEmotion.size()), cudaStreamData.mCudaStream->Data()), "Failed to accumulate zero emotion");
    A2F_CHECK_WARNING_FAILURE(!emotionAccumulator->Close(), "Failed to close emotion accumulator");

    EmotionAccumulatorData accumulatorData {
        std::move(emotionAccumulator),
        std::move(ToUniquePtr(nva2x::CreateDeviceTensorFloat(nbEmotions))),
        std::move(ToUniquePtr(nva2x::CreateHostTensorFloat(nbEmotions)))
    };

    status = SetPluginData<MayaEmotionAccumulatorData>(
        block,
        MPlug(thisMObject(), emotionAccumulatorAttr),
        std::make_shared<EmotionAccumulatorData>(std::move(accumulatorData))
        );
    CHECK_MSTATUS_AND_RETURN_IT(status);
    
    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::computeInitA2EExecutor(const MPlug& plug, MDataBlock& block) {
    // Wipe previous results so that it's empty in case of failure.
    MStatus status = SetPluginData<MayaA2EExecutorData>(
        block, MPlug(thisMObject(), a2eExecutorAttr), {}
        );
    CHECK_MSTATUS_AND_RETURN_IT(status);
    const maceProfilingScope scope("localAnimationPlayer_computeInitA2EExecutor", thisMObject());

    auto backgroundEvaluationDataPtr = GetPluginData<MayaBackgroundEvaluationData>(block, backgroundEvaluationCtxAttr);
    if (!backgroundEvaluationDataPtr || !*backgroundEvaluationDataPtr) { return MStatus::kFailure; }
    auto& backgroundEvaluationData = **backgroundEvaluationDataPtr;

    if (block.context().isNormal()) {
        CHECK_MSTATUS_AND_RETURN_IT(cancelBackgroundEvaluation(backgroundEvaluationData, block));
    }

    const auto audioAccumulatorDataPtr = GetPluginData<MayaAudioAccumulatorData>(block, audioAccumulatorAttr);
    if (!audioAccumulatorDataPtr || !*audioAccumulatorDataPtr) { return MStatus::kFailure; }
    const auto& audioAccumulatorData = **audioAccumulatorDataPtr;

    const auto cudaStreamDataPtr = GetPluginData<MayaCudaStreamData>(block, cudaStreamAttr);
    if (!cudaStreamDataPtr || !*cudaStreamDataPtr) { return MStatus::kFailure; }
    const auto& cudaStreamData = **cudaStreamDataPtr;
    
    nva2e::EmotionExecutorCreationParameters params;
    params.cudaStream = cudaStreamData.mCudaStream->Data();
    params.nbTracks = 1;
    auto* staticAudioAccumulatorPtr = audioAccumulatorData.mAudioAccumulator.get();
    params.sharedAudioAccumulators = &staticAudioAccumulatorPtr;

    auto modelPathValue = mace_maya::GetAttributeAsPath(block, a2eModelPath, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    auto a2fModelPathValue = mace_maya::GetAttributeAsPath(block, a2fModelPath, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    A2F_CHECK_WARNING_FAILURE(std::filesystem::exists(a2fModelPathValue), "A2F model path does not exist.");
    auto a2fModelInfo = Audio2FaceModelInfo(a2fModelPathValue.string());
    const std::size_t a2fInputEmotionLength = a2fModelInfo.getEmotionCount();

    // These parameters are not exposed in ACE, so we use their default values here.
    // However, it may be necessary to verify that these defaults match those used in ACE to ensure consistent behavior.
    const std::size_t fpsNumerator = 30;
    const std::size_t fpsDenominator = 1;
    const std::size_t a2eInputWindowSize = 60000;
    const std::size_t inferencesToSkip = 30;
    UniquePtr<nva2e::IEmotionInteractiveExecutor> executor;
    if (modelPathValue.string().empty()) {
        // PostProcess-only executor doens't do inference, so the mapping with value -1 means that it doesn't get mapped to any output emotion.
        // The size of the default emotion correspondence vector is the same as the default A2E network. But it can be other values too.
        std::vector<int> defaultEmotionCorrespondence(6, -1);
        nva2e::PostProcessData defaultPostProcessData{
            /*inferenceEmotionLength=*/defaultEmotionCorrespondence.size(), 
            /*outputEmotionLength=*/a2fInputEmotionLength,
            /*emotionCorrespondence=*/defaultEmotionCorrespondence.data(),
            /*emotionCorrespondenceSize=*/defaultEmotionCorrespondence.size()
        };
        std::vector<float> zeroEmotion(defaultPostProcessData.outputEmotionLength, 0.0f);
        nva2e::PostProcessParams defaultPostProcessParams{
            /*emotionContrast=*/1.0f,
            /*maxEmotions=*/0,
            /*beginningEmotion=*/nva2x::HostTensorFloatConstView(zeroEmotion.data(), zeroEmotion.size()),
            /*preferredEmotion=*/nva2x::HostTensorFloatConstView(zeroEmotion.data(), zeroEmotion.size()),
            /*liveBlendCoef=*/0.7f,
            /*enablePreferredEmotion=*/false,
            /*preferredEmotionStrength=*/0.5f,
            /*liveTransitionTime=*/0.5f,
            /*fixedDt=*/0.033f,
            /*emotionStrength=*/0.6f
        };
        nva2e::IPostProcessModel::EmotionExecutorCreationParameters defaultPostProcessExecutorCreationParams{
            /*samplingRate=*/16000,
            /*inputStrength=*/1.0f,
            fpsNumerator,
            fpsDenominator,
            defaultPostProcessData,
            defaultPostProcessParams,
            /*sharedPreferredEmotionAccumulators=*/nullptr
        };
        executor = ToUniquePtr(nva2e::CreatePostProcessEmotionInteractiveExecutor(params, defaultPostProcessExecutorCreationParams));
    } else if (isClassifierModel(modelPathValue.string())) {
        A2F_CHECK_WARNING_FAILURE(std::filesystem::exists(modelPathValue), "A2E model path does not exist.");
        auto modelInfo = ToUniquePtr(nva2e::ReadClassifierModelInfo(
            modelPathValue.string().c_str()
        ));
        A2F_CHECK_WARNING_FAILURE(modelInfo, "Unable to read A2E Classifier model info");
        // Notice that it produces frames at a different frame rate than the a2f executor.
        auto classifierParams = modelInfo->GetExecutorCreationParameters(            
            a2eInputWindowSize, 
            fpsNumerator, 
            fpsDenominator, 
            inferencesToSkip 
            );
        executor = ToUniquePtr(nva2e::CreateClassifierEmotionInteractiveExecutor(params, classifierParams, /*BatchSize=*/1));
    } else {
        A2F_CHECK_WARNING_FAILURE(std::filesystem::exists(modelPathValue), "A2E model path does not exist.");
        auto modelInfo = ToUniquePtr(nva2e::ReadPostProcessModelInfo(
            modelPathValue.string().c_str()
        ));
        A2F_CHECK_WARNING_FAILURE(modelInfo, "Unable to read A2E PostProcess model info");
        auto postProcessParams = modelInfo->GetExecutorCreationParameters(fpsNumerator, fpsDenominator);
        executor = ToUniquePtr(nva2e::CreatePostProcessEmotionInteractiveExecutor(params, postProcessParams));
    }
    A2F_CHECK_WARNING_FAILURE(executor, "Unable to create emotion executor");

    // Update emotionParams attribute with current values from the executor
    {
        nva2e::PostProcessParams ppParams;
        A2F_CHECK_WARNING_FAILURE(
            !nva2e::GetInteractiveExecutorPostProcessParameters(*executor, ppParams),
            "Unable to get post process parameters"
        );
        SetAttributeDefault(emotionStrength, ppParams.emotionStrength);
        SetAttributeDefault(emotionContrast, ppParams.emotionContrast);
        SetAttributeDefault(maxEmotions, static_cast<int>(ppParams.maxEmotions));
        SetAttributeDefault(liveBlendCoef, ppParams.liveBlendCoef);
        SetAttributeDefault(enablePreferredEmotion, ppParams.enablePreferredEmotion);
        SetAttributeDefault(preferredEmotionStrength, ppParams.preferredEmotionStrength);
        std::vector<float> preferredEmotionsVector = {
            ppParams.preferredEmotion.Data(), 
            ppParams.preferredEmotion.Data() + ppParams.preferredEmotion.Size()};
        CHECK_MSTATUS_AND_RETURN_IT(SetArrayOutput(block, preferredEmotions, preferredEmotionsVector, true));
    }
    

    A2EExecutorData a2eExecutorData {
        std::move(executor)
    };

    status = SetPluginData<MayaA2EExecutorData>(
        block,
        MPlug(thisMObject(), a2eExecutorAttr),
        std::make_shared<A2EExecutorData>(std::move(a2eExecutorData))
        );
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::computeInitA2FExecutor(const MPlug& plug, MDataBlock& block) {
    // Wipe previous results so that it's empty in case of failure.
    MStatus status = SetPluginData<MayaA2FExecutorData>(
        block, MPlug(thisMObject(), a2fExecutorAttr), {}
        );
    CHECK_MSTATUS_AND_RETURN_IT(status);
    const maceProfilingScope scope("localAnimationPlayer_computeInitA2FExecutor", thisMObject());

    auto backgroundEvaluationDataPtr = GetPluginData<MayaBackgroundEvaluationData>(block, backgroundEvaluationCtxAttr);
    if (!backgroundEvaluationDataPtr || !*backgroundEvaluationDataPtr) { return MStatus::kFailure; }
    auto& backgroundEvaluationData = **backgroundEvaluationDataPtr;

    if (block.context().isNormal()) {
        CHECK_MSTATUS_AND_RETURN_IT(cancelBackgroundEvaluation(backgroundEvaluationData, block));

        // Only add/remove attributes in the normal context, not in the background one.
        CHECK_MSTATUS_AND_RETURN_IT(RemoveArrayAliases(outputWeights, thisMObject()));
        CHECK_MSTATUS_AND_RETURN_IT(RemoveArrayAliases(preferredEmotions, thisMObject()));
        CHECK_MSTATUS_AND_RETURN_IT(RemoveArrayAliases(outputEmotions, thisMObject()));
        CHECK_MSTATUS_AND_RETURN_IT(RemoveArrayAliases(blendshapeFaceMultipliers, thisMObject()));
        CHECK_MSTATUS_AND_RETURN_IT(RemoveArrayAliases(blendshapeFaceOffsets, thisMObject()));
        CHECK_MSTATUS_AND_RETURN_IT(RemoveArrayAliases(blendshapeTongueMultipliers, thisMObject()));
        CHECK_MSTATUS_AND_RETURN_IT(RemoveArrayAliases(blendshapeTongueOffsets, thisMObject()));
    }
    auto modelPathValue = mace_maya::GetAttributeAsPath(block, a2fModelPath, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    A2F_CHECK_WARNING_FAILURE(std::filesystem::exists(modelPathValue), "A2F model path does not exist.");
    const bool useGpuBlendshapeSolverValue = block.inputValue(useGpuBlendshapeSolver).asBool();
    const auto identityIndexValue = block.inputValue(identityIndex).asInt();

    auto modelInfo = Audio2FaceModelInfo(modelPathValue.string());
    A2F_CHECK_WARNING_FAILURE(modelInfo.isValid(), "Unable to identify a2f model type");
    bool isRegressionModel = modelInfo.isRegression();

    const auto audioAccumulatorDataPtr = GetPluginData<MayaAudioAccumulatorData>(block, audioAccumulatorAttr);
    if (!audioAccumulatorDataPtr || !*audioAccumulatorDataPtr) { return MStatus::kFailure; }
    const auto& audioAccumulatorData = **audioAccumulatorDataPtr;

    const auto emotionAccumulatorDataPtr = GetPluginData<MayaEmotionAccumulatorData>(block, emotionAccumulatorAttr);
    if (!emotionAccumulatorDataPtr || !*emotionAccumulatorDataPtr) { return MStatus::kFailure; }
    const auto& emotionAccumulatorData = **emotionAccumulatorDataPtr;

    const auto cudaStreamDataPtr = GetPluginData<MayaCudaStreamData>(block, cudaStreamAttr);
    if (!cudaStreamDataPtr || !*cudaStreamDataPtr) { return MStatus::kFailure; }
    const auto& cudaStreamData = **cudaStreamDataPtr;

    nva2f::GeometryExecutorCreationParameters params;
    params.cudaStream = cudaStreamData.mCudaStream->Data();
    params.nbTracks = 1;
    auto* audioAccumulatorPtr = audioAccumulatorData.mAudioAccumulator.get();
    auto* emotionAccumulatorPtr = emotionAccumulatorData.mEmotionAccumulator.get();
    params.sharedAudioAccumulators = &audioAccumulatorPtr;
    params.sharedEmotionAccumulators = &emotionAccumulatorPtr;

    UniquePtr<nva2f::IGeometryInteractiveExecutor> geometryExecutor = nullptr;
    std::vector<std::string> emotionNames;
    std::size_t audioBufferLength = 0;

    audioBufferLength = modelInfo.getAudioBufferLength();

    geometryExecutor = createGeometryExecutor(
        params, modelPathValue.string().c_str(), identityIndexValue);
    A2F_CHECK_WARNING_FAILURE(geometryExecutor, "Unable to create geometry executor");

    emotionNames = modelInfo.getEmotionNames();
    A2F_CHECK_WARNING_FAILURE(emotionNames.size() > 0, "Unable to get emotion names");
    CHECK_MSTATUS_AND_RETURN_IT(CreateArrayAliases(preferredEmotions, thisMObject(), emotionNames, "preferred_"));
    CHECK_MSTATUS_AND_RETURN_IT(CreateArrayAliases(outputEmotions, thisMObject(), emotionNames, "out_"));

    // Update face & tongue parameters with the current values from the geometry executor
    CHECK_MSTATUS_AND_RETURN_IT(updateGeometryParameterDefaults(block, geometryExecutor.get()));

    // return if blendshape executor is not available
    if (!modelInfo.hasBlendshapeInfo()) {
        A2FExecutorData a2fExecutorData {
            nullptr,
            std::move(geometryExecutor),
            audioBufferLength
        };

        status = SetPluginData<MayaA2FExecutorData>(
            block,
            MPlug(thisMObject(), a2fExecutorAttr),
            std::make_shared<A2FExecutorData>(std::move(a2fExecutorData))
            );
        CHECK_MSTATUS_AND_RETURN_IT(status);
        return MS::kSuccess;
    }

    // else create blendshape executor. It can fail if blendshape data is defined and invalid
    UniquePtr<nva2f::IBlendshapeInteractiveExecutor> blendshapeSolveExecutor = nullptr;
    std::vector<std::string> facePoseNames{};
    std::vector<std::string> tonguePoseNames{};
    blendshapeSolveExecutor = createBlendshapeExecutor(
        std::move(geometryExecutor), modelPathValue.string(), facePoseNames, tonguePoseNames, useGpuBlendshapeSolverValue, identityIndexValue);
    A2F_CHECK_WARNING_FAILURE(blendshapeSolveExecutor, "Unable to create blendshape solve executor");

    // update pose names (blendshape output weight aliases)
    if (block.context().isNormal()) {
        // Only add/remove attributes in the normal context, not in the background one.
        CHECK_MSTATUS_AND_RETURN_IT(createBlendshapeAliases(block, facePoseNames, tonguePoseNames));
    }

    // Update blendshape parameters with the current values from the blendshape solve executor
    CHECK_MSTATUS_AND_RETURN_IT(resetBlendshapeMultipliersAndOffsets(block, blendshapeSolveExecutor));

    A2FExecutorData a2fExecutorData {
        std::move(blendshapeSolveExecutor),
        nullptr,
        audioBufferLength
    };

    status = SetPluginData<MayaA2FExecutorData>(
        block,
        MPlug(thisMObject(), a2fExecutorAttr),
        std::make_shared<A2FExecutorData>(std::move(a2fExecutorData))
        );
    CHECK_MSTATUS_AND_RETURN_IT(status);
    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::updateGeometryParameterDefaults(
    MDataBlock& block, nva2f::IGeometryInteractiveExecutor* geometryExecutor) {
    {
        nva2f::AnimatorSkinParams params;
        A2F_CHECK_WARNING_FAILURE(
            !nva2f::GetInteractiveExecutorSkinParameters(*geometryExecutor, params),
            "Unable to get face parameters"
        );
        SetAttributeDefault(lowerFaceSmoothing, params.lowerFaceSmoothing);
        SetAttributeDefault(upperFaceSmoothing, params.upperFaceSmoothing);
        SetAttributeDefault(lowerFaceStrength, params.lowerFaceStrength);
        SetAttributeDefault(faceMaskLevel, params.faceMaskLevel);
        SetAttributeDefault(faceMaskSoftness, params.faceMaskSoftness);
        SetAttributeDefault(skinStrength, params.skinStrength);
        SetAttributeDefault(eyelidOpenOffset, params.eyelidOpenOffset);
        SetAttributeDefault(lipOpenOffset, params.lipOpenOffset);
    }
    {
        nva2f::AnimatorTongueParams params;
        A2F_CHECK_WARNING_FAILURE(
            !nva2f::GetInteractiveExecutorTongueParameters(*geometryExecutor, params),
            "Unable to get tongue parameters"
        );
        SetAttributeDefault(tongueStrength, params.tongueStrength);
        SetAttributeDefault(tongueHeightOffset, params.tongueHeightOffset);
        SetAttributeDefault(tongueDepthOffset, params.tongueDepthOffset);
    }
    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::createBlendshapeAliases(
    MDataBlock& block, 
    const std::vector<std::string>& facePoseNames,
    const std::vector<std::string>& tonguePoseNames)
{
    // update pose names (blendshape output weight aliases)
    CHECK_MSTATUS_AND_RETURN_IT(CreateArrayAliases(blendshapeFaceMultipliers, thisMObject(), facePoseNames, "faceMultiplier_"));
    CHECK_MSTATUS_AND_RETURN_IT(CreateArrayAliases(blendshapeFaceOffsets, thisMObject(), facePoseNames, "faceOffset_"));

    CHECK_MSTATUS_AND_RETURN_IT(CreateArrayAliases(blendshapeTongueMultipliers, thisMObject(), tonguePoseNames, "tongueMultiplier_"));
    CHECK_MSTATUS_AND_RETURN_IT(CreateArrayAliases(blendshapeTongueOffsets, thisMObject(), tonguePoseNames, "tongueOffset_"));

    std::vector<std::string> outputPoseNames;
    outputPoseNames.insert(outputPoseNames.end(), facePoseNames.begin(), facePoseNames.end());
    outputPoseNames.insert(outputPoseNames.end(), tonguePoseNames.begin(), tonguePoseNames.end());
    CHECK_MSTATUS_AND_RETURN_IT(CreateArrayAliases(outputWeights, thisMObject(), outputPoseNames, "out_"));

    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::resetBlendshapeMultipliersAndOffsets(
    MDataBlock& block,
    UniquePtr<nva2f::IBlendshapeInteractiveExecutor>& blendshapeSolveExecutor) 
{
    // Update blendshape parameters with the current values from the blendshape solve executor
    nva2f::BlendshapeSolverConfig faceSolverConfig;
    A2F_CHECK_WARNING_FAILURE(!nva2f::GetInteractiveExecutorBlendshapeSkinConfig(*blendshapeSolveExecutor, faceSolverConfig), "Unable to get face solver config");
    nva2f::BlendshapeSolverConfig tongueSolverConfig;
    A2F_CHECK_WARNING_FAILURE(!nva2f::GetInteractiveExecutorBlendshapeTongueConfig(*blendshapeSolveExecutor, tongueSolverConfig), "Unable to get tongue solver config");
    
    std::vector<float> faceMultiplierValues(faceSolverConfig.multipliers.Data(), faceSolverConfig.multipliers.Data() + faceSolverConfig.multipliers.Size());
    std::vector<float> faceOffsetValues(faceSolverConfig.offsets.Data(), faceSolverConfig.offsets.Data() + faceSolverConfig.offsets.Size());
    std::vector<float> tongueMultiplierValues(tongueSolverConfig.multipliers.Data(), tongueSolverConfig.multipliers.Data() + tongueSolverConfig.multipliers.Size());
    std::vector<float> tongueOffsetValues(tongueSolverConfig.offsets.Data(), tongueSolverConfig.offsets.Data() + tongueSolverConfig.offsets.Size());
    CHECK_MSTATUS_AND_RETURN_IT(SetArrayOutput(block, blendshapeFaceMultipliers, faceMultiplierValues, true));
    CHECK_MSTATUS_AND_RETURN_IT(SetArrayOutput(block, blendshapeFaceOffsets, faceOffsetValues, true));
    CHECK_MSTATUS_AND_RETURN_IT(SetArrayOutput(block, blendshapeTongueMultipliers, tongueMultiplierValues, true));
    CHECK_MSTATUS_AND_RETURN_IT(SetArrayOutput(block, blendshapeTongueOffsets, tongueOffsetValues, true));

    // Reset the multiplier and offset in "solvers" to 1.0 and 0.0
    // We manually apply multipliers and offsets after getting the results from the blendshape solve executor.
    // It's a workaround to make sure multipliers and offsets are keyable.
    {
        const auto numBlendshapePoses = faceSolverConfig.multipliers.Size();
        std::vector<float> multipliers(numBlendshapePoses, 1.0f);
        std::vector<float> offsets(numBlendshapePoses, 0.0f);
        faceSolverConfig.multipliers = nva2x::HostTensorFloatConstView{multipliers.data(), multipliers.size()};
        faceSolverConfig.offsets = nva2x::HostTensorFloatConstView{offsets.data(), offsets.size()};
        A2F_CHECK_WARNING_FAILURE(!nva2f::SetInteractiveExecutorBlendshapeSkinConfig(*blendshapeSolveExecutor, faceSolverConfig), "Unable to set face solver config");
    }
    {
        const auto numBlendshapePoses = tongueSolverConfig.multipliers.Size();
        std::vector<float> multipliers(numBlendshapePoses, 1.0f);
        std::vector<float> offsets(numBlendshapePoses, 0.0f);
        tongueSolverConfig.multipliers = nva2x::HostTensorFloatConstView{multipliers.data(), multipliers.size()};
        tongueSolverConfig.offsets = nva2x::HostTensorFloatConstView{offsets.data(), offsets.size()};
        A2F_CHECK_WARNING_FAILURE(!nva2f::SetInteractiveExecutorBlendshapeTongueConfig(*blendshapeSolveExecutor, tongueSolverConfig), "Unable to set tongue solver config");
    }
    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::computeInitAccumulatedAudioAccumulator(const MPlug& plug, MDataBlock& block) {
    // Wipe previous results so that it's empty in case of failure.
    MStatus status = SetPluginData<MayaAudioAccumulatorData>(
        block, MPlug(thisMObject(), accumulatedAudioAccumulatorAttr), {}
        );
    CHECK_MSTATUS_AND_RETURN_IT(status);
    const maceProfilingScope scope("localAnimationPlayer_computeInitAccumulatedAudioAccumulator", thisMObject());

    if (block.context().isNormal()) {
        const auto backgroundEvaluationDataPtr = GetPluginData<MayaBackgroundEvaluationData>(block, backgroundEvaluationCtxAttr);
        if (!backgroundEvaluationDataPtr || !*backgroundEvaluationDataPtr) { return MStatus::kFailure; }
        auto& backgroundEvaluationData = **backgroundEvaluationDataPtr;
        CHECK_MSTATUS_AND_RETURN_IT(cancelBackgroundEvaluation(backgroundEvaluationData, block));
    }

    const auto a2fExecutorDataPtr = GetPluginData<MayaA2FExecutorData>(block, a2fExecutorAttr);
    if (!a2fExecutorDataPtr || !*a2fExecutorDataPtr) { return MStatus::kFailure; }
    const auto& a2fExecutorData = **a2fExecutorDataPtr;

    // Load audio
    auto audioBuffer = [&block, this, sampleRate = a2fExecutorData.getSamplingRate()]() -> std::vector<float> {
        MStatus status;

        MDataHandle audiofileHandle = block.inputValue(audiofile, &status);
        CHECK_MSTATUS_AND_RETURN(status, std::vector<float>());
        std::filesystem::path audiofilePath(audiofileHandle.asString().asUTF8());
        A2F_CHECK_WARNING(std::filesystem::exists(audiofilePath), "audio file path does not exist.");

        auto rawAudioBuffer = get_file_wav_content(audiofilePath.string().c_str(), sampleRate);

        auto offset = block.inputValue(audioOffset).asTime().asUnits(MTime::kSeconds);
        auto start = block.inputValue(audioStart).asTime().asUnits(MTime::kSeconds);
        auto end = block.inputValue(audioEnd).asTime().asUnits(MTime::kSeconds);

        // recalculate Audio End that is not bigger than audio length, and not negative
        if (end < 0) {
            end = rawAudioBuffer.size() / sampleRate;
        }
        end = std::min(end, (double)rawAudioBuffer.size() / sampleRate);

        std::vector<float> audioBuffer;
        // Add leading silence if offset is positive, otherwise adjust start point.
        if (offset > 0) {
            audioBuffer.insert(audioBuffer.end(), offset * sampleRate, 0.0f);
        } else {
            start += -offset;
        }
        std::size_t startSample = start * sampleRate;
        std::size_t endSample = end * sampleRate;
        if (startSample < endSample && startSample < rawAudioBuffer.size() && endSample <= rawAudioBuffer.size()) {
            audioBuffer.insert(audioBuffer.end(), rawAudioBuffer.begin() + startSample, rawAudioBuffer.begin() + endSample);
        }

        return audioBuffer;
    }();
    A2F_CHECK_WARNING_FAILURE(!audioBuffer.empty(), "Unable to get audio buffer");

    const auto audioAccumulatorDataPtr = GetPluginData<MayaAudioAccumulatorData>(block, audioAccumulatorAttr);
    if (!audioAccumulatorDataPtr || !*audioAccumulatorDataPtr) { return MStatus::kFailure; }
    auto& audioAccumulatorData = **audioAccumulatorDataPtr;

    {
        const auto cudaStreamDataPtr = GetPluginData<MayaCudaStreamData>(block, cudaStreamAttr);
        if (!cudaStreamDataPtr || !*cudaStreamDataPtr) { return MStatus::kFailure; }
        auto& cudaStreamData = **cudaStreamDataPtr;

        const auto a2eExecutorDataPtr = GetPluginData<MayaA2EExecutorData>(block, a2eExecutorAttr);
        if (!a2eExecutorDataPtr || !*a2eExecutorDataPtr) { return MStatus::kFailure; }
        auto& a2eExecutorData = **a2eExecutorDataPtr;

        const auto a2fExecutorDataPtr = GetPluginData<MayaA2FExecutorData>(block, a2fExecutorAttr);
        if (!a2fExecutorDataPtr || !*a2fExecutorDataPtr) { return MStatus::kFailure; }
        auto& a2fExecutorData = **a2fExecutorDataPtr;

        auto& staticAudioAccumulator = *audioAccumulatorData.mAudioAccumulator;
        auto& cudaStream = *cudaStreamData.mCudaStream;
        auto& emotionExecutor = *a2eExecutorData.mEmotionInteractiveExecutor;
        auto& faceExecutor = *a2fExecutorData.getFaceExecutor();
       
        // Accumulate the entire audio buffer for a2e interactive executor.
        A2F_CHECK_WARNING_FAILURE(!staticAudioAccumulator.Reset(), "Unable to reset audio accumulator");
        A2F_CHECK_WARNING_FAILURE(!staticAudioAccumulator.Accumulate(nva2x::HostTensorFloatConstView{audioBuffer.data(), audioBuffer.size()}, cudaStream.Data()), "Unable to accumulate audio");
        A2F_CHECK_WARNING_FAILURE(!staticAudioAccumulator.Close(), "Unable to close audio accumulator");
        A2F_CHECK_WARNING_FAILURE(!emotionExecutor.Invalidate(nva2e::IEmotionInteractiveExecutor::kLayerAll), "Unable to invalidate emotion executor");
        A2F_CHECK_WARNING_FAILURE(!faceExecutor.Invalidate(nva2f::IFaceInteractiveExecutor::kLayerAll), "Unable to invalidate face executor");
    }

    status = SetPluginData<MayaAudioAccumulatorData>(
        block,
        MPlug(thisMObject(), accumulatedAudioAccumulatorAttr),
        *audioAccumulatorDataPtr
        );
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::updateA2EExecutorParameters(nva2e::IEmotionInteractiveExecutor& emotionExecutor, MDataBlock& block) {
    nva2e::PostProcessParams params;
    A2F_CHECK_WARNING_FAILURE(
        !nva2e::GetInteractiveExecutorPostProcessParameters(emotionExecutor, params),
        "Unable to get emotion post processing parameters"
    );
    params.emotionContrast = block.inputValue(emotionContrast).asFloat();
    params.maxEmotions = block.inputValue(maxEmotions).asInt();
    params.liveBlendCoef = block.inputValue(liveBlendCoef).asFloat();
    params.enablePreferredEmotion = block.inputValue(enablePreferredEmotion).asBool();
    params.preferredEmotionStrength = block.inputValue(preferredEmotionStrength).asFloat();
    params.emotionStrength = block.inputValue(emotionStrength).asFloat();
    std::vector<float> preferredEmotion;
    CHECK_MSTATUS_AND_RETURN_IT(GetArrayInput(block, preferredEmotions, preferredEmotion));
    if (preferredEmotion.size() > 0) {
        params.preferredEmotion = {preferredEmotion.data(), preferredEmotion.size()};
    }
    A2F_CHECK_WARNING_FAILURE(
        !nva2e::SetInteractiveExecutorPostProcessParameters(emotionExecutor, params),
        "Unable to set emotion post processing parameters"
    );

    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::runA2EExecutorOffline(
    nva2x::IAudioAccumulator& staticAudioAccumulator, 
    nva2e::IEmotionInteractiveExecutor& emotionExecutor, 
    nva2x::IEmotionAccumulator& emotionAccumulator) 
{
    assert(staticAudioAccumulator.IsClosed());
    assert(!emotionAccumulator.IsClosed());

    auto callback = [](void* userdata, const nva2e::IEmotionInteractiveExecutor::Results& results) {
        assert(results.trackIndex == 0);
        auto& emotionAccumulator = *static_cast<nva2x::IEmotionAccumulator*>(userdata);
        [[maybe_unused]] const auto success = emotionAccumulator.Accumulate(results.timeStampCurrentFrame, results.emotions, results.cudaStream);
        assert(!success);
        return true;
    };
    A2F_CHECK_WARNING_FAILURE(
        !emotionExecutor.SetResultsCallback(callback, &emotionAccumulator),
        "Unable to set emotionExecutor results callback"
    );

    // Process all emotion.
    {
        const auto result = emotionExecutor.ComputeAllFrames();
        A2F_CHECK_WARNING_FAILURE(
            result == nva2x::ErrorCode::eSuccess || 
            result == nva2x::ErrorCode::eInterrupted, "Unable to compute all frames");
    }
    A2F_CHECK_WARNING_FAILURE(
        !emotionAccumulator.Close(),
        "Unable to close emotion accumulator"
    );

    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::updateA2FExecutorParameters(nva2f::IFaceInteractiveExecutor& geometryExecutor, MDataBlock& block) {
    {
        nva2f::AnimatorSkinParams params;
        A2F_CHECK_WARNING_FAILURE(
            !nva2f::GetInteractiveExecutorSkinParameters(geometryExecutor, params),
            "Unable to get face parameters"
        );
        params.lowerFaceSmoothing = block.inputValue(lowerFaceSmoothing).asFloat();
        params.upperFaceSmoothing = block.inputValue(upperFaceSmoothing).asFloat();
        params.lowerFaceStrength = block.inputValue(lowerFaceStrength).asFloat();
        params.upperFaceStrength = block.inputValue(upperFaceStrength).asFloat();
        params.faceMaskLevel = block.inputValue(faceMaskLevel).asFloat();
        params.faceMaskSoftness = block.inputValue(faceMaskSoftness).asFloat();
        params.skinStrength = block.inputValue(skinStrength).asFloat();
        params.eyelidOpenOffset = block.inputValue(eyelidOpenOffset).asFloat();
        params.lipOpenOffset = block.inputValue(lipOpenOffset).asFloat();
        A2F_CHECK_WARNING_FAILURE(
            !nva2f::SetInteractiveExecutorSkinParameters(geometryExecutor, params),
            "Unable to set face parameters"
        );
    }
    {
        nva2f::AnimatorTongueParams params;
        A2F_CHECK_WARNING_FAILURE(
            !nva2f::GetInteractiveExecutorTongueParameters(geometryExecutor, params),
            "Unable to get tongue parameters"
        );
        params.tongueStrength = block.inputValue(tongueStrength).asFloat();
        params.tongueHeightOffset = block.inputValue(tongueHeightOffset).asFloat();
        params.tongueDepthOffset = block.inputValue(tongueDepthOffset).asFloat();
        A2F_CHECK_WARNING_FAILURE(
            !nva2f::SetInteractiveExecutorTongueParameters(geometryExecutor, params),
            "Unable to set tongue parameters"
        );
    }
    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::runA2FExecutorOffline(
    nva2x::ICudaStream& cudaStream,
    nva2x::IAudioAccumulator& audioAccumulator,
    nva2x::IEmotionAccumulator& emotionAccumulator,
    nva2f::IFaceInteractiveExecutor& faceExecutor,
    ExecutorResultsData& executorResultsData,
    double targetTime,
    bool quickEstimate
) {
    assert(audioAccumulator.IsClosed());
    assert(emotionAccumulator.IsClosed());

    const auto totalNbFrames = faceExecutor.GetTotalNbFrames();
    const auto nbFramesToCompute = quickEstimate ? 1 : totalNbFrames;
    assert(executorResultsData.nbFrames >= nbFramesToCompute);

    std::size_t targetFrameIndex = 0;
    {
        std::size_t frameRateNum, frameRateDenom;
        faceExecutor.GetFrameRate(frameRateNum, frameRateDenom);
        assert(executorResultsData.frameRateNum == frameRateNum);
        assert(executorResultsData.frameRateDenom == frameRateDenom);
        targetFrameIndex = std::clamp(static_cast<std::size_t>(std::round(targetTime * frameRateNum)) / frameRateDenom, std::size_t(0), totalNbFrames - 1);
    }

    nva2f::IGeometryInteractiveExecutor* geometryExecutor = nullptr;
    auto blendshapeSolveExecutor = getFaceExecutorAs<nva2f::IBlendshapeInteractiveExecutor>(&faceExecutor);

    if (blendshapeSolveExecutor) {
        nva2f::BlendshapeSolverConfig faceSolverConfig;
        nva2f::BlendshapeSolverConfig tongueSolverConfig;
        A2F_CHECK_WARNING_FAILURE(
            !nva2f::GetInteractiveExecutorBlendshapeSkinConfig(*blendshapeSolveExecutor, faceSolverConfig),
            "Unable to get face solver config"
        );
        A2F_CHECK_WARNING_FAILURE(
            !nva2f::GetInteractiveExecutorBlendshapeTongueConfig(*blendshapeSolveExecutor, tongueSolverConfig),
            "Unable to get tongue solver config"
        );
        assert(executorResultsData.blendshapeExecutorResultsData.faceWeightsCount 
            == faceSolverConfig.multipliers.Size());
        assert(executorResultsData.blendshapeExecutorResultsData.tongueWeightsCount 
            == tongueSolverConfig.multipliers.Size());
        A2F_CHECK_WARNING_FAILURE(
            !nva2f::GetInteractiveExecutorGeometryExecutor(*blendshapeSolveExecutor, &geometryExecutor), 
            "Unable to get geometry executor");
    }
    else {
        geometryExecutor = getFaceExecutorAs<nva2f::IGeometryInteractiveExecutor>(&faceExecutor);
    }
    A2F_CHECK_WARNING_FAILURE(geometryExecutor, "Unable to get geometry executor. Null pointer.");
    
    // Geometry results
    const auto geometryResultSize =
        geometryExecutor->GetSkinGeometrySize() + geometryExecutor->GetTongueGeometrySize() +
        geometryExecutor->GetJawTransformSize() + geometryExecutor->GetEyesRotationSize();
    assert(executorResultsData.geometryExecutorResultsData.faceSize == geometryExecutor->GetSkinGeometrySize());
    assert(executorResultsData.geometryExecutorResultsData.tongueSize == geometryExecutor->GetTongueGeometrySize());
    assert(executorResultsData.geometryExecutorResultsData.jawSize == geometryExecutor->GetJawTransformSize());
    assert(executorResultsData.geometryExecutorResultsData.eyesSize == geometryExecutor->GetEyesRotationSize());
    assert(executorResultsData.geometryExecutorResultsData.resultsBuffer->Size() >= nbFramesToCompute * geometryResultSize);
    
    // Set callback for geometry results
    struct GeometryCallbackData {
        TimeSliderProgress::CustomDrawID context;
        nva2x::HostTensorFloatView output;
        cudaStream_t cudaStream;
        std::size_t geometryResultSize;
        std::size_t frameIndex;
        std::size_t sampleRate;
        bool drawProgress;
    };
    GeometryCallbackData geometryCallbackData{
        m_customDrawContext,
        *executorResultsData.geometryExecutorResultsData.resultsBuffer, 
        cudaStream.Data(), 
        geometryResultSize, 
        0,
        faceExecutor.GetSamplingRate(),
        !quickEstimate
    };
    nva2f::IGeometryExecutor::results_callback_t geometryCallback = [](void* userdata, const nva2f::IGeometryExecutor::Results& results) -> bool {
        assert(results.trackIndex == 0);
        auto& data = *static_cast<GeometryCallbackData*>(userdata);
        // We assume it's all the same stream, otherwise extra synchronization will be needed.
        assert(data.cudaStream == results.skinCudaStream);
        assert(data.cudaStream == results.tongueCudaStream);
        assert(data.cudaStream == results.jawCudaStream);
        assert(data.cudaStream == results.eyesCudaStream);

        const auto output = data.output.View(data.frameIndex++ * data.geometryResultSize, data.geometryResultSize);
        
        // This could look nicer if all results were on the GPU.
        // Instead, we re-upload them to make it easier to pass them around using existing nodes.
        std::size_t offset = 0;
        A2F_CHECK_WARNING_FAILURE(
            !nva2x::CopyDeviceToHost(
                output.View(offset, results.skinGeometry.Size()), results.skinGeometry, results.skinCudaStream
            ),
            "Unable to copy face geometry"
        );
        offset += results.skinGeometry.Size();
        A2F_CHECK_WARNING_FAILURE(
            !nva2x::CopyDeviceToHost(
                output.View(offset, results.tongueGeometry.Size()), results.tongueGeometry, results.tongueCudaStream
            ),
            "Unable to copy tongue geometry"
        );
        offset += results.tongueGeometry.Size();
        A2F_CHECK_WARNING_FAILURE(
            !nva2x::CopyDeviceToHost(
                output.View(offset, results.jawTransform.Size()), results.jawTransform, results.jawCudaStream
            ),
            "Unable to copy jaw transform"
        );
        offset += results.jawTransform.Size();
        A2F_CHECK_WARNING_FAILURE(
            !nva2x::CopyDeviceToHost(
                output.View(offset, results.eyesRotation.Size()), results.eyesRotation, results.eyesCudaStream
            ),
            "Unable to copy right eye rotation"
        );
        offset += results.eyesRotation.Size();
        assert(offset == data.geometryResultSize);

        if (data.drawProgress) {
            TimeSliderProgress::draw(data.context, static_cast<double>(results.timeStampNextFrame) / data.sampleRate, /*isPending=*/false, /*done=*/false);
        }
        return true;
    };
    A2F_CHECK_WARNING_FAILURE(
        !nva2f::SetInteractiveExecutorGeometryResultsCallback(faceExecutor, geometryCallback, &geometryCallbackData),
        "Unable to set geometry results callback"
    );

    // return if blendshape executor is not available
    if (!blendshapeSolveExecutor) {
        assert(faceExecutor.GetTotalNbFrames());
        if (quickEstimate) {
            const maceProfilingScope scope("localAnimationPlayer_quickEstimate_geometry", thisMObject());
            faceExecutor.ComputeFrame(targetFrameIndex);
        } else {
            auto result = faceExecutor.ComputeAllFrames();
            A2F_CHECK_WARNING_FAILURE(
                result == nva2x::ErrorCode::eSuccess || 
                result == nva2x::ErrorCode::eInterrupted, "Unable to compute all frames");
        }
        executorResultsData.nbActualFrames = geometryCallbackData.frameIndex;
        return MS::kSuccess;
    }

    // Blendshape results
    const auto blendshapeResultsSize = blendshapeSolveExecutor->GetWeightCount();
    assert(executorResultsData.blendshapeExecutorResultsData.resultsBuffer->Size() >= nbFramesToCompute * blendshapeResultsSize);

    // Set callback for blendshape results
    struct BlendshapeCallbackData {
        TimeSliderProgress::CustomDrawID context;
        nva2x::HostTensorFloatView output;
        cudaStream_t cudaStream;
        std::size_t resultSize;
        std::size_t frameIndex;
        std::size_t sampleRate;
        bool drawProgress;
    };
    
    BlendshapeCallbackData blendshapeCallbackData{
        m_customDrawContext,
        *executorResultsData.blendshapeExecutorResultsData.resultsBuffer,
        cudaStream.Data(),
        blendshapeResultsSize,
        0,
        blendshapeSolveExecutor->GetSamplingRate(),
        !quickEstimate
    };
    
    if (blendshapeSolveExecutor->GetResultType() == nva2f::IBlendshapeInteractiveExecutor::ResultsType::HOST) {
        nva2f::IBlendshapeExecutor::host_results_callback_t callback = [](
            void* userdata, const nva2f::IBlendshapeExecutor::HostResults& results, std::error_code errorCode
            ) -> void {
            assert(results.trackIndex == 0);
            assert(results.weights.Data() != nullptr);
            A2F_CHECK_WARNING_RETURN(!errorCode, "Received an error in the callback",);

            auto& data = *static_cast<BlendshapeCallbackData*>(userdata);

            assert(results.weights.Size() == data.resultSize);
            const auto output = data.output.View(data.frameIndex++ * data.resultSize, data.resultSize);
            A2F_CHECK_WARNING_RETURN(
                !nva2x::CopyHostToHost(output, results.weights, data.cudaStream),
                "Unable to copy blendshape results",
            );

            TimeSliderProgress::draw(data.context, static_cast<double>(results.timeStampNextFrame) / data.sampleRate, /*isPending=*/false, /*done=*/false);
        };

        A2F_CHECK_WARNING_FAILURE(
            !blendshapeSolveExecutor->SetResultsCallback(callback, &blendshapeCallbackData),
            "Unable to set host blendshapeSolveExecutor results callback"
        );
    } else if (blendshapeSolveExecutor->GetResultType() == nva2f::IBlendshapeInteractiveExecutor::ResultsType::DEVICE) {
        nva2f::IBlendshapeExecutor::device_results_callback_t callback = [](void* userdata, const nva2f::IBlendshapeExecutor::DeviceResults& results) -> bool {
            assert(results.trackIndex == 0);
            assert(results.weights.Data() != nullptr);
            auto& data = *static_cast<BlendshapeCallbackData*>(userdata);

            // We assume it's all the same stream, otherwise extra synchronization will be needed.
            assert(data.cudaStream == results.cudaStream);
            assert(results.weights.Size() == data.resultSize);
            const auto output = data.output.View(data.frameIndex++ * data.resultSize, data.resultSize);
            A2F_CHECK_WARNING_FAILURE(
                !nva2x::CopyDeviceToHost(output, results.weights, data.cudaStream),
                "Unable to copy blendshape results"
            );

            if (data.drawProgress) {
                TimeSliderProgress::draw(data.context, static_cast<double>(results.timeStampNextFrame) / data.sampleRate, /*isPending=*/false, /*done=*/false);
            }
            return true;
        };

        A2F_CHECK_WARNING_FAILURE(
            !blendshapeSolveExecutor->SetResultsCallback(callback, &blendshapeCallbackData),
            "Unable to set device blendshapeSolveExecutor results callback"
        );
    } else {
        A2F_CHECK_WARNING_FAILURE(false, "Unsupported blendshape executor result type");
    }

    // Run everything!
    {
        assert(blendshapeSolveExecutor->GetTotalNbFrames() > 0);
        if (quickEstimate) {
            const maceProfilingScope scope("localAnimationPlayer_quickEstimate_blendshape", thisMObject());
            blendshapeSolveExecutor->ComputeFrame(targetFrameIndex);
        } else {
            auto result = faceExecutor.ComputeAllFrames();
            A2F_CHECK_WARNING_FAILURE(
                result == nva2x::ErrorCode::eSuccess || 
                result == nva2x::ErrorCode::eInterrupted, "Unable to compute all frames");
        }
    }
    executorResultsData.nbActualFrames = blendshapeCallbackData.frameIndex;

    return MS::kSuccess;
}

long A2FAnimationPlayer::computeFrameIndex(
    MDataBlock& block, 
    std::size_t frameRateNum, std::size_t frameRateDenom, 
    std::size_t nbFrames, 
    std::size_t sampleRate,
    MStatus* status) {
    const MTime timeValue = block.inputValue(time).asTime();
    const double sampleTime = timeValue.asUnits(MTime::kSeconds);
    const std::size_t frameIndex = static_cast<std::size_t>(std::round(sampleTime * frameRateNum)) / frameRateDenom;
    return static_cast<long>(std::clamp(frameIndex, std::size_t(0), nbFrames - 1));
}

MStatus A2FAnimationPlayer::backgroundEvaluationThreadFunc(
    nva2x::ICudaStream* cudaStream,
    nva2x::IAudioAccumulator* staticAudioAccumulator,
    nva2x::IEmotionAccumulator* emotionAccumulator,
    nva2e::IEmotionInteractiveExecutor* emotionExecutor,
    nva2f::IFaceInteractiveExecutor* faceExecutor,
    BackgroundEvaluationData* bgData,
    ExecutorResultsData* executorResultsData
) {

    TimeSliderProgress::draw(m_customDrawContext, static_cast<double>(staticAudioAccumulator->NbAccumulatedSamples()) / 
        faceExecutor->GetSamplingRate(), /*isPending*/true);

    {
        // Delay starting the background evaluation thread to avoid canceling it too often
        std::unique_lock<std::mutex> lock(bgData->mStartedMtx);
        if (bgData->mStartedCV.wait_for(lock, std::chrono::milliseconds(300), [&bgData]() {
            return !bgData->mBackgroundThreadRunning.load(std::memory_order_acquire);
        })) {
            // cancel before timeout
            return MS::kSuccess;
        }
    }

    CHECK_MSTATUS_AND_RETURN_IT(runA2FExecutorOffline(
        *cudaStream,
        *staticAudioAccumulator,
        *emotionAccumulator,
        *faceExecutor,
        *executorResultsData,
        0, false));

    if (!bgData->mBackgroundThreadRunning.load(std::memory_order_acquire)) {
        return MS::kSuccess;
    }

    A2F_CHECK_WARNING_FAILURE(!cudaStream->Synchronize(), "Unable to synchronize cuda stream");
    assert(executorResultsData->nbActualFrames == executorResultsData->nbFrames);
    bgData->mBackgroundThreadFinished.store(true, std::memory_order_release);

    TimeSliderProgress::draw(m_customDrawContext, static_cast<double>(staticAudioAccumulator->NbAccumulatedSamples()) / 
        faceExecutor->GetSamplingRate(), /*isPending=*/false, /*done=*/true);
    return MS::kSuccess;
} 

A2XParameters A2FAnimationPlayer::getParameters(MDataBlock& block) {
    A2XParameters params;
    params.emotion.emotionStrength = block.inputValue(emotionStrength).asFloat();
    params.emotion.emotionContrast = block.inputValue(emotionContrast).asFloat();
    params.emotion.maxEmotions = block.inputValue(maxEmotions).asFloat();
    params.emotion.liveBlendCoef = block.inputValue(liveBlendCoef).asFloat();
    params.emotion.enablePreferredEmotion = block.inputValue(enablePreferredEmotion).asBool();
    params.emotion.preferredEmotionStrength = block.inputValue(preferredEmotionStrength).asFloat();
    std::vector<float> cur_preferred_emotions;
    GetArrayInput(block, preferredEmotions, cur_preferred_emotions);
    params.setPreferredEmotions(cur_preferred_emotions);

    params.face.lowerFaceSmoothing = block.inputValue(lowerFaceSmoothing).asFloat();
    params.face.upperFaceSmoothing = block.inputValue(upperFaceSmoothing).asFloat();
    params.face.lowerFaceStrength = block.inputValue(lowerFaceStrength).asFloat();
    params.face.upperFaceStrength = block.inputValue(upperFaceStrength).asFloat();
    params.face.faceMaskLevel = block.inputValue(faceMaskLevel).asFloat();
    params.face.faceMaskSoftness = block.inputValue(faceMaskSoftness).asFloat();
    params.face.skinStrength = block.inputValue(skinStrength).asFloat();
    params.face.eyelidOpenOffset = block.inputValue(eyelidOpenOffset).asFloat();
    params.face.lipOpenOffset = block.inputValue(lipOpenOffset).asFloat();

    params.tongue.tongueStrength = block.inputValue(tongueStrength).asFloat();
    params.tongue.tongueHeightOffset = block.inputValue(tongueHeightOffset).asFloat();
    params.tongue.tongueDepthOffset = block.inputValue(tongueDepthOffset).asFloat();

    return params;
}

MStatus A2FAnimationPlayer::allocateExecutorResultsData(MDataBlock& block, ExecutorResultsData& executorResultsData, std::size_t nbFrames) {
    // Allocate results data that can hold N frames of results (based on the accumulated audio size or explicitly set nbFrames by the user)
    const auto emotionAccumulatorDataPtr = GetPluginData<MayaEmotionAccumulatorData>(block, emotionAccumulatorAttr);
    if (!emotionAccumulatorDataPtr || !*emotionAccumulatorDataPtr) { return MStatus::kFailure; }
    const auto& emotionAccumulatorData = **emotionAccumulatorDataPtr;
    
    const auto a2fExecutorDataPtr = GetPluginData<MayaA2FExecutorData>(block, a2fExecutorAttr);
    if (!a2fExecutorDataPtr || !*a2fExecutorDataPtr) { return MStatus::kFailure; }
    const auto& a2fExecutorData = **a2fExecutorDataPtr;

    auto& emotionAccumulator = *emotionAccumulatorData.mEmotionAccumulator;
    auto& faceExecutor = *a2fExecutorData.getFaceExecutor();

    nva2f::IGeometryInteractiveExecutor* geometryExecutor = nullptr;
    if (a2fExecutorData.hasBlendshapeExecutor()) {
        A2F_CHECK_WARNING_FAILURE(!nva2f::GetInteractiveExecutorGeometryExecutor(
            *a2fExecutorData.mBlendshapeInteractiveExecutor, &geometryExecutor), "Unable to get geometry executor");
    } else {
        geometryExecutor = a2fExecutorData.mGeometryInteractiveExecutor.get();
    }
    A2F_CHECK_WARNING_FAILURE(geometryExecutor, "Unable to get geometry executor. Null pointer.");

    if (nbFrames == 0) {
        // if nbFrames is 0, that means user didn't specify the number of frames, so we use the total number of frames
        nbFrames = a2fExecutorData.getTotalNbFrames();
    }
    A2F_CHECK_WARNING_FAILURE(nbFrames > 0, "totalNbFrames is 0");

    // Geometry results
    const auto geometryResultSize =
        geometryExecutor->GetSkinGeometrySize() + geometryExecutor->GetTongueGeometrySize() +
        geometryExecutor->GetJawTransformSize() + geometryExecutor->GetEyesRotationSize();
    const auto resultsBufferSize = nbFrames * geometryResultSize;
    auto resultsBuffer = ToUniquePtr(nva2x::CreateHostTensorFloat(resultsBufferSize));
    A2F_CHECK_WARNING_FAILURE(resultsBuffer, "Unable to allocate result buffer");

    // Move allocated buffers to the target data structure
    GeometryExecutorResultsData geometryExecutorResultsData {
        std::move(resultsBuffer),
        geometryExecutor->GetSkinGeometrySize(),
        geometryExecutor->GetTongueGeometrySize(),
        geometryExecutor->GetJawTransformSize(),
        geometryExecutor->GetEyesRotationSize()
    };

    ///
    // Blendshape Executor
    ///
    BlendshapeExecutorResultsData blendshapeExecutorResultsData;
    if (a2fExecutorData.hasBlendshapeExecutor()) {
        auto& blendshapeSolveExecutor = *a2fExecutorData.mBlendshapeInteractiveExecutor;

        // Blendshape results
        const auto blendshapeResultsSize = blendshapeSolveExecutor.GetWeightCount();
        const auto blendshapeResultsBufferSize = nbFrames * blendshapeResultsSize;
        auto blendshapeResultsBuffer = ToUniquePtr(blendshapeSolveExecutor.GetResultType() == nva2f::IBlendshapeInteractiveExecutor::ResultsType::DEVICE ?
            nva2x::CreateHostPinnedTensorFloat(blendshapeResultsBufferSize) : 
            nva2x::CreateHostTensorFloat(blendshapeResultsBufferSize));
        A2F_CHECK_WARNING_FAILURE(blendshapeResultsBuffer, "Unable to allocate blendshape results buffer");

        nva2f::BlendshapeSolverConfig faceSolverConfig;
        A2F_CHECK_WARNING_FAILURE(!nva2f::GetInteractiveExecutorBlendshapeSkinConfig(blendshapeSolveExecutor, faceSolverConfig), "Unable to get face solver config");
        nva2f::BlendshapeSolverConfig tongueSolverConfig;
        A2F_CHECK_WARNING_FAILURE(!nva2f::GetInteractiveExecutorBlendshapeTongueConfig(blendshapeSolveExecutor, tongueSolverConfig), "Unable to get tongue solver config");
        blendshapeExecutorResultsData = BlendshapeExecutorResultsData {
            std::move(blendshapeResultsBuffer),
            faceSolverConfig.multipliers.Size(),
            tongueSolverConfig.multipliers.Size()
        };
    }
    std::size_t frameRateNum, frameRateDenom;
    a2fExecutorData.getFrameRate(frameRateNum, frameRateDenom);
    ExecutorResultsData newExecutorResultsData {
        nbFrames,
        0,
        frameRateNum,
        frameRateDenom,
        std::move(geometryExecutorResultsData),
        std::move(blendshapeExecutorResultsData),
        false
    };

    executorResultsData = std::move(newExecutorResultsData);

    
    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::computeInitQuickEstimateBuffer(const MPlug& plug, MDataBlock& block) {
    // Wipe previous results so that it's empty in case of failure.
    MStatus status = SetPluginData<MayaExecutorResultsData>(
        block, MPlug(thisMObject(), quickEstimateBufferAttr), {}
    );
    CHECK_MSTATUS_AND_RETURN_IT(status);

    std::shared_ptr<ExecutorResultsData> executorResultsData = std::make_shared<ExecutorResultsData>();
    // Quick estimate frame is just 1 frame. So allocating 1 frame of buffer is enough.
    CHECK_MSTATUS_AND_RETURN_IT(allocateExecutorResultsData(block, *executorResultsData, /*nbFrames=*/1));
    executorResultsData->isQuickEstimate = true;

    status = SetPluginData<MayaExecutorResultsData>(
        block,
        MPlug(thisMObject(), quickEstimateBufferAttr),
        executorResultsData
        );
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::computeInitBackgroundEvaluationBuffer(const MPlug& plug, MDataBlock& block) {
    // Wipe previous results so that it's empty in case of failure.
    MStatus status = SetPluginData<MayaExecutorResultsData>(
        block, MPlug(thisMObject(), backgroundEvaluationBufferAttr), {}
    );
    CHECK_MSTATUS_AND_RETURN_IT(status);

    if (!block.context().isNormal()) {
        // skip in non-normal context
        return MStatus::kSuccess;
    }

    const auto backgroundEvaluationDataPtr = GetPluginData<MayaBackgroundEvaluationData>(block, backgroundEvaluationCtxAttr);
    if (!backgroundEvaluationDataPtr || !*backgroundEvaluationDataPtr) { return MStatus::kFailure; }
    auto& backgroundEvaluationData = **backgroundEvaluationDataPtr;

    const auto cudaStreamDataPtr = GetPluginData<MayaCudaStreamData>(block, cudaStreamAttr);
    if (!cudaStreamDataPtr || !*cudaStreamDataPtr) { return MStatus::kFailure; }
    const auto& cudaStreamData = **cudaStreamDataPtr;

    const auto accumulatedAudioAccumulatorDataPtr = GetPluginData<MayaAudioAccumulatorData>(block, accumulatedAudioAccumulatorAttr);
    if (!accumulatedAudioAccumulatorDataPtr || !*accumulatedAudioAccumulatorDataPtr) { return MStatus::kFailure; }
    const auto& accumulatedAudioAccumulatorData = **accumulatedAudioAccumulatorDataPtr;

    auto& audioAccumulator = *accumulatedAudioAccumulatorData.mAudioAccumulator;
    assert(audioAccumulator.IsClosed()); // audio accumulator should have been accumulated and closed.
    assert(backgroundEvaluationData.mBackgroundThreadRunning.load(std::memory_order_acquire) == false); // background thread should not be running

    std::shared_ptr<ExecutorResultsData> executorResultsData = std::make_shared<ExecutorResultsData>();
    CHECK_MSTATUS_AND_RETURN_IT(allocateExecutorResultsData(block, *executorResultsData));
    executorResultsData->isQuickEstimate = false;

    status = SetPluginData<MayaExecutorResultsData>(
        block,
        MPlug(thisMObject(), backgroundEvaluationBufferAttr),
        executorResultsData
    );
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MS::kSuccess;
}

bool A2FAnimationPlayer::isCacheStale(MDataBlock& block, MStatus* status) {
    *status = MStatus::kSuccess;
    if (!block.context().isNormal()) {
        return true; // always stale in non-normal context to return quick estimated frame
    }

    auto backgroundEvaluationDataPtr = GetPluginData<MayaBackgroundEvaluationData>(block, backgroundEvaluationCtxAttr);
    if (!backgroundEvaluationDataPtr || !*backgroundEvaluationDataPtr) { 
        if (status) { *status = MStatus::kFailure; }
        return false; // value doesn't matter, status is failure
    }
    auto& backgroundEvaluationData = **backgroundEvaluationDataPtr;

    auto& bgThreadRunning = backgroundEvaluationData.mBackgroundThreadRunning;
    auto& bgThreadFinished = backgroundEvaluationData.mBackgroundThreadFinished;

    if (!bgThreadFinished.load(std::memory_order_acquire) && !bgThreadRunning.load(std::memory_order_acquire)) {
        // background cache is not ready yet and no background thread is running
        return true;
    }

    auto& prevTime = backgroundEvaluationData.mPrevTime;
    auto& prevParameters = backgroundEvaluationData.mPrevParameters;
    auto& prevParametersSame = backgroundEvaluationData.mPrevParametersSame;

    const auto parameters = getParameters(block);
    if (parameters != prevParameters) {
        prevParameters = parameters;
        prevParametersSame = false;

        // parameter changed, means cache is stale
        return true;
    }

    const auto timeValue = block.inputValue(time).asTime();
    if (timeValue != prevTime) {
        prevTime = timeValue;
        // parameter is the same, but time changed, means cache is not stale
        return false;
    }
    
    // same time and parameters
    // we need to differentiate these cases
    // 1. same time is triggered twice (when scrub the timeline but not changing the time value)
        // in previous compute, the parameter was same
    // 2. same parameters are set twice (mouse up in attribute editor)
        // in previous compute, the parameter was not the same
    
    bool stale = !prevParametersSame;
    prevParametersSame = true;
    return stale;
}

MStatus A2FAnimationPlayer::computeInitExecutorResults(const MPlug& plug, MDataBlock& block) {
    // Wipe previous results so that it's empty in case of failure.
    MStatus status = SetPluginData<MayaExecutorResultsData>(
        block, MPlug(thisMObject(), executorResultsAttr), {}
        );
    CHECK_MSTATUS_AND_RETURN_IT(status);
    const maceProfilingScope scope("localAnimationPlayer_computeInitExecutorResults", thisMObject());

    std::shared_ptr<ExecutorResultsData> executorResultsData;
    const auto cudaStreamDataPtr = GetPluginData<MayaCudaStreamData>(block, cudaStreamAttr);
    if (!cudaStreamDataPtr || !*cudaStreamDataPtr) { return MStatus::kFailure; }
    const auto& cudaStreamData = **cudaStreamDataPtr;

    const auto a2eExecutorDataPtr = GetPluginData<MayaA2EExecutorData>(block, a2eExecutorAttr);
    if (!a2eExecutorDataPtr || !*a2eExecutorDataPtr) { return MStatus::kFailure; }
    const auto& a2eExecutorData = **a2eExecutorDataPtr;

    const auto a2fExecutorDataPtr = GetPluginData<MayaA2FExecutorData>(block, a2fExecutorAttr);
    if (!a2fExecutorDataPtr || !*a2fExecutorDataPtr) { return MStatus::kFailure; }
    const auto& a2fExecutorData = **a2fExecutorDataPtr;

    const auto emotionAccumulatorDataPtr = GetPluginData<MayaEmotionAccumulatorData>(block, emotionAccumulatorAttr);
    if (!emotionAccumulatorDataPtr || !*emotionAccumulatorDataPtr) { return MStatus::kFailure; }
    const auto& emotionAccumulatorData = **emotionAccumulatorDataPtr;

    const auto accumulatedAudioAccumulatorDataPtr = GetPluginData<MayaAudioAccumulatorData>(block, accumulatedAudioAccumulatorAttr);
    if (!accumulatedAudioAccumulatorDataPtr || !*accumulatedAudioAccumulatorDataPtr) { return MStatus::kFailure; }
    const auto& accumulatedAudioAccumulatorData = **accumulatedAudioAccumulatorDataPtr;

    const auto quickEstimateBufferPtr = GetPluginData<MayaExecutorResultsData>(block, quickEstimateBufferAttr);
    if (!quickEstimateBufferPtr || !*quickEstimateBufferPtr) { return MStatus::kFailure; }
    auto& quickEstimateBufferData = **quickEstimateBufferPtr;

    auto& cudaStream = *cudaStreamData.mCudaStream;
    auto& staticAudioAccumulator = *accumulatedAudioAccumulatorData.mAudioAccumulator;
    auto& emotionAccumulator = *emotionAccumulatorData.mEmotionAccumulator;
    auto& emotionExecutor = *a2eExecutorData.mEmotionInteractiveExecutor;
    auto& faceExecutor = *a2fExecutorData.getFaceExecutor();

    bool cacheStale = isCacheStale(block, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    if (cacheStale) {
        if (block.context().isNormal()) {
            const maceProfilingScope scope("localAnimationPlayer_computeInitExecutorResults_cancelBgThread", thisMObject());
            auto backgroundEvaluationDataPtr = GetPluginData<MayaBackgroundEvaluationData>(block, backgroundEvaluationCtxAttr);
            if (!backgroundEvaluationDataPtr || !*backgroundEvaluationDataPtr) { return MStatus::kFailure; }
            auto& backgroundEvaluationData = **backgroundEvaluationDataPtr;
            CHECK_MSTATUS_AND_RETURN_IT(cancelBackgroundEvaluation(backgroundEvaluationData, block));
        }

        // update emotion for all the frames. It's fast to compute when audio track is not changed.
        CHECK_MSTATUS_AND_RETURN_IT(updateA2EExecutorParameters(emotionExecutor, block));
        const bool emotionPostProcessingLayerValid = emotionExecutor.IsValid(nva2e::IEmotionInteractiveExecutor::kLayerPostProcessing);
        if (!emotionPostProcessingLayerValid || !emotionAccumulator.IsClosed()) {
            // emotion post processing parameters are changed, so face executor needs to be invalidated
            A2F_CHECK_WARNING_FAILURE(!emotionAccumulator.Reset(), "Unable to reset emotion accumulator");
            {
                const maceProfilingScope scope("localAnimationPlayer_quickEstimate_a2e", thisMObject());
                CHECK_MSTATUS_AND_RETURN_IT(runA2EExecutorOffline(
                    staticAudioAccumulator, 
                    emotionExecutor,
                    emotionAccumulator));
            }
            // Notify the face executor that the emotion accumulator is changed
            A2F_CHECK_WARNING_FAILURE(
                !faceExecutor.Invalidate(nva2f::IGeometryInteractiveExecutor::kLayerEmotionAccumulator), 
                "Unable to invalidate kLayerEmotionAccumulator in face executor"
            );
        }

        // do quick estimation
        CHECK_MSTATUS_AND_RETURN_IT(updateA2FExecutorParameters(faceExecutor, block));
        {
            const maceProfilingScope scope("localAnimationPlayer_quickEstimate_a2f", thisMObject());
            CHECK_MSTATUS_AND_RETURN_IT(runA2FExecutorOffline(
                cudaStream,
                staticAudioAccumulator,
                emotionAccumulator,
                faceExecutor,
                quickEstimateBufferData,
                block.inputValue(time).asTime().asUnits(MTime::kSeconds), true));
        }
        {
            const maceProfilingScope scope("localAnimationPlayer_quickEstimate_cudaSync", thisMObject());
            A2F_CHECK_WARNING_FAILURE(!cudaStream.Synchronize(), "Unable to synchronize cuda stream");
        }
        executorResultsData = *quickEstimateBufferPtr;

        if (block.context().isNormal()) {
            const maceProfilingScope scope("localAnimationPlayer_computeInitExecutorResults_scheduleBgThread", thisMObject());
            auto backgroundEvaluationBufferPtr = GetPluginData<MayaExecutorResultsData>(block, backgroundEvaluationBufferAttr);
            if (!backgroundEvaluationBufferPtr || !*backgroundEvaluationBufferPtr) { return MStatus::kFailure; }
            auto& backgroundEvaluationBuffer = **backgroundEvaluationBufferPtr;

            auto backgroundEvaluationDataPtr = GetPluginData<MayaBackgroundEvaluationData>(block, backgroundEvaluationCtxAttr);
            if (!backgroundEvaluationDataPtr || !*backgroundEvaluationDataPtr) { return MStatus::kFailure; }
            auto& backgroundEvaluationData = **backgroundEvaluationDataPtr;
            auto& bgThreadRunning = backgroundEvaluationData.mBackgroundThreadRunning;
            auto& bgThread = backgroundEvaluationData.mBackgroundThread;
            // Only start background evaluation thread in normal context

            if (!emotionPostProcessingLayerValid) {
                // Notify the face executor that the emotion accumulator is changed
                A2F_CHECK_WARNING_FAILURE(
                    !faceExecutor.Invalidate(nva2f::IGeometryInteractiveExecutor::kLayerEmotionAccumulator), 
                    "Unable to invalidate kLayerEmotionAccumulator in face executor"
                );
            }            

            // Start a new background evaluation thread
            bgThreadRunning.store(true, std::memory_order_release);

            bgThread = std::thread(
                &A2FAnimationPlayer::backgroundEvaluationThreadFunc, this,
                &cudaStream,
                &staticAudioAccumulator,
                &emotionAccumulator,
                &emotionExecutor,
                &faceExecutor,
                &backgroundEvaluationData,
                &backgroundEvaluationBuffer);
        }
    } else {
        if (block.context().isNormal()) {
            const maceProfilingScope scope("localAnimationPlayer_computeInitExecutorResults_waitForBgThread", thisMObject());
            auto backgroundEvaluationBufferPtr = GetPluginData<MayaExecutorResultsData>(block, backgroundEvaluationBufferAttr);
            if (!backgroundEvaluationBufferPtr || !*backgroundEvaluationBufferPtr) { return MStatus::kFailure; }

            auto backgroundEvaluationDataPtr = GetPluginData<MayaBackgroundEvaluationData>(block, backgroundEvaluationCtxAttr);
            if (!backgroundEvaluationDataPtr || !*backgroundEvaluationDataPtr) { return MStatus::kFailure; }
            auto& backgroundEvaluationData = **backgroundEvaluationDataPtr;
            auto& bgThread = backgroundEvaluationData.mBackgroundThread;
            auto& bgThreadFinished = backgroundEvaluationData.mBackgroundThreadFinished;
            // only time is changed and no parameters were changed
            // meaning we can simply wait for the background thread to finish.
            if (bgThread.joinable()) {
                bgThread.join();
            }

            assert(bgThreadFinished.load(std::memory_order_acquire));

            executorResultsData = *backgroundEvaluationBufferPtr;
        }
    }
    
    status = SetPluginData<MayaExecutorResultsData>(
        block,
        MPlug(thisMObject(), executorResultsAttr),
        executorResultsData
        );
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::computeGeometryResults(const MPlug& plug, MDataBlock& block) {
    const maceProfilingScope scope("localAnimationPlayer_computeGeometryResults", thisMObject());
    MStatus status;

    const auto executorResultsDataPtr = GetPluginData<MayaExecutorResultsData>(block, executorResultsAttr);
    if (!executorResultsDataPtr || !*executorResultsDataPtr) { return MStatus::kFailure; }
    const auto& executorResultsData = **executorResultsDataPtr;

    const auto a2fExecutorDataPtr = GetPluginData<MayaA2FExecutorData>(block, a2fExecutorAttr);
    if (!a2fExecutorDataPtr || !*a2fExecutorDataPtr) { return MStatus::kFailure; }
    const auto& a2fExecutorData = **a2fExecutorDataPtr;

    auto& resultsBuffer = executorResultsData.geometryExecutorResultsData.resultsBuffer;
    auto faceSize = executorResultsData.geometryExecutorResultsData.faceSize;
    auto tongueSize = executorResultsData.geometryExecutorResultsData.tongueSize;
    auto jawSize = executorResultsData.geometryExecutorResultsData.jawSize;
    auto eyesSize = executorResultsData.geometryExecutorResultsData.eyesSize;

    const auto frameIndex = std::min(
        static_cast<long>(executorResultsData.nbActualFrames) - 1, 
        computeFrameIndex(block, executorResultsData.frameRateNum, executorResultsData.frameRateDenom, 
            executorResultsData.nbFrames, a2fExecutorData.getSamplingRate(), &status));
    CHECK_MSTATUS_AND_RETURN_IT(status);
    assert(frameIndex >= 0 && frameIndex < executorResultsData.nbFrames);

    // set geometry outputs
    if (resultsBuffer->Size() % executorResultsData.nbFrames != 0) { return MStatus::kInvalidParameter; }
    const auto stride = resultsBuffer->Size() / executorResultsData.nbFrames;

    auto start = stride * frameIndex;
    size_t offset = start;
    auto&& face = resultsBuffer->View(offset, faceSize);
    offset += faceSize;
    auto&& tongue = resultsBuffer->View(offset, tongueSize);
    offset += tongueSize;
    auto&& jaw = resultsBuffer->View(offset, jawSize);
    offset += jawSize;
    auto&& eyes = resultsBuffer->View(offset, eyesSize);
    offset += eyesSize;

    MFnMeshData dataCreator;

    // Face geometry
    MPlug faceGeometryPlug(thisMObject(), faceGeometry);
    if (faceGeometryPlug.isConnected()) {
        const maceProfilingScope scope("computeGeometryResults_faceGeometry", thisMObject());
        MDataHandle faceGeometryOrigCacheHandle = block.inputValue(faceGeometryOrigCache);
        MObject faceGeometryOrigCacheMesh = faceGeometryOrigCacheHandle.asMesh();
        A2F_CHECK_WARNING_FAILURE(!faceGeometryOrigCacheMesh.isNull(), "no cached face mesh");

        MFnMesh fnOutputFaceGeometry(faceGeometryOrigCacheMesh);
        {
            const maceProfilingScope scope("computeGeometryResults_faceSetPoints", thisMObject());
            MFloatPointArray facePoints;
            facePoints.setLength(face.Size() / 3);
            for (size_t i = 0; i < face.Size(); i += 3) {
                facePoints.set(MFloatPoint(face.Data()[i], face.Data()[i + 1], face.Data()[i + 2]), i / 3);
            }
            fnOutputFaceGeometry.setPoints(facePoints);
        }
        MDataHandle faceGeometryHandle = block.outputValue(faceGeometry, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        CHECK_MSTATUS_AND_RETURN_IT(faceGeometryHandle.set(faceGeometryOrigCacheMesh));
        faceGeometryHandle.setClean();
    }

    // Tongue geometry
    MPlug tongueGeometryPlug(thisMObject(), tongueGeometry);
    if (tongueGeometryPlug.isConnected()) {
        const maceProfilingScope scope("computeGeometryResults_tongueGeometry", thisMObject());
        MDataHandle tongueGeometryOrigCacheHandle = block.inputValue(tongueGeometryOrigCache);
        MObject tongueGeometryOrigCacheMesh = tongueGeometryOrigCacheHandle.asMesh();
        A2F_CHECK_WARNING_FAILURE(!tongueGeometryOrigCacheMesh.isNull(), "no cached tongue mesh");

        MFnMesh fnOutputTongueGeometry(tongueGeometryOrigCacheMesh);
        {
            const maceProfilingScope scope("computeGeometryResults_tongueSetPoints", thisMObject());
            MFloatPointArray tonguePoints;
            tonguePoints.setLength(tongue.Size() / 3);
            for (size_t i = 0; i < tongue.Size(); i += 3) {
                tonguePoints.set(MFloatPoint(tongue.Data()[i], tongue.Data()[i + 1], tongue.Data()[i + 2]), i / 3);
            }
            fnOutputTongueGeometry.setPoints(tonguePoints);
        }
        MDataHandle tongueGeometryHandle = block.outputValue(tongueGeometry, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        CHECK_MSTATUS_AND_RETURN_IT(tongueGeometryHandle.set(tongueGeometryOrigCacheMesh));
        tongueGeometryHandle.setClean();
    }

    // Jaw transform
    MDataHandle jawTransformHandle = block.outputValue(jawTransform, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MFloatMatrix& jawTransformValue = jawTransformHandle.asFloatMatrix();
    jawTransformValue.setToIdentity();
    A2F_CHECK_WARNING_FAILURE(
        !nva2x::CopyHostToHost({jawTransformValue[0], 16}, jaw),
        "Unable to copy jaw transform"
    );
    jawTransformHandle.setClean();

    // Eye rotation
    float right_rotate[3] = { 0.0f, 0.0f, 0.0f };
    float left_rotate[3] = { 0.0f, 0.0f, 0.0f };
    A2F_CHECK_WARNING_FAILURE(
        !nva2x::CopyHostToHost({right_rotate, 3}, eyes.View(0, 3)),
        "Unable to copy right eye rotation"
    );
    A2F_CHECK_WARNING_FAILURE(
        !nva2x::CopyHostToHost({left_rotate, 3}, eyes.View(3, 3)),
        "Unable to copy left eye rotation"
    );
    auto deg_to_rad = [](float deg) {
        static constexpr double factor = 3.14159265358979323846 / 180.0;
        return factor * deg;
    };
    MDataHandle outputRightEyeRotationHandle = block.outputValue(rightEyeRotation);
    outputRightEyeRotationHandle.set(
        deg_to_rad(right_rotate[0]),
        deg_to_rad(right_rotate[1]),
        deg_to_rad(right_rotate[2])
    );
    outputRightEyeRotationHandle.setClean();

    MDataHandle outputLeftEyeRotationHandle = block.outputValue(leftEyeRotation);
    outputLeftEyeRotationHandle.set(
        deg_to_rad(left_rotate[0]),
        deg_to_rad(left_rotate[1]),
        deg_to_rad(left_rotate[2])
    );
    outputLeftEyeRotationHandle.setClean();

    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::computeBlendshapeSolveResults(const MPlug& plug, MDataBlock& block) {
    const maceProfilingScope scope("localAnimationPlayer_computeBlendshapeSolveResults", thisMObject());
    MStatus status;

    const auto executorResultsDataPtr = GetPluginData<MayaExecutorResultsData>(block, executorResultsAttr);
    if (!executorResultsDataPtr || !*executorResultsDataPtr) { return MStatus::kFailure; }
    const auto& executorResultsData = **executorResultsDataPtr;

    const auto a2fExecutorDataPtr = GetPluginData<MayaA2FExecutorData>(block, a2fExecutorAttr);
    if (!a2fExecutorDataPtr || !*a2fExecutorDataPtr) { return MStatus::kFailure; }
    const auto& a2fExecutorData = **a2fExecutorDataPtr;

    // return if blendshape executor is not available
    if (!a2fExecutorData.hasBlendshapeExecutor()) {
        CHECK_MSTATUS_AND_RETURN_IT(SetArrayOutput(block, outputWeights, std::vector<float>(), true));
        CHECK_MSTATUS_AND_RETURN_IT(SetOutput(block, this->faceWeightsCount, 0, true));
        CHECK_MSTATUS_AND_RETURN_IT(SetOutput(block, this->tongueWeightsCount, 0, true));
        return MS::kSuccess;
    }

    // else, when blendshape executor presents
    auto& resultsBuffer = executorResultsData.blendshapeExecutorResultsData.resultsBuffer;
    const auto faceWeightsCount = executorResultsData.blendshapeExecutorResultsData.faceWeightsCount;
    const auto tongueWeightsCount = executorResultsData.blendshapeExecutorResultsData.tongueWeightsCount;
    std::vector<float> faceMultiplierValues;
    std::vector<float> faceOffsetValues;
    std::vector<float> tongueMultiplierValues;
    std::vector<float> tongueOffsetValues;
    CHECK_MSTATUS_AND_RETURN_IT(GetArrayInput(block, blendshapeFaceMultipliers, faceMultiplierValues));
    CHECK_MSTATUS_AND_RETURN_IT(GetArrayInput(block, blendshapeFaceOffsets, faceOffsetValues));
    CHECK_MSTATUS_AND_RETURN_IT(GetArrayInput(block, blendshapeTongueMultipliers, tongueMultiplierValues));
    CHECK_MSTATUS_AND_RETURN_IT(GetArrayInput(block, blendshapeTongueOffsets, tongueOffsetValues));

    const auto frameIndex = std::min(
        static_cast<long>(executorResultsData.nbActualFrames) - 1, 
        computeFrameIndex(block, executorResultsData.frameRateNum, executorResultsData.frameRateDenom, 
            executorResultsData.nbFrames, a2fExecutorData.getSamplingRate(), &status));
    CHECK_MSTATUS_AND_RETURN_IT(status);
    assert(frameIndex >= 0 && frameIndex < executorResultsData.nbFrames);

    // set blendshape outputs
    if (resultsBuffer->Size() % executorResultsData.nbFrames != 0) { return MStatus::kInvalidParameter; }
    const auto stride = resultsBuffer->Size() / executorResultsData.nbFrames;

    auto start = stride * frameIndex;
    size_t offset = start;
    auto&& weights = resultsBuffer->View(offset, faceWeightsCount + tongueWeightsCount);

    std::vector<float> outputWeightsVector;
    outputWeightsVector.reserve(weights.Size());
    outputWeightsVector.insert(outputWeightsVector.end(), weights.Data(), weights.Data() + weights.Size());

    // Apply the multipliers and offsets from Maya
    auto applyMultipliersAndOffsets = [](float* weights, const std::vector<float>& multipliers, const std::vector<float>& offsets) {
        assert(multipliers.size() == offsets.size());
        for (size_t i = 0; i < multipliers.size(); ++i) {
            weights[i] = weights[i] * multipliers[i] + offsets[i];
        }
    };
    applyMultipliersAndOffsets(outputWeightsVector.data(), faceMultiplierValues, faceOffsetValues);
    applyMultipliersAndOffsets(outputWeightsVector.data() + faceWeightsCount, tongueMultiplierValues, tongueOffsetValues);

    CHECK_MSTATUS_AND_RETURN_IT(SetArrayOutput(block, outputWeights, outputWeightsVector, true));
    CHECK_MSTATUS_AND_RETURN_IT(SetOutput(block, this->faceWeightsCount, faceWeightsCount, true));
    CHECK_MSTATUS_AND_RETURN_IT(SetOutput(block, this->tongueWeightsCount, tongueWeightsCount, true));

    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::computeEmotionStates(const MPlug& plug, MDataBlock& block) {
    const maceProfilingScope scope("localAnimationPlayer_computeEmotionStates", thisMObject());
    MStatus status;

    const auto a2fExecutorDataPtr = GetPluginData<MayaA2FExecutorData>(block, a2fExecutorAttr);
    if (!a2fExecutorDataPtr || !*a2fExecutorDataPtr) { return MStatus::kFailure; }
    const auto& a2fExecutorData = **a2fExecutorDataPtr;

    const auto cudaStreamDataPtr = GetPluginData<MayaCudaStreamData>(block, cudaStreamAttr);
    if (!cudaStreamDataPtr || !*cudaStreamDataPtr) { return MStatus::kFailure; }
    const auto& cudaStreamData = **cudaStreamDataPtr;

    const auto emotionAccumulatorDataPtr = GetPluginData<MayaEmotionAccumulatorData>(block, emotionAccumulatorAttr);
    if (!emotionAccumulatorDataPtr || !*emotionAccumulatorDataPtr) { return MStatus::kFailure; }
    const auto& emotionAccumulatorData = **emotionAccumulatorDataPtr;

    auto& cudaStream = *cudaStreamData.mCudaStream;
    auto& emotionAccumulator = *emotionAccumulatorData.mEmotionAccumulator;
    auto& emotionDeviceTensor = *emotionAccumulatorData.mEmotionDeviceTensor;
    auto& emotionHostTensor = *emotionAccumulatorData.mEmotionHostTensor;

    assert(emotionAccumulator.IsClosed());

    const double sampleTime = block.inputValue(time).asTime().asUnits(MTime::kSeconds);
    nva2x::IEmotionAccumulator::timestamp_t timestamp = static_cast<nva2x::IEmotionAccumulator::timestamp_t>(sampleTime * a2fExecutorData.getSamplingRate());

    // sample from emotion accumulator
    A2F_CHECK_WARNING_FAILURE(!emotionAccumulator.Read(emotionDeviceTensor, timestamp, cudaStream.Data()), "Unable to read emotion accumulator");
    A2F_CHECK_WARNING_FAILURE(!nva2x::CopyDeviceToHost(emotionHostTensor, emotionDeviceTensor, cudaStream.Data()), "Unable to copy emotion to host");
 
    std::vector<float> emotionsVector(emotionHostTensor.Data(), emotionHostTensor.Data() + emotionHostTensor.Size());

    CHECK_MSTATUS_AND_RETURN_IT(SetArrayOutput(block, outputEmotions, emotionsVector, true));

    return MS::kSuccess;
}

MStatus A2FAnimationPlayer::computeIsQuickEstimate(const MPlug& plug, MDataBlock& block) {
    const maceProfilingScope scope("localAnimationPlayer_computeIsQuickEstimate", thisMObject());
    MStatus status;

    const auto executorResultsDataPtr = GetPluginData<MayaExecutorResultsData>(block, executorResultsAttr);
    if (!executorResultsDataPtr || !*executorResultsDataPtr) { return MStatus::kFailure; }
    const auto& executorResultsData = **executorResultsDataPtr;

    MDataHandle isQuickEstimateHandle = block.outputValue(isQuickEstimate, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    isQuickEstimateHandle.set(executorResultsData.isQuickEstimate);
    isQuickEstimateHandle.setClean();

    return MS::kSuccess;
}
