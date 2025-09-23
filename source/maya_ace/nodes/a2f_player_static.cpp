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

#include <maya/MGlobal.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMatrixAttribute.h>

MTypeId A2FAnimationPlayer::id(0x90120);
const char* A2FAnimationPlayer::typeName = "A2FAnimationPlayer";

MObject A2FAnimationPlayer::dummyInput;
MObject A2FAnimationPlayer::time;

MObject A2FAnimationPlayer::audioParams;
MObject A2FAnimationPlayer::audiofile;
MObject A2FAnimationPlayer::audioOffset;
MObject A2FAnimationPlayer::audioStart;
MObject A2FAnimationPlayer::audioEnd;

MObject A2FAnimationPlayer::modelConfigs;
MObject A2FAnimationPlayer::a2eModelPath;
MObject A2FAnimationPlayer::a2fModelPath;
MObject A2FAnimationPlayer::identityIndex;
MObject A2FAnimationPlayer::useGpuBlendshapeSolver;

MObject A2FAnimationPlayer::faceGeometryOrig;
MObject A2FAnimationPlayer::tongueGeometryOrig;

MObject A2FAnimationPlayer::emotionParams;
MObject A2FAnimationPlayer::emotionStrength;
MObject A2FAnimationPlayer::emotionContrast;
MObject A2FAnimationPlayer::maxEmotions;
MObject A2FAnimationPlayer::liveBlendCoef;
MObject A2FAnimationPlayer::enablePreferredEmotion;
MObject A2FAnimationPlayer::preferredEmotionStrength;
MObject A2FAnimationPlayer::preferredEmotions;

MObject A2FAnimationPlayer::faceParams;
MObject A2FAnimationPlayer::lowerFaceSmoothing;
MObject A2FAnimationPlayer::upperFaceSmoothing;
MObject A2FAnimationPlayer::lowerFaceStrength;
MObject A2FAnimationPlayer::upperFaceStrength;
MObject A2FAnimationPlayer::faceMaskLevel;
MObject A2FAnimationPlayer::faceMaskSoftness;
MObject A2FAnimationPlayer::skinStrength;
MObject A2FAnimationPlayer::eyelidOpenOffset;
MObject A2FAnimationPlayer::lipOpenOffset;

MObject A2FAnimationPlayer::tongueParams;
MObject A2FAnimationPlayer::tongueStrength;
MObject A2FAnimationPlayer::tongueHeightOffset;
MObject A2FAnimationPlayer::tongueDepthOffset;

MObject A2FAnimationPlayer::blendshapeParams;
MObject A2FAnimationPlayer::blendshapeFaceMultipliers;
MObject A2FAnimationPlayer::blendshapeFaceOffsets;
MObject A2FAnimationPlayer::blendshapeTongueMultipliers;
MObject A2FAnimationPlayer::blendshapeTongueOffsets;

MObject A2FAnimationPlayer::isQuickEstimate;

MObject A2FAnimationPlayer::outputEmotionStates;
MObject A2FAnimationPlayer::outputEmotions;

MObject A2FAnimationPlayer::outputAnimationResults;
MObject A2FAnimationPlayer::faceWeightsCount;
MObject A2FAnimationPlayer::tongueWeightsCount;
MObject A2FAnimationPlayer::outputWeights;

MObject A2FAnimationPlayer::outputInferenceResults;
MObject A2FAnimationPlayer::faceGeometry;
MObject A2FAnimationPlayer::tongueGeometry;
MObject A2FAnimationPlayer::jawTransform;
MObject A2FAnimationPlayer::rightEyeRotationX;
MObject A2FAnimationPlayer::rightEyeRotationY;
MObject A2FAnimationPlayer::rightEyeRotationZ;
MObject A2FAnimationPlayer::rightEyeRotation;
MObject A2FAnimationPlayer::leftEyeRotationX;
MObject A2FAnimationPlayer::leftEyeRotationY;
MObject A2FAnimationPlayer::leftEyeRotationZ;
MObject A2FAnimationPlayer::leftEyeRotation;

