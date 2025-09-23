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
#include "ace_player.h"

#include <maya/MGlobal.h>
#include <maya/MTypeId.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnNumericData.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnCompoundAttribute.h>


MTypeId AceAnimationPlayer::id(0x9011f);
const char* AceAnimationPlayer::typeName = "AceAnimationPlayer";

MObject AceAnimationPlayer::time;

MObject AceAnimationPlayer::audioParams;
MObject AceAnimationPlayer::audiofile;
MObject AceAnimationPlayer::audioOffset;
MObject AceAnimationPlayer::audioStart;
MObject AceAnimationPlayer::audioEnd;

MObject AceAnimationPlayer::serviceParams;
MObject AceAnimationPlayer::networkAddress;
MObject AceAnimationPlayer::apiKey;
MObject AceAnimationPlayer::functionId;

MObject AceAnimationPlayer::faceParams;
MObject AceAnimationPlayer::lowerFaceSmoothing;
MObject AceAnimationPlayer::upperFaceSmoothing;
MObject AceAnimationPlayer::lowerFaceStrength;
MObject AceAnimationPlayer::upperFaceStrength;
MObject AceAnimationPlayer::faceMaskLevel;
MObject AceAnimationPlayer::faceMaskSoftness;
MObject AceAnimationPlayer::skinStrength;
MObject AceAnimationPlayer::eyelidOpenOffset;
MObject AceAnimationPlayer::lipOpenOffset;

// MObject AceAnimationPlayer::tongueParams;
// MObject AceAnimationPlayer::tongueStrength;
// MObject AceAnimationPlayer::tongueHeightOffset;
// MObject AceAnimationPlayer::tongueDepthOffset;

MObject AceAnimationPlayer::emotionParams;
MObject AceAnimationPlayer::emotionStrength;
MObject AceAnimationPlayer::emotionContrast;
MObject AceAnimationPlayer::maxEmotions;
MObject AceAnimationPlayer::liveBlendCoef;
MObject AceAnimationPlayer::enablePreferredEmotion;
MObject AceAnimationPlayer::preferredEmotionStrength;

MObject AceAnimationPlayer::preferredEmotions;

MObject AceAnimationPlayer::blendshapeParams;
MObject AceAnimationPlayer::blendshapeFaceMultipliers;
MObject AceAnimationPlayer::blendshapeFaceOffsets;
// MObject AceAnimationPlayer::blendshapeTongueMultipliers;
// MObject AceAnimationPlayer::blendshapeTongueOffsets;

// Outputs
MObject AceAnimationPlayer::outputAnimationResults;
MObject AceAnimationPlayer::outputWeights;
MObject AceAnimationPlayer::outputWeightNames;

MObject AceAnimationPlayer::outputEmotionStates;
MObject AceAnimationPlayer::outputEmotions;
MObject AceAnimationPlayer::outputEmotionNames;

MObject AceAnimationPlayer::status;
MObject AceAnimationPlayer::statusLoaded;
MObject AceAnimationPlayer::statusLoadedAudio;
MObject AceAnimationPlayer::statusAudioSamples;  // number of audio samples
MObject AceAnimationPlayer::statusReceived;
MObject AceAnimationPlayer::statusReceivedTime;
MObject AceAnimationPlayer::statusReceivedFrames;  // number of received animation frames
MObject AceAnimationPlayer::statusCurrentFrame;
MObject AceAnimationPlayer::statusNeedsUpdate;

MObject AceAnimationPlayer::triggerSendAudio;
MObject AceAnimationPlayer::triggerLoad;
MObject AceAnimationPlayer::triggerUpdateParameters;

MStatus AceAnimationPlayer::initialize() {
    MStatus return_status;
    MFnTypedAttribute t_attr;
    MFnUnitAttribute u_attr;
    MFnNumericAttribute n_attr;
    MFnCompoundAttribute c_attr;
    MFnStringData defaultStringData;

    // time
    time = u_attr.create("time", "tm", MFnUnitAttribute::kTime, 0.0, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    u_attr.setKeyable(true);
    u_attr.setReadable(true);
    u_attr.setWritable(true);
    addAttribute(time);

    // ace server configs
    networkAddress = t_attr.create("networkAddress", "na", MFnData::kString, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    t_attr.setStorable(true);
    t_attr.setKeyable(false);
    t_attr.setDefault(defaultStringData.create("https://grpc.nvcf.nvidia.com:443"));

    apiKey = t_attr.create("apiKey", "apiKey", MFnData::kString, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    t_attr.setStorable(true);
    t_attr.setKeyable(false);
    t_attr.setDefault(defaultStringData.create("$NVCF_API_KEY")); // read from env or empty

    functionId = t_attr.create("functionId", "functionId", MFnData::kString, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    t_attr.setStorable(true);
    t_attr.setKeyable(false);
    // v1.3 James - obtained from https://build.nvidia.com/nvidia/audio2face-3d/api
    t_attr.setDefault(defaultStringData.create("9327c39f-a361-4e02-bd72-e11b4c9b7b5e"));

    serviceParams = c_attr.create("serviceParameters", "sparams");
    c_attr.addChild(networkAddress);
    c_attr.addChild(apiKey);
    c_attr.addChild(functionId);
    addAttribute(serviceParams);

    ////////////////////////////////////////////////
    // Audio
    ////////////////////////////////////////////////
    audiofile = t_attr.create("audiofile", "af", MFnData::kString, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    t_attr.setStorable(true);
    t_attr.setKeyable(false);

    audioOffset = u_attr.create("audioOffset", "ao", MFnUnitAttribute::kTime, 0.0);
    u_attr.setKeyable(true);
    u_attr.setReadable(true);
    u_attr.setWritable(true);

    audioStart = u_attr.create("audioStart", "as", MFnUnitAttribute::kTime, 0.0);
    u_attr.setKeyable(true);
    u_attr.setReadable(true);
    u_attr.setWritable(true);

    audioEnd = u_attr.create("audioEnd", "ae", MFnUnitAttribute::kTime, -1.0);
    u_attr.setKeyable(true);
    u_attr.setReadable(true);
    u_attr.setWritable(true);

    audioParams = c_attr.create("audioParameters", "aparams");
    c_attr.addChild(audiofile);
    c_attr.addChild(audioOffset);
    c_attr.addChild(audioStart);
    c_attr.addChild(audioEnd);
    addAttribute(audioParams);

    ////////////////////////////////////////////////
    // Skin Parameters
    ////////////////////////////////////////////////
    lowerFaceSmoothing  = n_attr.create("lowerFaceSmoothing", "lfs", MFnNumericData::kFloat);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setSoftMax(0.1);
    n_attr.setDefault(0.0023);

    upperFaceSmoothing = n_attr.create("upperFaceSmoothing", "ufs", MFnNumericData::kFloat);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setSoftMax(0.1);
    n_attr.setDefault(0.001);

    lowerFaceStrength = n_attr.create("lowerFaceStrength", "lfst", MFnNumericData::kFloat);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setSoftMax(2.0);
    n_attr.setDefault(1.3);

    upperFaceStrength = n_attr.create("upperFaceStrength", "ufst", MFnNumericData::kFloat);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setSoftMax(2.0);
    n_attr.setDefault(1.0);

    faceMaskLevel = n_attr.create("faceMaskLevel", "fml", MFnNumericData::kFloat);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setSoftMax(1.0);
    n_attr.setDefault(0.6);

    faceMaskSoftness = n_attr.create("faceMaskSoftness", "fms", MFnNumericData::kFloat);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0.001);
    n_attr.setSoftMax(0.5);
    n_attr.setDefault(0.0085);

    skinStrength = n_attr.create("skinStrength", "skst", MFnNumericData::kFloat);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setSoftMax(2.0);
    n_attr.setDefault(1.0);

    eyelidOpenOffset = n_attr.create("eyelidOpenOffset", "eoo", MFnNumericData::kFloat);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(-1.0);
    n_attr.setSoftMax(1.0);
    n_attr.setDefault(0.06);

    lipOpenOffset = n_attr.create("lipOpenOffset", "loo", MFnNumericData::kFloat);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(-0.2);
    n_attr.setSoftMax(0.2);
    n_attr.setDefault(-0.03);

    faceParams = c_attr.create("faceParameters", "fparams");
    c_attr.addChild(lowerFaceSmoothing);
    c_attr.addChild(upperFaceSmoothing);
    c_attr.addChild(lowerFaceStrength);
    c_attr.addChild(upperFaceStrength);
    c_attr.addChild(faceMaskLevel);
    c_attr.addChild(faceMaskSoftness);
    c_attr.addChild(skinStrength);
    c_attr.addChild(eyelidOpenOffset);
    c_attr.addChild(lipOpenOffset);
    addAttribute(faceParams);

    ////////////////////////////////////////////////
    // Emotion Parameters
    ////////////////////////////////////////////////
    emotionStrength = n_attr.create("emotionStrength", "ems", MFnNumericData::kFloat);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0.0);
    n_attr.setSoftMax(1.0);
    n_attr.setDefault(0.6);

    emotionContrast = n_attr.create("emotionContrast", "emc", MFnNumericData::kFloat);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0.1);
    n_attr.setSoftMin(0.3);
    n_attr.setSoftMax(3.0);
    n_attr.setDefault(1.0);

    liveBlendCoef = n_attr.create("liveBlendCoef", "lbc", MFnNumericData::kFloat);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0.0);
    n_attr.setSoftMax(1.0);
    n_attr.setDefault(0.7);

    maxEmotions = n_attr.create("maxEmotions", "mxe", MFnNumericData::kInt);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(1);
    n_attr.setSoftMax(6);
    n_attr.setDefault(6);

    enablePreferredEmotion = n_attr.create(
        "enablePreferredEmotion", "epe", MFnNumericData::kBoolean);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setDefault(true);

    preferredEmotionStrength = n_attr.create(
        "preferredEmotionStrength", "pes", MFnNumericData::kFloat);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0.0);
    n_attr.setSoftMax(1.0);
    n_attr.setDefault(0.5);

    preferredEmotions = n_attr.create("preferredEmotions", "pem", MFnNumericData::kFloat);
    n_attr.setArray(true);
    n_attr.setUsesArrayDataBuilder(true);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setStorable(true);
    n_attr.setIndexMatters(true);
    n_attr.setKeyable(false);
    n_attr.setMin(0.0);
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
    addAttribute(emotionParams);

    // Blendshape Parameters
    blendshapeFaceMultipliers = n_attr.create("faceMultipliers", "bsfm", MFnNumericData::kFloat, 1.0);
    n_attr.setArray(true);
    n_attr.setUsesArrayDataBuilder(true);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setStorable(true);
    n_attr.setIndexMatters(true);
    n_attr.setKeyable(true);
    n_attr.setMin(0.0);
    n_attr.setSoftMax(2.0);

    blendshapeFaceOffsets = n_attr.create("faceOffsets", "bsfo", MFnNumericData::kFloat, 0.0);
    n_attr.setArray(true);
    n_attr.setUsesArrayDataBuilder(true);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setStorable(true);
    n_attr.setIndexMatters(true);
    n_attr.setKeyable(true);
    n_attr.setMin(0.0);
    n_attr.setSoftMax(2.0);

    blendshapeParams = c_attr.create("blendshapeParameters", "bparams");
    c_attr.addChild(blendshapeFaceMultipliers);
    c_attr.addChild(blendshapeFaceOffsets);
    addAttribute(blendshapeParams);

    ///
    // Output
    //
    outputWeights = n_attr.create("outputWeights", "ow", MFnNumericData::kFloat, 0.0);
    n_attr.setArray(true);
    n_attr.setUsesArrayDataBuilder(true);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(false);
    n_attr.setIndexMatters(true);

    outputAnimationResults = c_attr.create("animationResults", "oar");
    c_attr.addChild(outputWeights);
    addAttribute(outputAnimationResults);

    outputEmotions = n_attr.create("outputEmotions", "oe", MFnNumericData::kFloat, 0.0);
    n_attr.setArray(true);
    n_attr.setUsesArrayDataBuilder(true);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(false);
    n_attr.setIndexMatters(true);

    outputEmotionStates = c_attr.create("emotionStates", "oes");
    c_attr.addChild(outputEmotions);
    addAttribute(outputEmotionStates);

    ///
    // Hidden
    ///
    outputWeightNames = t_attr.create("outputWeightNames", "own", MFnData::kString);
    t_attr.setHidden(true);
    t_attr.setArray(true);
    t_attr.setUsesArrayDataBuilder(true);
    t_attr.setReadable(true);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setCached(false);
    n_attr.setKeyable(false);
    t_attr.setIndexMatters(true);
    addAttribute(outputWeightNames);

    outputEmotionNames = t_attr.create("outputEmotionNames", "oen", MFnData::kString);
    t_attr.setHidden(true);
    t_attr.setArray(true);
    t_attr.setUsesArrayDataBuilder(true);
    t_attr.setReadable(true);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setCached(false);
    n_attr.setKeyable(false);
    t_attr.setIndexMatters(true);
    addAttribute(outputEmotionNames);

    ///
    //Status
    //
    statusLoaded = n_attr.create("loaded", "sl", MFnNumericData::kBoolean, false);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);
    n_attr.setKeyable(false);

    statusLoadedAudio = t_attr.create("loadedAudio", "sla", MFnData::kString);
    t_attr.setReadable(true);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setCached(true);
    t_attr.setKeyable(false);

    statusReceived = n_attr.create("received", "sr", MFnNumericData::kBoolean, false);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);
    n_attr.setKeyable(false);

    statusAudioSamples  = n_attr.create("audioSamples", "sas", MFnNumericData::kLong, 0.0);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);
    n_attr.setKeyable(false);

    statusReceivedTime  = n_attr.create("receivedTime", "srt", MFnNumericData::kDouble, 0.0);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);
    n_attr.setKeyable(false);

    statusReceivedFrames  = n_attr.create("receivedFrames", "srf", MFnNumericData::kLong, 0.0);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);
    n_attr.setKeyable(false);

    statusCurrentFrame  = n_attr.create("currentFrame", "scf", MFnNumericData::kLong, 0.0);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);
    n_attr.setKeyable(false);

    statusNeedsUpdate = n_attr.create("needsUpdate", "snu", MFnNumericData::kBoolean, true);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);
    n_attr.setKeyable(false);

    status = c_attr.create("status", "status");
    c_attr.addChild(statusLoaded);
    c_attr.addChild(statusLoadedAudio);
    c_attr.addChild(statusAudioSamples);
    c_attr.addChild(statusReceived);
    c_attr.addChild(statusReceivedTime);
    c_attr.addChild(statusReceivedFrames);
    c_attr.addChild(statusCurrentFrame);
    c_attr.addChild(statusNeedsUpdate);
    c_attr.setHidden(true);
    addAttribute(status);

    ///
    // Controllers
    //
    triggerSendAudio = n_attr.create("triggerSendAudio", "tr", MFnNumericData::kDouble, 0);
    n_attr.setStorable(false);
    n_attr.setWritable(false);
    n_attr.setHidden(true);
    addAttribute(triggerSendAudio);

    triggerLoad = n_attr.create("triggerLoad", "tl", MFnNumericData::kDouble, 0);
    n_attr.setStorable(false);
    n_attr.setWritable(false);
    n_attr.setHidden(true);
    addAttribute(triggerLoad);

    triggerUpdateParameters = n_attr.create(
        "triggerUpdateParameters", "tup", MFnNumericData::kDouble, 0);
    n_attr.setStorable(false);
    n_attr.setWritable(false);
    n_attr.setHidden(true);
    addAttribute(triggerUpdateParameters);

    // service parameters
    setAffectsNeedsUpdate(AceAnimationPlayer::serviceParams);
    attributeAffects(AceAnimationPlayer::serviceParams, AceAnimationPlayer::status);

    // audio parameters
    setAffectsNeedsUpdate(AceAnimationPlayer::audioParams);
    attributeAffects(AceAnimationPlayer::audiofile, AceAnimationPlayer::triggerLoad);
    attributeAffects(AceAnimationPlayer::audiofile, AceAnimationPlayer::status);    

    // a2f parameters
    setAffectsNeedsUpdate(AceAnimationPlayer::faceParams);

    // a2e parameters
    setAffectsNeedsUpdate(AceAnimationPlayer::emotionParams);

    // preferred emotion
    setAffectsNeedsUpdate(AceAnimationPlayer::preferredEmotions);

    // blendshape parameters
    setAffectsNeedsUpdate(AceAnimationPlayer::blendshapeParams);

    // update frame output
    attributeAffects(AceAnimationPlayer::time, AceAnimationPlayer::outputWeights);
    attributeAffects(AceAnimationPlayer::time, AceAnimationPlayer::outputEmotions);
    attributeAffects(AceAnimationPlayer::time, AceAnimationPlayer::statusCurrentFrame);
    attributeAffects(AceAnimationPlayer::time, AceAnimationPlayer::statusReceived);

    // internal triggers
    attributeAffects(AceAnimationPlayer::triggerLoad, AceAnimationPlayer::triggerSendAudio);
    attributeAffects(AceAnimationPlayer::triggerLoad, AceAnimationPlayer::statusNeedsUpdate);
    attributeAffects(AceAnimationPlayer::triggerLoad, AceAnimationPlayer::status);

    attributeAffects(AceAnimationPlayer::triggerUpdateParameters, AceAnimationPlayer::statusNeedsUpdate);
    attributeAffects(AceAnimationPlayer::triggerUpdateParameters, AceAnimationPlayer::status);

    attributeAffects(AceAnimationPlayer::triggerSendAudio, AceAnimationPlayer::triggerUpdateParameters);
    attributeAffects(AceAnimationPlayer::triggerSendAudio, AceAnimationPlayer::outputWeightNames);
    attributeAffects(AceAnimationPlayer::triggerSendAudio, AceAnimationPlayer::outputEmotionNames);
    attributeAffects(AceAnimationPlayer::triggerSendAudio, AceAnimationPlayer::outputWeights);
    attributeAffects(AceAnimationPlayer::triggerSendAudio, AceAnimationPlayer::outputEmotions);
    attributeAffects(AceAnimationPlayer::triggerSendAudio, AceAnimationPlayer::statusNeedsUpdate);
    attributeAffects(AceAnimationPlayer::triggerSendAudio, AceAnimationPlayer::status);

    return MStatus::kSuccess;
}

void AceAnimationPlayer::setAffectsNeedsUpdate(MObject &source) {
    attributeAffects(source, AceAnimationPlayer::statusReceived);
    attributeAffects(source, AceAnimationPlayer::statusNeedsUpdate);
    attributeAffects(source, AceAnimationPlayer::triggerSendAudio);
}

void* AceAnimationPlayer::creator()
{
    return new AceAnimationPlayer();
}