MObject A2FAnimationPlayer::faceGeometryOrigCache;
MObject A2FAnimationPlayer::tongueGeometryOrigCache;
MObject A2FAnimationPlayer::audioAccumulatorAttr;
MObject A2FAnimationPlayer::emotionAccumulatorAttr;
MObject A2FAnimationPlayer::cudaStreamAttr;
MObject A2FAnimationPlayer::a2eExecutorAttr;
MObject A2FAnimationPlayer::a2fExecutorAttr;
MObject A2FAnimationPlayer::accumulatedAudioAccumulatorAttr;
MObject A2FAnimationPlayer::executorResultsAttr;
MObject A2FAnimationPlayer::quickEstimateBufferAttr;
MObject A2FAnimationPlayer::backgroundEvaluationBufferAttr;
MObject A2FAnimationPlayer::backgroundEvaluationCtxAttr; 

void* A2FAnimationPlayer::creator() {
    return new A2FAnimationPlayer();
}

MStatus A2FAnimationPlayer::initialize() {
    MStatus status;
    MFnTypedAttribute t_attr;
    MFnUnitAttribute u_attr;
    MFnNumericAttribute n_attr;
    MFnCompoundAttribute c_attr;
    MFnMatrixAttribute m_attr;
    MFnStringData defaultStringData;

    //
    // Create input attributes
    //
    dummyInput = n_attr.create(
        "dummyInput", "di", MFnNumericData::kInt, 0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setStorable(false);
    n_attr.setReadable(false);
    n_attr.setWritable(false);
    n_attr.setHidden(true);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(dummyInput));

    time = u_attr.create(
        "time", "tm", MFnUnitAttribute::kTime, 0.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    u_attr.setKeyable(true);
    u_attr.setStorable(true);
    u_attr.setReadable(false);
    u_attr.setWritable(true);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(time));

    a2eModelPath = t_attr.create(
        "a2eModelPath", "a2emp", MFnData::kString, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setKeyable(false);
    t_attr.setStorable(true);
    t_attr.setReadable(false);
    t_attr.setWritable(true);
    t_attr.setDefault(defaultStringData.create("Local A2E model.json path"));
    // CHECK_MSTATUS_AND_RETURN_IT(addAttribute(a2eModelPath));

    a2fModelPath = t_attr.create(
        "a2fModelPath", "a2fmp", MFnData::kString, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setKeyable(false);
    t_attr.setStorable(true);
    t_attr.setReadable(false);
    t_attr.setWritable(true);
    t_attr.setDefault(defaultStringData.create("Local A2F model.json path"));
    // CHECK_MSTATUS_AND_RETURN_IT(addAttribute(a2fModelPath));

    identityIndex = n_attr.create(
        "identityIndex", "ii", MFnNumericData::kInt, 0.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setStorable(true);
    n_attr.setReadable(false);
    n_attr.setWritable(true);
    // CHECK_MSTATUS_AND_RETURN_IT(addAttribute(identityIndex));
    
    useGpuBlendshapeSolver = n_attr.create(
        "useGpuBlendshapeSolver", "gpubs", MFnNumericData::kBoolean, true, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setStorable(true);
    n_attr.setReadable(false);
    n_attr.setWritable(true);
    // CHECK_MSTATUS_AND_RETURN_IT(addAttribute(useGpuBlendshapeSolver));

    modelConfigs = c_attr.create("modelConfigs", "mc");
    c_attr.addChild(a2eModelPath);
    c_attr.addChild(a2fModelPath);
    c_attr.addChild(identityIndex);
    c_attr.addChild(useGpuBlendshapeSolver);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(modelConfigs));

    ////////////////////////////////////////////////
    // Audio
    ////////////////////////////////////////////////
    audiofile = t_attr.create(
        "audiofile", "af", MFnData::kString, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setStorable(true); // Attribute will be stored with the scene data
    t_attr.setKeyable(false); // Attribute will appear in the channel box when true

    audioOffset = u_attr.create(
        "audioOffset", "ao", MFnUnitAttribute::kTime, 0.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    u_attr.setKeyable(true);
    u_attr.setReadable(true);
    u_attr.setWritable(true);

    audioStart = u_attr.create(
        "audioStart", "as", MFnUnitAttribute::kTime, 0.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    u_attr.setKeyable(true);
    u_attr.setReadable(true);
    u_attr.setWritable(true);

    audioEnd = u_attr.create(
        "audioEnd", "ae", MFnUnitAttribute::kTime, -1.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    u_attr.setKeyable(true);
    u_attr.setReadable(true);
    u_attr.setWritable(true);

    audioParams = c_attr.create("audioParameters", "aparams");
    c_attr.addChild(audiofile);
    c_attr.addChild(audioOffset);
    c_attr.addChild(audioStart);
    c_attr.addChild(audioEnd);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(audioParams));

    ////////////////////////////////////////////////
    // Geometry In
    ////////////////////////////////////////////////
    faceGeometryOrig = t_attr.create(
        "faceGeometryOrig", "fgo", MFnData::kMesh, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setKeyable(true);
    t_attr.setStorable(false);
    t_attr.setReadable(false);
    t_attr.setWritable(true);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(faceGeometryOrig));

    tongueGeometryOrig = t_attr.create(
        "tongueGeometryOrig", "tgo", MFnData::kMesh, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setKeyable(true);
    t_attr.setStorable(false);
    t_attr.setReadable(false);
    t_attr.setWritable(true);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(tongueGeometryOrig));

    ////////////////////////////////////////////////
    // Emotion Parameters
    ////////////////////////////////////////////////
    emotionStrength = n_attr.create(
        "emotionStrength", "ems", MFnNumericData::kFloat, 0.6, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0.0);
    n_attr.setSoftMax(1.0);

    emotionContrast = n_attr.create(
        "emotionContrast", "emc", MFnNumericData::kFloat, 1.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0.1);
    n_attr.setSoftMin(0.3);
    n_attr.setSoftMax(3.0);

    liveBlendCoef = n_attr.create(
        "liveBlendCoef", "lbc", MFnNumericData::kFloat, 0.7, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0.0);
    n_attr.setSoftMax(1.0);


    maxEmotions = n_attr.create(
        "maxEmotions", "mxes", MFnNumericData::kInt, 6, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(1);
    n_attr.setSoftMax(6);

    enablePreferredEmotion = n_attr.create(
        "enablePreferredEmotion", "epe", MFnNumericData::kBoolean, true, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);

    preferredEmotionStrength = n_attr.create(
        "preferredEmotionStrength", "pes", MFnNumericData::kFloat, 0.5, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0.0);
    n_attr.setSoftMax(1.0);

    preferredEmotions = n_attr.create(
        "preferredEmotions", "pem", MFnNumericData::kFloat, 0.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setArray(true);
    n_attr.setUsesArrayDataBuilder(true);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setStorable(true);
    n_attr.setIndexMatters(true);
    n_attr.setKeyable(false);
    n_attr.setMin(0);
    n_attr.setSoftMax(1.0);
    n_attr.setDefault(0.0);

    emotionParams = c_attr.create("emotionParameters", "eparams");
    c_attr.addChild(emotionStrength);
    c_attr.addChild(emotionContrast);
    c_attr.addChild(liveBlendCoef);
    c_attr.addChild(maxEmotions);
    c_attr.addChild(enablePreferredEmotion);
    c_attr.addChild(preferredEmotionStrength);
    c_attr.addChild(preferredEmotions);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(emotionParams));

    ////////////////////////////////////////////////
    // Face Parameters
    ////////////////////////////////////////////////
    lowerFaceSmoothing  = n_attr.create(
        "lowerFaceSmoothing", "lfs", MFnNumericData::kFloat, 0.0023, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setMax(0.1);

    upperFaceSmoothing = n_attr.create(
        "upperFaceSmoothing", "ufs", MFnNumericData::kFloat, 0.001, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setMax(0.1);

    lowerFaceStrength = n_attr.create(
        "lowerFaceStrength", "lfst", MFnNumericData::kFloat, 1.3, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setMax(2.0);

    upperFaceStrength = n_attr.create(
        "upperFaceStrength", "ufst", MFnNumericData::kFloat, 1.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setMax(2.0);

    faceMaskLevel = n_attr.create(
        "faceMaskLevel", "fml", MFnNumericData::kFloat, 0.6, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setMax(1.0);

    faceMaskSoftness = n_attr.create(
        "faceMaskSoftness", "fms", MFnNumericData::kFloat, 0.0085, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0.001);
    n_attr.setMax(0.5);

    skinStrength = n_attr.create(
        "skinStrength", "skst", MFnNumericData::kFloat, 1.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setMax(2.0);

    eyelidOpenOffset = n_attr.create(
        "eyelidOpenOffset", "eoo", MFnNumericData::kFloat, 0.06, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(-1.0);
    n_attr.setMax(1.0);

    lipOpenOffset = n_attr.create(
        "lipOpenOffset", "loo", MFnNumericData::kFloat, -0.03, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(-0.2);
    n_attr.setMax(0.2);

    faceParams = c_attr.create("faceParameters", "fparams", &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    c_attr.addChild(lowerFaceSmoothing);
    c_attr.addChild(upperFaceSmoothing);
    c_attr.addChild(lowerFaceStrength);
    c_attr.addChild(upperFaceStrength);
    c_attr.addChild(faceMaskLevel);
    c_attr.addChild(faceMaskSoftness);
    c_attr.addChild(skinStrength);
    c_attr.addChild(eyelidOpenOffset);
    c_attr.addChild(lipOpenOffset);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(faceParams));

    ////////////////////////////////////////////////
    // Tongue Parameters
    ////////////////////////////////////////////////
    tongueStrength = n_attr.create(
        "tongueStrength", "tgs", MFnNumericData::kFloat, 1.5, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setMax(3.0);

    tongueHeightOffset = n_attr.create(
        "tongueHeightOffset", "tho", MFnNumericData::kFloat, 0.2, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(-3.0);
    n_attr.setMax(3.0);

    tongueDepthOffset = n_attr.create(
        "tongueDepthOffset", "tdo", MFnNumericData::kFloat, 0.13, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(-3.0);
    n_attr.setMax(3.0);

    tongueParams = c_attr.create("tongueParameters", "tparams", &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    c_attr.addChild(tongueStrength);
    c_attr.addChild(tongueHeightOffset);
    c_attr.addChild(tongueDepthOffset);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(tongueParams));

    ////////////////////////////////////////////////
    // Blendshape Parameters
    ////////////////////////////////////////////////
    blendshapeFaceMultipliers = n_attr.create(
        "faceMultipliers", "bsfm", MFnNumericData::kFloat, 1.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setArray(true);
    n_attr.setUsesArrayDataBuilder(true);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setStorable(true);
    n_attr.setIndexMatters(true);
    n_attr.setKeyable(true);
    n_attr.setMin(0);
    n_attr.setSoftMax(2.0);

    blendshapeFaceOffsets = n_attr.create(
        "faceOffsets", "bsfo", MFnNumericData::kFloat, 0.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setArray(true);
    n_attr.setUsesArrayDataBuilder(true);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setStorable(true);
    n_attr.setIndexMatters(true);
    n_attr.setKeyable(true);
    n_attr.setMin(0);
    n_attr.setSoftMax(2.0);

    blendshapeTongueMultipliers = n_attr.create(
        "tongueMultipliers", "bstm", MFnNumericData::kFloat, 1.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setArray(true);
    n_attr.setUsesArrayDataBuilder(true);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setStorable(true);
    n_attr.setIndexMatters(true);
    n_attr.setKeyable(true);
    n_attr.setMin(0);
    n_attr.setSoftMax(2.0);

    blendshapeTongueOffsets = n_attr.create(
        "tongueOffsets", "bsto", MFnNumericData::kFloat, 0.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setArray(true);
    n_attr.setUsesArrayDataBuilder(true);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setStorable(true);
    n_attr.setIndexMatters(true);
    n_attr.setKeyable(true);
    n_attr.setMin(0);
    n_attr.setSoftMax(2.0);

    blendshapeParams = c_attr.create("blendshapeParameters", "bparams", &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    c_attr.addChild(blendshapeFaceMultipliers);
    c_attr.addChild(blendshapeFaceOffsets);
    c_attr.addChild(blendshapeTongueMultipliers);
    c_attr.addChild(blendshapeTongueOffsets);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(blendshapeParams));

    //
    // Create output attributes
    //

    isQuickEstimate = n_attr.create(
        "isQuickEstimate", "iqe", MFnNumericData::kBoolean, false, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setStorable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setHidden(true);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(isQuickEstimate));

    outputEmotions = n_attr.create(
        "outputEmotions", "oe", MFnNumericData::kFloat, 0.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setArray(true);
    n_attr.setUsesArrayDataBuilder(true);
    n_attr.setKeyable(false);
    n_attr.setStorable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(false);

    outputEmotionStates = c_attr.create("emotionStates", "oes", &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    c_attr.addChild(outputEmotions);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(outputEmotionStates));

    faceGeometry = t_attr.create(
        "faceGeometry", "sg", MFnData::kMesh, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setKeyable(false);
    t_attr.setStorable(false);
    t_attr.setReadable(true);
    t_attr.setWritable(false);

    tongueGeometry = t_attr.create(
        "tongueGeometry", "tg", MFnData::kMesh, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setKeyable(false);
    t_attr.setStorable(false);
    t_attr.setReadable(true);
    t_attr.setWritable(false);

    jawTransform = m_attr.create(
        "jawTransform", "jt", MFnMatrixAttribute::kFloat, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    m_attr.setKeyable(false);
    m_attr.setStorable(false);
    m_attr.setReadable(true);
    m_attr.setWritable(false);

    rightEyeRotationX = u_attr.create(
        "rightEyeRotationX", "rrx", MFnUnitAttribute::kAngle, 0.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    rightEyeRotationY = u_attr.create(
        "rightEyeRotationY", "rry", MFnUnitAttribute::kAngle, 0.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    rightEyeRotationZ = u_attr.create(
        "rightEyeRotationZ", "rrz", MFnUnitAttribute::kAngle, 0.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    rightEyeRotation = n_attr.create(
        "rightEyeRotation", "rr", rightEyeRotationX, rightEyeRotationY, rightEyeRotationZ, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setStorable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    
    leftEyeRotationX = u_attr.create(
        "leftEyeRotationX", "lrx", MFnUnitAttribute::kAngle, 0.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    leftEyeRotationY = u_attr.create(
        "leftEyeRotationY", "lry", MFnUnitAttribute::kAngle, 0.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    leftEyeRotationZ = u_attr.create(
        "leftEyeRotationZ", "lrz", MFnUnitAttribute::kAngle, 0.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    leftEyeRotation = n_attr.create(
        "leftEyeRotation", "lr", leftEyeRotationX, leftEyeRotationY, leftEyeRotationZ, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setStorable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(false);

    outputInferenceResults = c_attr.create("inferenceResults", "oir", &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    CHECK_MSTATUS_AND_RETURN_IT(c_attr.addChild(faceGeometry));
    CHECK_MSTATUS_AND_RETURN_IT(c_attr.addChild(tongueGeometry));
    CHECK_MSTATUS_AND_RETURN_IT(c_attr.addChild(jawTransform));
    CHECK_MSTATUS_AND_RETURN_IT(c_attr.addChild(rightEyeRotation));
    CHECK_MSTATUS_AND_RETURN_IT(c_attr.addChild(leftEyeRotation));
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(outputInferenceResults));

    faceWeightsCount = n_attr.create(
        "faceWeightsCount", "fwc", MFnNumericData::kInt, 0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setStorable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(false);

    tongueWeightsCount = n_attr.create(
        "tongueWeightsCount", "twc", MFnNumericData::kInt, 0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setKeyable(false);
    n_attr.setStorable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(false);

    outputWeights = n_attr.create(
        "outputWeights", "ow", MFnNumericData::kFloat, 0.0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    n_attr.setArray(true);
    n_attr.setUsesArrayDataBuilder(true);
    n_attr.setKeyable(false);
    n_attr.setStorable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(false);

    outputAnimationResults = c_attr.create("animationResults", "oar", &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    c_attr.addChild(outputWeights);
    c_attr.addChild(faceWeightsCount);
    c_attr.addChild(tongueWeightsCount);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(outputAnimationResults));

    //
    // Create internal attributes
    //

    faceGeometryOrigCache = t_attr.create(
        "faceGeometryOrigCache", "fgoc", MFnData::kMesh, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setKeyable(false);
    t_attr.setReadable(false);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setConnectable(false);
    t_attr.setHidden(true);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(faceGeometryOrigCache));

    tongueGeometryOrigCache = t_attr.create(
        "tongueGeometryOrigCache", "tgoc", MFnData::kMesh, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setKeyable(false);
    t_attr.setReadable(false);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setConnectable(false);
    t_attr.setHidden(true);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(tongueGeometryOrigCache));

    audioAccumulatorAttr = t_attr.create(
        "audioAccumulator", "aacc", MayaAudioAccumulatorData::Id(), MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setKeyable(false);
    t_attr.setReadable(false);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setConnectable(false);
    t_attr.setHidden(true);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(audioAccumulatorAttr));

    emotionAccumulatorAttr = t_attr.create(
        "emotionAccumulator", "eacc", MayaEmotionAccumulatorData::Id(), MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setKeyable(false);
    t_attr.setReadable(false);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setConnectable(false);
    t_attr.setHidden(true);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(emotionAccumulatorAttr));

    cudaStreamAttr = t_attr.create(
        "cudaStream", "cudast", MayaCudaStreamData::Id(), MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setKeyable(false);
    t_attr.setReadable(false);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setConnectable(false);
    t_attr.setHidden(true);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(cudaStreamAttr));

    a2eExecutorAttr = t_attr.create(
        "a2eExecutor", "a2eexe", MayaA2EExecutorData::Id(), MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setKeyable(false);
    t_attr.setReadable(false);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setConnectable(false);
    t_attr.setHidden(true);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(a2eExecutorAttr));

    a2fExecutorAttr = t_attr.create(
        "a2fExecutor", "a2fexe", MayaA2FExecutorData::Id(), MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setKeyable(false);
    t_attr.setReadable(false);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setConnectable(false);
    t_attr.setHidden(true);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(a2fExecutorAttr));

    accumulatedAudioAccumulatorAttr = t_attr.create(
        "accumulatedAudioAccumulator", "aaacc", MayaAudioAccumulatorData::Id(), MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setKeyable(false);
    t_attr.setReadable(false);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setConnectable(false);
    t_attr.setHidden(true);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(accumulatedAudioAccumulatorAttr));

    executorResultsAttr = t_attr.create(
        "executorResults", "exres", MayaExecutorResultsData::Id(), MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setKeyable(false);
    t_attr.setReadable(false);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setConnectable(false);
    t_attr.setHidden(true);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(executorResultsAttr));

    quickEstimateBufferAttr = t_attr.create(
        "quickEstimateBuffer", "qeb", MayaExecutorResultsData::Id(), MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setKeyable(false);
    t_attr.setReadable(false);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setConnectable(false);
    t_attr.setHidden(true);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(quickEstimateBufferAttr));

    backgroundEvaluationBufferAttr = t_attr.create(
        "backgroundEvaluationBuffer", "bgevbuf", MayaBackgroundEvaluationData::Id(), MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setKeyable(false);
    t_attr.setReadable(false);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setConnectable(false);
    t_attr.setHidden(true);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(backgroundEvaluationBufferAttr));

    backgroundEvaluationCtxAttr = t_attr.create(
        "backgroundEvaluationCtx", "bgevctx", MayaBackgroundEvaluationData::Id(), MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    t_attr.setKeyable(false);
    t_attr.setReadable(false);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setConnectable(false);
    t_attr.setHidden(true);
    CHECK_MSTATUS_AND_RETURN_IT(addAttribute(backgroundEvaluationCtxAttr));

    //
    // Setup attribute dependencies (transitive dependencies have to be explicit)
    //

    // These attributes doesn't depend on any input attributes,
    // setting the dependencies to dummyInput to trigger evaluation
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(dummyInput, audioAccumulatorAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(dummyInput, backgroundEvaluationCtxAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(dummyInput, cudaStreamAttr));

    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2eModelPath, a2eExecutorAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2eModelPath, quickEstimateBufferAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2eModelPath, backgroundEvaluationBufferAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2eModelPath, executorResultsAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2eModelPath, outputInferenceResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2eModelPath, outputAnimationResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2eModelPath, outputEmotionStates));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2eModelPath, isQuickEstimate));

    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2fModelPath, emotionAccumulatorAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2fModelPath, a2eExecutorAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2fModelPath, a2fExecutorAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2fModelPath, quickEstimateBufferAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2fModelPath, backgroundEvaluationBufferAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2fModelPath, executorResultsAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2fModelPath, outputInferenceResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2fModelPath, outputAnimationResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2fModelPath, outputEmotionStates));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2fModelPath, isQuickEstimate));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(identityIndex, a2fExecutorAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(identityIndex, executorResultsAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(identityIndex, outputInferenceResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(identityIndex, outputAnimationResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(identityIndex, outputEmotionStates));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(identityIndex, isQuickEstimate));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(useGpuBlendshapeSolver, a2fExecutorAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(useGpuBlendshapeSolver, executorResultsAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(useGpuBlendshapeSolver, outputInferenceResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(useGpuBlendshapeSolver, outputAnimationResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(useGpuBlendshapeSolver, outputEmotionStates));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(useGpuBlendshapeSolver, isQuickEstimate));

    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(audioParams, accumulatedAudioAccumulatorAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(audioParams, backgroundEvaluationBufferAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(audioParams, executorResultsAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(audioParams, outputInferenceResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(audioParams, outputAnimationResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(audioParams, outputEmotionStates));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(audioParams, isQuickEstimate));

    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(emotionParams, executorResultsAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(emotionParams, outputInferenceResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(emotionParams, outputAnimationResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(emotionParams, outputEmotionStates));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(emotionParams, isQuickEstimate));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(faceParams, executorResultsAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(faceParams, outputInferenceResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(faceParams, outputAnimationResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(faceParams, outputEmotionStates));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(faceParams, isQuickEstimate));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(tongueParams, executorResultsAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(tongueParams, outputInferenceResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(tongueParams, outputAnimationResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(tongueParams, outputEmotionStates));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(tongueParams, isQuickEstimate));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(blendshapeParams, outputAnimationResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(blendshapeFaceMultipliers, outputAnimationResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(blendshapeFaceOffsets, outputAnimationResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(blendshapeTongueMultipliers, outputAnimationResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(blendshapeTongueOffsets, outputAnimationResults));

    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(accumulatedAudioAccumulatorAttr, executorResultsAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2eExecutorAttr, executorResultsAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(a2fExecutorAttr, executorResultsAttr));

    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(faceGeometryOrig, faceGeometry));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(faceGeometryOrig, faceGeometryOrigCache));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(faceGeometryOrigCache, faceGeometry));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(tongueGeometryOrig, tongueGeometry));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(tongueGeometryOrig, tongueGeometryOrigCache));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(tongueGeometryOrigCache, tongueGeometry));
    
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(time, executorResultsAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(time, outputInferenceResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(time, outputAnimationResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(time, outputEmotionStates));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(time, isQuickEstimate));

    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(executorResultsAttr, outputInferenceResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(executorResultsAttr, outputAnimationResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(executorResultsAttr, outputEmotionStates));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(executorResultsAttr, isQuickEstimate));

    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(quickEstimateBufferAttr, executorResultsAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(quickEstimateBufferAttr, outputInferenceResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(quickEstimateBufferAttr, outputAnimationResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(quickEstimateBufferAttr, outputEmotionStates));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(quickEstimateBufferAttr, isQuickEstimate));

    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(backgroundEvaluationBufferAttr, executorResultsAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(backgroundEvaluationBufferAttr, outputInferenceResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(backgroundEvaluationBufferAttr, outputAnimationResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(backgroundEvaluationBufferAttr, outputEmotionStates));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(backgroundEvaluationBufferAttr, isQuickEstimate));

    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(backgroundEvaluationCtxAttr, executorResultsAttr));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(backgroundEvaluationCtxAttr, outputInferenceResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(backgroundEvaluationCtxAttr, outputAnimationResults));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(backgroundEvaluationCtxAttr, outputEmotionStates));
    CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(backgroundEvaluationCtxAttr, isQuickEstimate));

    return MS::kSuccess;
}
