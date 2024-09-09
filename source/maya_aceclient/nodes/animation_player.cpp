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
#include "animation_player.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>

#include <maya/MArrayDataBuilder.h>
#include <maya/MComputation.h>

#include "aceclient/aceclient.h"
#include "aceclient/audio.h"
#include "aceclient/animation.h"
#include "aceclient/frame_receiver.h"
#include "aceclient/logger.h"

#include "common/names.h"

MTypeId AceAnimationPlayer::id(0x9011f);
const char* AceAnimationPlayer::typeName = "AceAnimationPlayer";

MObject AceAnimationPlayer::time;

MObject AceAnimationPlayer::audio;
MObject AceAnimationPlayer::audiofile;
MObject AceAnimationPlayer::audioOffset;
MObject AceAnimationPlayer::audioStart;
MObject AceAnimationPlayer::audioEnd;

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

MObject AceAnimationPlayer::emotionParams;
MObject AceAnimationPlayer::emotionStrength;
MObject AceAnimationPlayer::emotionContrast;
MObject AceAnimationPlayer::maxEmotion;
MObject AceAnimationPlayer::liveBlendCoef;
MObject AceAnimationPlayer::enablePreferredEmotion;
MObject AceAnimationPlayer::preferredEmotionStrength;

MObject AceAnimationPlayer::emotions;
MObject AceAnimationPlayer::emotionList[EMOTION_COUNT];

MObject AceAnimationPlayer::blendshapeParams;
MObject AceAnimationPlayer::blendshapeMultipliers[BLENDSHAPE_COUNT];
MObject AceAnimationPlayer::blendshapeOffsets[BLENDSHAPE_COUNT];
// MObject AceAnimationPlayer::blendshapeLabels;

MObject AceAnimationPlayer::output;
MObject AceAnimationPlayer::outputWeights;
MObject AceAnimationPlayer::outputBlendshapeNames;
MObject AceAnimationPlayer::outputEmotionState;
MObject AceAnimationPlayer::outputEmotionStateNames;

MObject AceAnimationPlayer::status;
MObject AceAnimationPlayer::statusLoaded;
MObject AceAnimationPlayer::statusLoadedAudio;
MObject AceAnimationPlayer::statusAudioSamples;  // number of audio samples
MObject AceAnimationPlayer::statusReceived;
MObject AceAnimationPlayer::statusReceivedTime;
MObject AceAnimationPlayer::statusReceivedFrames;  // number of received animation frames
MObject AceAnimationPlayer::statusCurrentFrame;

MObject AceAnimationPlayer::triggerRequest;
MObject AceAnimationPlayer::triggerLoad;

const std::unordered_map<AceClientStatus, MString> ACECLIENT_ERROR_MESSAGE_MAP = {
    {AceClientStatus::ERROR_UNAUTHENTICATED, "Invalid or empty API key provided. Please check and try again."},
    {AceClientStatus::ERROR_SSL_HANDSHAKE, "The remote server does not support SSL. Try using HTTP instead of HTTPS."},
    {AceClientStatus::ERROR_CONNECTION, "The remote server is unreachable or is using HTTPS when HTTP is expected."},
    {AceClientStatus::ERROR_DNS_RESOLUTION, "DNS resolution failed. Please ensure that the server URL is correct."},
    {AceClientStatus::ERROR_CREDITS_EXPIRED, "Your API key has run out of cloud credits; Please get in touch with NVIDIA representatives for assistance."}
};

AceAnimationPlayer::AceAnimationPlayer(){}
AceAnimationPlayer::~AceAnimationPlayer(){}

void* AceAnimationPlayer::creator()
{
    return new AceAnimationPlayer();
}

MStatus AceAnimationPlayer::initialize()
{
    MStatus return_status;
    MFnTypedAttribute t_attr;
    MFnUnitAttribute u_attr;
    MFnNumericAttribute n_attr;
    MFnCompoundAttribute c_attr;
    MFnStringData defaultStringData;

    // ace server configs
    networkAddress = t_attr.create("networkAddress", "na", MFnData::kString, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    t_attr.setStorable(true); // Attribute will be stored with the scene data
    t_attr.setKeyable(false); // Attribute will appear in the channel box
    t_attr.setDefault(defaultStringData.create("https://grpc.nvcf.nvidia.com:443"));
    addAttribute(networkAddress);

    apiKey = t_attr.create("apiKey", "apiKey", MFnData::kString, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    t_attr.setStorable(true); // Attribute will be stored with the scene data
    t_attr.setKeyable(false); // Attribute will appear in the channel box
    t_attr.setDefault(defaultStringData.create("$NVCF_API_KEY")); // read from env or empty
    addAttribute(apiKey);

    functionId = t_attr.create("functionId", "functionId", MFnData::kString, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    t_attr.setStorable(true); // Attribute will be stored with the scene data
    t_attr.setKeyable(false); // Attribute will appear in the channel box
    // obtained from https://build.nvidia.com/nvidia/audio2face/api
    t_attr.setDefault(defaultStringData.create("462f7853-60e8-474a-9728-7b598e58472c"));
    addAttribute(functionId);

    // time
    time = u_attr.create("time", "tm", MFnUnitAttribute::kTime, 0.0, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    u_attr.setKeyable(true);
    u_attr.setReadable(true);
    u_attr.setWritable(true);
    addAttribute(time);

    ////////////////////////////////////////////////
    // Audio
    ////////////////////////////////////////////////
    audiofile = t_attr.create("audiofile", "af", MFnData::kString, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    t_attr.setStorable(true); // Attribute will be stored with the scene data
    t_attr.setKeyable(false); // Attribute will appear in the channel box

    audioOffset = u_attr.create("audioOffset", "ao", MFnUnitAttribute::kTime, 0.0, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    u_attr.setKeyable(true);
    u_attr.setReadable(true);
    u_attr.setWritable(true);

    audioStart = u_attr.create("audioStart", "as", MFnUnitAttribute::kTime, 0.0, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    u_attr.setKeyable(true);
    u_attr.setReadable(true);
    u_attr.setWritable(true);

    audioEnd = u_attr.create("audioEnd", "ae", MFnUnitAttribute::kTime, -1.0, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    u_attr.setKeyable(true);
    u_attr.setReadable(true);
    u_attr.setWritable(true);

    audio = c_attr.create("audio", "aud");
    c_attr.addChild(audiofile);
    c_attr.addChild(audioOffset);
    c_attr.addChild(audioStart);
    c_attr.addChild(audioEnd);
    addAttribute(audio);

    ////////////////////////////////////////////////
    // Skin Parameters
    ////////////////////////////////////////////////
    lowerFaceSmoothing  = n_attr.create("lowerFaceSmoothing", "lfs", MFnNumericData::kFloat, 0.0);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setSoftMax(0.1);
    n_attr.setDefault(0.0023);

    upperFaceSmoothing = n_attr.create("upperFaceSmoothing", "ufs", MFnNumericData::kFloat, 0.0);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setSoftMax(0.1);
    n_attr.setDefault(0.001);

    lowerFaceStrength = n_attr.create("lowerFaceStrength", "lfst", MFnNumericData::kFloat, 0.0);
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setSoftMax(2.0);
    n_attr.setDefault(1.3);

    upperFaceStrength = n_attr.create("upperFaceStrength", "ufst", MFnNumericData::kFloat, 0.0);;
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setSoftMax(2.0);
    n_attr.setDefault(1.0);

    faceMaskLevel = n_attr.create("faceMaskLevel", "fml", MFnNumericData::kFloat, 0.0);;
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setSoftMax(1.0);
    n_attr.setDefault(0.6);

    faceMaskSoftness = n_attr.create("faceMaskSoftness", "fms", MFnNumericData::kFloat, 0.0);;
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0.001);
    n_attr.setSoftMax(0.5);
    n_attr.setDefault(0.0085);

    skinStrength = n_attr.create("skinStrength", "skst", MFnNumericData::kFloat, 0.0);;
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0);
    n_attr.setSoftMax(2.0);
    n_attr.setDefault(1.0);

    eyelidOpenOffset = n_attr.create("eyelidOpenOffset", "eoo", MFnNumericData::kFloat, 0.0);;
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(-1.0);
    n_attr.setSoftMax(1.0);
    n_attr.setDefault(0.06);

    lipOpenOffset = n_attr.create("lipOpenOffset", "loo", MFnNumericData::kFloat, 0.0);;
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(-0.2);
    n_attr.setSoftMax(0.2);
    n_attr.setDefault(-0.03);

    faceParams = c_attr.create("faceParameters", "skp");
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
    emotionStrength = n_attr.create("emotionStrength", "ems", MFnNumericData::kFloat, 0.6);;
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0.0);
    n_attr.setSoftMax(1.0);

    emotionContrast = n_attr.create("emotionContrast", "emc", MFnNumericData::kFloat, 1.0);;
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0.1);
    n_attr.setSoftMax(3.0);

    liveBlendCoef = n_attr.create("liveBlendCoef", "lbc", MFnNumericData::kFloat, 0.7);;
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0.0);
    n_attr.setSoftMax(1.0);

    maxEmotion = n_attr.create("maxEmotion", "mxe", MFnNumericData::kInt, 6);;
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(1);
    n_attr.setSoftMax(6);

    enablePreferredEmotion = n_attr.create(
        "enablePreferredEmotion", "epe", MFnNumericData::kBoolean, true);;
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);

    preferredEmotionStrength = n_attr.create(
        "preferredEmotionStrength", "pes", MFnNumericData::kFloat, 1.0);;
    n_attr.setKeyable(false);
    n_attr.setReadable(true);
    n_attr.setWritable(true);
    n_attr.setMin(0.0);
    n_attr.setSoftMax(1.0);

    emotionParams = c_attr.create("emotionParameters", "emp");
    c_attr.addChild(emotionStrength);
    c_attr.addChild(emotionContrast);
    c_attr.addChild(liveBlendCoef);
    c_attr.addChild(maxEmotion);
    c_attr.addChild(enablePreferredEmotion);
    c_attr.addChild(preferredEmotionStrength);
    addAttribute(emotionParams);

    ////////////////////////////////////////////////
    // Preferred Emotions
    ////////////////////////////////////////////////
    for(int i =0; i < EMOTION_COUNT; ++i){
        emotionList[i]  = n_attr.create(EMOTION_NAMES[i], "", MFnNumericData::kFloat, 0.0);
        n_attr.setKeyable(false);  // NOTE: need to be keyable
        n_attr.setReadable(true);
        n_attr.setWritable(true);
        n_attr.setMin(0.0);
        n_attr.setSoftMax(1.0);

    }
    emotions = c_attr.create("emotions", "emotions", &return_status);
    if(return_status != MS::kSuccess) return return_status;
    for(int i =0; i < EMOTION_COUNT; ++i){
        c_attr.addChild(emotionList[i] );
    }
    addAttribute(emotions);

    ///
    // Blendshape Parameters
    //
    for(int i =0; i < BLENDSHAPE_COUNT; ++i) {
        MString bs_name = MString(ARKIT_FACE_EXPRESSIONS[i]);
        blendshapeMultipliers[i]  = n_attr.create("multiply_" + bs_name, "", MFnNumericData::kFloat, 1.0);
        n_attr.setKeyable(true);
        n_attr.setReadable(true);
        n_attr.setWritable(true);
        n_attr.setSoftMin(0.0);
        n_attr.setSoftMax(2.0);
    }
    for(int i =0; i < BLENDSHAPE_COUNT; ++i) {
        MString bs_name = MString(ARKIT_FACE_EXPRESSIONS[i]);
        blendshapeOffsets[i]  = n_attr.create("offset_" + bs_name, "", MFnNumericData::kFloat, 0.0);
        n_attr.setKeyable(true);
        n_attr.setReadable(true);
        n_attr.setWritable(true);
        n_attr.setSoftMin(-1.0);
        n_attr.setSoftMax(1.0);
    }
    blendshapeParams = c_attr.create("blendshapeParameters", "blendshapeParameters", &return_status);
    if(return_status != MS::kSuccess) return return_status;
    for(int i =0; i < BLENDSHAPE_COUNT; ++i){
        c_attr.addChild(blendshapeMultipliers[i] );
        c_attr.addChild(blendshapeOffsets[i] );
    }
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

    outputEmotionState = n_attr.create("outputEmotionState", "outputEmotionState", MFnNumericData::kFloat, 0.0);
    n_attr.setArray(true);
    n_attr.setUsesArrayDataBuilder(true);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(false);
    n_attr.setIndexMatters(true);

    output = c_attr.create("output", "output", &return_status);
    if(return_status != MS::kSuccess) return return_status;
    c_attr.addChild(outputWeights);
    c_attr.addChild(outputEmotionState);
    addAttribute(output);

    ///
    // Hidden Outputs
    //
    outputBlendshapeNames = t_attr.create("outputBlendshapeNames", "on", MFnData::kString, &return_status);
    t_attr.setHidden(true);
    t_attr.setArray(true);
    t_attr.setUsesArrayDataBuilder(true);
    t_attr.setReadable(true);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setCached(false);
    n_attr.setKeyable(false);
    t_attr.setIndexMatters(true);
    addAttribute(outputBlendshapeNames);

    outputEmotionStateNames = t_attr.create("outputEmotionStateNames", "outputEmotionStateNames", MFnData::kString, &return_status);
    t_attr.setHidden(true);
    t_attr.setArray(true);
    t_attr.setUsesArrayDataBuilder(true);
    t_attr.setReadable(true);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setCached(false);
    n_attr.setKeyable(false);
    t_attr.setIndexMatters(true);
    addAttribute(outputEmotionStateNames);

    ///
    //Status
    //
    statusLoaded = n_attr.create("loaded", "sl", MFnNumericData::kBoolean, 0.0);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);

    statusLoadedAudio = t_attr.create("loadedAudio", "sla", MFnData::kString);
    t_attr.setReadable(true);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setCached(true);

    statusReceived = n_attr.create("received", "sr", MFnNumericData::kBoolean, 0.0);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);

    statusAudioSamples  = n_attr.create("audioSamples", "sas", MFnNumericData::kLong, 0);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);

    statusReceivedTime  = n_attr.create("receivedTime", "srt", MFnNumericData::kDouble, 0);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);

    statusReceivedFrames  = n_attr.create("receivedFrames", "srf", MFnNumericData::kLong, 0);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);

    statusCurrentFrame  = n_attr.create("currentFrame", "scf", MFnNumericData::kLong, 0);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);

    status = c_attr.create("status", "status", &return_status);
    if(return_status != MS::kSuccess) return return_status;
    c_attr.addChild(statusLoaded);
    c_attr.addChild(statusLoadedAudio);
    c_attr.addChild(statusAudioSamples);
    c_attr.addChild(statusReceived);
    c_attr.addChild(statusReceivedTime);
    c_attr.addChild(statusReceivedFrames);
    c_attr.addChild(statusCurrentFrame);
    addAttribute(status);

    ///
    // Controllers
    //
    triggerRequest = n_attr.create("triggerRequest", "tr", MFnNumericData::kDouble, 0, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    n_attr.setStorable(false);
    n_attr.setWritable(false);
    n_attr.setHidden(true);
    addAttribute(triggerRequest);

    triggerLoad = n_attr.create("triggerLoad", "tl", MFnNumericData::kDouble, 0, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    n_attr.setStorable(false);
    n_attr.setWritable(false);
    n_attr.setHidden(true);
    addAttribute(triggerLoad);

    // ... code to affect the attribute, if necessary ...
    attributeAffects(AceAnimationPlayer::time, AceAnimationPlayer::outputWeights);
    attributeAffects(AceAnimationPlayer::time, AceAnimationPlayer::outputEmotionState);
    attributeAffects(AceAnimationPlayer::time, AceAnimationPlayer::statusCurrentFrame);
    attributeAffects(AceAnimationPlayer::audioOffset, AceAnimationPlayer::outputWeights);
    attributeAffects(AceAnimationPlayer::audioOffset, AceAnimationPlayer::outputEmotionState);
    attributeAffects(AceAnimationPlayer::audioOffset, AceAnimationPlayer::statusCurrentFrame);
    attributeAffects(AceAnimationPlayer::audioStart, AceAnimationPlayer::outputWeights);
    attributeAffects(AceAnimationPlayer::audioStart, AceAnimationPlayer::outputEmotionState);
    attributeAffects(AceAnimationPlayer::audioStart, AceAnimationPlayer::statusCurrentFrame);
    attributeAffects(AceAnimationPlayer::audioEnd, AceAnimationPlayer::outputWeights);
    attributeAffects(AceAnimationPlayer::audioEnd, AceAnimationPlayer::outputEmotionState);
    attributeAffects(AceAnimationPlayer::audioEnd, AceAnimationPlayer::statusCurrentFrame);
    for(int i =0; i < BLENDSHAPE_COUNT; ++i) {
        attributeAffects(AceAnimationPlayer::blendshapeMultipliers[i], AceAnimationPlayer::outputWeights);
        attributeAffects(AceAnimationPlayer::blendshapeOffsets[i], AceAnimationPlayer::outputWeights);
    }

    attributeAffects(AceAnimationPlayer::audiofile, AceAnimationPlayer::triggerLoad);

    attributeAffects(AceAnimationPlayer::audiofile, AceAnimationPlayer::triggerRequest);
    attributeAffects(AceAnimationPlayer::audiofile, AceAnimationPlayer::outputBlendshapeNames);
    attributeAffects(AceAnimationPlayer::audiofile, AceAnimationPlayer::outputEmotionStateNames);
    attributeAffects(AceAnimationPlayer::networkAddress, AceAnimationPlayer::triggerRequest);
    attributeAffects(AceAnimationPlayer::networkAddress, AceAnimationPlayer::outputBlendshapeNames);
    attributeAffects(AceAnimationPlayer::networkAddress, AceAnimationPlayer::outputEmotionStateNames);
    attributeAffects(AceAnimationPlayer::apiKey, AceAnimationPlayer::triggerRequest);
    attributeAffects(AceAnimationPlayer::apiKey, AceAnimationPlayer::outputBlendshapeNames);
    attributeAffects(AceAnimationPlayer::apiKey, AceAnimationPlayer::outputEmotionStateNames);
    attributeAffects(AceAnimationPlayer::functionId, AceAnimationPlayer::triggerRequest);
    attributeAffects(AceAnimationPlayer::functionId, AceAnimationPlayer::outputBlendshapeNames);
    attributeAffects(AceAnimationPlayer::functionId, AceAnimationPlayer::outputEmotionStateNames);
    attributeAffects(AceAnimationPlayer::faceParams, AceAnimationPlayer::triggerRequest);
    attributeAffects(AceAnimationPlayer::faceParams, AceAnimationPlayer::outputBlendshapeNames);
    attributeAffects(AceAnimationPlayer::faceParams, AceAnimationPlayer::outputEmotionStateNames);
    for(int i =0; i < EMOTION_COUNT; ++i) {
        attributeAffects(AceAnimationPlayer::emotionList[i], AceAnimationPlayer::triggerRequest);
    }

    attributeAffects(AceAnimationPlayer::audiofile, AceAnimationPlayer::status);
    attributeAffects(AceAnimationPlayer::networkAddress, AceAnimationPlayer::status);
    attributeAffects(AceAnimationPlayer::apiKey, AceAnimationPlayer::status);
    attributeAffects(AceAnimationPlayer::functionId, AceAnimationPlayer::status);

    return MStatus::kSuccess;
}

void AceAnimationPlayer::postConstructor() {
}

void AceAnimationPlayer::lockAttribute(MObject attribute) {
    MObject thisNode = thisMObject();
    MFnDependencyNode fnNode(thisNode);
    MPlug plug = fnNode.findPlug(attribute, true);
    if (!plug.isNull()) {
        plug.setLocked(true);
    }
}


MStatus AceAnimationPlayer::compute( const MPlug& plug, MDataBlock& block) {
    // input networkAddress
    MString url_string = block.inputValue(networkAddress).asString();

    MStatus return_status = MS::kUnknownParameter;

    if (plug == triggerLoad || plug == triggerRequest) {
        // dirty audiofile input
        return_status = updateAudioBuffer(block);

        setOutput(block, statusAudioSamples, audioSamples.size());
        setOutput(block, statusLoadedAudio, currentFile);
        if (return_status == MS::kSuccess) {
            // succeeded to load new audio, or re-using the existing audio
            MString msg("Audio data is loaded: ");
            msg += (int)audioSamples.size();
            msg += " samples.";
            MGlobal::displayInfo(msg);
            setOutput(block, statusLoaded, true);
        }
        else {
            setOutput(block, statusLoaded, false);
        }
        setOutput(block, triggerLoad, (double)mace::GetCurrentTime());
    }

    if (plug == triggerRequest && url_string.length() > 1) {
        if (audioSamples.size() < DefaultBufferLength) {
            LOG_ERROR("Not enough audio samples to request animation: " << audioSamples.size() << " samples");
            return MS::kFailure;
        }

        // set parameters
        return_status = updateClientParameters(block);
        if (return_status != MS::kSuccess) {
            MGlobal::displayError("Failed to set parameters.");
            return MS::kFailure;
        }
        displayClientParameters();  // display sending parameters for visibility

        // request new animation and update frames
        return_status = updateAnimation(block);

        // set frames count, whether success or failure
        setOutput(block, statusReceivedFrames, client.GetFramesCount());
        setOutput(block, statusReceivedTime, (double)client.GetLastUpdated());

        if (return_status == MS::kSuccess) {
            setOutput(block, statusReceived, true);
            MString msg("Received animation of ");
            msg += (int)client.GetFramesCount();
            msg += " samples";
            MGlobal::displayInfo(msg);
        }
        else {
            setOutput(block, statusReceived, false);
            MGlobal::displayError("Failed to receive animation");
        }
        setOutput(block, triggerRequest, (double)mace::GetCurrentTime());
    }

    if (plug == outputBlendshapeNames) {
        if (client.GetFramesCount() < 1) {
            // no animation yet received or failed
            return MS::kFailure;
        }

        std::vector<std::string> names = client.GetBlendshapeNames();
        return_status = setOutputArray(block, outputBlendshapeNames, names);
        if (return_status != MS::kSuccess) {
            MGlobal::displayError("Cannot update output blendshape names");
        }
    }

    if (plug == outputEmotionStateNames) {
        if (client.GetFramesCount() < 1) {
            // no animation yet received or failed
            return MS::kFailure;
        }

        std::vector<std::string> names = client.GetEmotionStateNames();
        return_status = setOutputArray(block, outputEmotionStateNames, names);
        if (return_status != MS::kSuccess) {
            MGlobal::displayError("Cannot update output emotion state names");
        }
    }

    if (plug == outputWeights || plug == outputEmotionState || plug == statusCurrentFrame || plug == output || plug == status) {
        // NOTE: This section is usually pulled by "output" plug.
        if (client.GetFramesCount() < 1) {
            // no animation is received or there was a communication failure
            return MS::kFailure;
        }

        return_status = updateFrame(block);
        if (return_status != MS::kSuccess) {
            MGlobal::displayError("Cannot update animation");
        }

        block.setClean(outputWeights);
        block.setClean(outputEmotionState);
        block.setClean(statusCurrentFrame);
        block.setClean(output);
        block.setClean(status);
    }

    return return_status;
}

MStatus AceAnimationPlayer::updateFrame(MDataBlock &block) {
    /*
    Update output to represent the current time context.
    */
    MStatus return_status = MS::kFailure;

    // read time info
    MTime t = getCurrentAudioTime(block);
    double audio_position = t.asUnits(MTime::kSeconds);
    size_t frame_idx = client.GetFrameIndex((float)audio_position);

    // else: dirty current time
    std::vector<float> weights = getBlendshapeWeights(block, frame_idx);
    return_status = setOutputArray(block, outputWeights, weights);
    if (return_status != MS::kSuccess) {
        MGlobal::displayError("Cannot update output blendshape weights.");
        return return_status;
    }

    std::vector<float> emotion_state = getOutputEmotionState(block, frame_idx);
    return_status = setOutputArray(block, outputEmotionState, emotion_state);
    if (return_status != MS::kSuccess) {
        MGlobal::displayError("Cannot update output emotion state.");
        return return_status;
    }

    // update timestamps
    setOutput(block, statusCurrentFrame, frame_idx);
    lastUpdatedTime = client.GetLastUpdated();
    lastUpdatedFrame = frame_idx;

    return return_status;
}

std::vector<float> AceAnimationPlayer::getBlendshapeWeights(MDataBlock &block, size_t frame_index) {
    std::vector<float> weights = client.GetBlendshapeWeights(frame_index);

    // NOTE: disabled local compute for this version
    // for (size_t i = 0; i < std::min(BLENDSHAPE_COUNT, weights.size()); i++) {
    //     MDataHandle bs_gain = block.inputValue(blendshapeMultipliers[i]);
    //     MDataHandle bs_offset = block.inputValue(blendshapeOffsets[i]);
    //     weights[i] *= bs_gain.asFloat();
    //     weights[i] += bs_offset.asFloat();
    // }

    return weights;
}

std::vector<float> AceAnimationPlayer::getOutputEmotionState(MDataBlock &block, size_t frame_index) {
    return client.GetEmotionState(frame_index);
}

std::vector<std::string> AceAnimationPlayer::getBlendshapeNames() {
    std::vector<std::string> names = client.GetBlendshapeNames();
    return names;
}

MTime AceAnimationPlayer::getCurrentAudioTime(MDataBlock &block) {
    // get the adjusted audio time
    double t = getTimeAsSeconds(block, time);

    double audio_length = audioSamples.size() / (double)audioSamplerate;
    double audio_offset = getTimeAsSeconds(block, audioOffset);  // +offset: padding in front of the audio
    double audio_start = clamp(getTimeAsSeconds(block, audioStart), 0.0f, audio_length);
    double audio_end = getTimeAsSeconds(block, audioEnd);
    if (audio_end < 0.0) {
        audio_end = audio_length;
    }
    else {
        audio_end = clamp(audio_end, audio_start, audio_length);
    }

    double audio_timing = clamp(t - audio_offset + audio_start, audio_start, audio_end);

    return MTime(audio_timing, MTime::kSeconds);
}

double AceAnimationPlayer::getTimeAsSeconds(MDataBlock &block, MObject &time_attribute) {
    MTime current = block.inputValue(time_attribute).asTime();
    return current.asUnits(MTime::kSeconds);
}

MStatus AceAnimationPlayer::updateAudioBuffer(MDataBlock &block, bool force) {
    MStatus return_status = MS::kFailure;

    // input audiofile
    MString audiofile_mstring = block.inputValue(audiofile).asString();
    std::filesystem::path audiofile_path(audiofile_mstring.asUTF8());

    if (!std::filesystem::exists(audiofile_path)) {
        // file path is wrong
        std::string msg("Invalid audio file path: '" + audiofile_path.string() + "'");
        MGlobal::displayError(msg.c_str());
        return MS::kFailure;
    }

    if (audiofile_mstring == currentFile && !force) {
        // already loaded
        return MS::kSuccess;
    }

    // load and update audio data
    LOG_INFO("Loading a new audio file: " + audiofile_path.string());
    audioSamples.clear();
    return_status = loadAudioFile(audiofile_mstring, audioSamples);  // fills audioSamples vector

    if (return_status != MS::kSuccess || audioSamples.size() < 1) {
        std::string msg("No audio samples from the file: " + audiofile_path.string());
        MGlobal::displayError(msg.c_str());
        return MS::kFailure;
    }

    // succeeded
    currentFile = audiofile_mstring;
    return MS::kSuccess;
}

MStatus AceAnimationPlayer::updateClientParameters(MDataBlock &block) {
    // set parameters
    auto face_params = getFaceParameters(block);
    auto emo_params = getEmotionParameters(block);
    auto pref_emotion = getEmotionState(block);

    client.SetFaceParameters(face_params);
    client.SetEmotionParameters(emo_params);
    client.SetEmotionState(pref_emotion);
    for (size_t i = 0; i < BLENDSHAPE_COUNT; i++) {
        // set blendshape multipliers and offsets
        MDataHandle bs_gain = block.inputValue(blendshapeMultipliers[i]);
        client.SetBlendshapeMultiplier(ARKIT_FACE_EXPRESSIONS[i], bs_gain.asFloat());
        MDataHandle bs_offset = block.inputValue(blendshapeOffsets[i]);
        client.SetBlendshapeOffset(ARKIT_FACE_EXPRESSIONS[i], bs_offset.asFloat());
    }

    return MS::kSuccess;
}

void AceAnimationPlayer::displayClientParameters() {
    char buffer[120];

    MGlobal::displayInfo("  Sending with Parameters:");
    auto face_params = client.GetFaceParameters();
    for (auto const& entry : face_params.GetParameterMap()) {
        std::sprintf(buffer, "%30s: %1.4f", entry.first, entry.second);
        MGlobal::displayInfo(MString(buffer));
    }
    auto emo_params = client.GetEmotionParameters();
    for (auto const& entry : emo_params.GetParameterMap()) {
        std::sprintf(buffer, "%30s: %1.4f", entry.first, (float)entry.second);
        MGlobal::displayInfo(MString(buffer));
    }
    auto pref_emo = client.GetEmotionState();
    for (auto const& entry : pref_emo.GetParameterMap()) {
        std::sprintf(buffer, "%30s: %1.4f", entry.first, entry.second);
        MGlobal::displayInfo(MString(buffer));
    }

    MGlobal::displayInfo(MString("  Blendshape Multipliers and Offsets:"));
    auto multipliers = client.GetBlendshapeMultipliers();
    auto offsets = client.GetBlendshapeOffsets();
    for (size_t i = 0; i < BLENDSHAPE_COUNT; i++) {
        auto key = ARKIT_FACE_EXPRESSIONS[i];
        std::sprintf(buffer, "%30s: %1.4f  %1.4f", key, multipliers[key], offsets[key]);
        MGlobal::displayInfo(MString(buffer));
    }
}

MStatus AceAnimationPlayer::updateAnimation(MDataBlock &block) {
    AceClientStatus svc_status = AceClientStatus::ERROR_UNKNOWN;

    MString url_mstring = block.inputValue(networkAddress).asString();
    svc_status = client.SetUrl(url_mstring.asChar());
    if (svc_status != AceClientStatus::OK) {
        MGlobal::displayError("Cannot set url as " + url_mstring + ". Please ensure http:// or https:// prefix is added");
        return MS::kFailure;
    }
    MString apiKey_mstring = block.inputValue(apiKey).asString();
    svc_status = client.SetAPIKey(apiKey_mstring.asChar());
    if (svc_status != AceClientStatus::OK) {
        MGlobal::displayError("Cannot set api key as " + apiKey_mstring);
        return MS::kFailure;
    }
    MString functionId_mstring = block.inputValue(functionId).asString();
    svc_status = client.SetFunctionId(functionId_mstring.asChar());
    if (svc_status != AceClientStatus::OK) {
        MGlobal::displayError("Cannot set funciton id as " + functionId_mstring);
        return MS::kFailure;
    }

    // start communication
    MComputation computation;
    computation.beginComputation();
    svc_status = client.UpdateAnimation(audioSamples);
    computation.endComputation();

    if (svc_status != AceClientStatus::OK) {
        MString reason = "An unknown error occurred. Please verify that your server URL, API key, or function ID is correct.";
        if (auto iter = ACECLIENT_ERROR_MESSAGE_MAP.find(svc_status); iter != ACECLIENT_ERROR_MESSAGE_MAP.end()) {
            reason = iter->second;
        }
        MGlobal::displayError("Cannot retrieve data from " + url_mstring + ", reason: " + reason);
        return MS::kFailure;
    }
    currentUrl = url_mstring;
    return MS::kSuccess;
}

mace::AceFaceParameters AceAnimationPlayer::getFaceParameters(MDataBlock &block) {
    mace::AceFaceParameters params;

    params.SkinStrength = block.inputValue(skinStrength).asFloat();
    params.UpperFaceStrength = block.inputValue(upperFaceStrength).asFloat();
    params.LowerFaceStrength = block.inputValue(lowerFaceStrength).asFloat();
    params.EyelidOpenOffset = block.inputValue(eyelidOpenOffset).asFloat();
    params.LipOpenOffset = block.inputValue(lipOpenOffset).asFloat();
    params.UpperFaceSmoothing = block.inputValue(upperFaceSmoothing).asFloat();
    params.LowerFaceSmoothing = block.inputValue(lowerFaceSmoothing).asFloat();
    params.FaceMaskLevel = block.inputValue(faceMaskLevel).asFloat();
    params.FaceMaskSoftness = block.inputValue(faceMaskSoftness).asFloat();

    return params;
}

mace::AceEmotionParameters AceAnimationPlayer::getEmotionParameters(MDataBlock &block) {
    mace::AceEmotionParameters params;

    params.emotion_contrast = block.inputValue(emotionContrast).asFloat();
    params.live_blend_coef = block.inputValue(liveBlendCoef).asFloat();
    params.emotion_strength = block.inputValue(emotionStrength).asFloat();
    params.max_emotions = block.inputValue(maxEmotion).asInt();
    params.enable_preferred_emotion = block.inputValue(enablePreferredEmotion).asBool();
    params.preferred_emotion_strength = block.inputValue(preferredEmotionStrength).asFloat();

    return params;
}

mace::AceEmotionState AceAnimationPlayer::getEmotionState(MDataBlock &block) {
    mace::AceEmotionState params;

    params.amazement = block.inputValue(emotionList[0]).asFloat();
    params.anger = block.inputValue(emotionList[1]).asFloat();
    params.cheekiness = block.inputValue(emotionList[2]).asFloat();
    params.disgust = block.inputValue(emotionList[3]).asFloat();
    params.fear = block.inputValue(emotionList[4]).asFloat();
    params.grief = block.inputValue(emotionList[5]).asFloat();
    params.joy = block.inputValue(emotionList[6]).asFloat();
    params.out_of_breath = block.inputValue(emotionList[7]).asFloat();
    params.pain = block.inputValue(emotionList[8]).asFloat();
    params.sadness = block.inputValue(emotionList[9]).asFloat();

    return params;
}

MStatus AceAnimationPlayer::loadAudioFile(MString &filepath, std::vector<int16_t> &outBuffer) {
    std::vector<int16_t> padding_before(DefaultBufferOffset, 0);  // (4160, 0);
    std::vector<int16_t> padding_after(DefaultBufferOffset, 0);  // (4160, 0);

    std::vector<float> buffer_f = get_file_wav_content(filepath.asUTF8(), audioSamplerate);
    std::vector<int16_t> buffer = convert_float_to_int16(buffer_f);

    if(buffer.size() == 0) return MStatus::kFailure;

    outBuffer.insert(outBuffer.end(), buffer.begin(), buffer.end());

    return MS::kSuccess;
}

std::vector<float> AceAnimationPlayer::getInputArray(
    MDataBlock &block, MObject &attribute, MStatus *ptr_status) {

    std::vector<float> return_values;
    MArrayDataHandle array = block.inputArrayValue(attribute, ptr_status);
    for (size_t i = 0; i < array.elementCount(); i++) {
        array.jumpToArrayElement(i);
        MDataHandle data = array.inputValue(ptr_status);
        return_values.push_back(data.asFloat());
    }

    if (ptr_status != nullptr) {
        *ptr_status = MS::kSuccess;
    }
    return return_values;
}

MStatus AceAnimationPlayer::setOutputArray(
    MDataBlock &block, MObject &attribute, std::vector<float> &values, bool setClean) {

    MStatus return_status = MS::kFailure;
    MArrayDataHandle array_handle = block.outputArrayValue(attribute, &return_status);

    if (return_status != MS::kSuccess) {
        return return_status;
    }

    MArrayDataBuilder array_builder = array_handle.builder();
    int i = 0;
    for (auto value : values) {
        MDataHandle data_handle = array_builder.addElement(i++, &return_status);
        data_handle.setFloat(value);
    }
    array_handle.set(array_builder);

    if (setClean) {
        // array_handle.setAllClean();
        block.setClean(attribute);
    }
    return MS::kSuccess;
}

MStatus AceAnimationPlayer::setOutputArray(
    MDataBlock &block, MObject &attribute, std::vector<std::string> &values, bool setClean)
{
    MStatus return_status = MS::kFailure;
    MArrayDataHandle array_handle = block.outputArrayValue(attribute, &return_status);

    if (return_status != MS::kSuccess) {
        MGlobal::displayError("Cannot acquire output array.");
        return return_status;
    }

    MArrayDataBuilder array_builder = array_handle.builder();
    int i = 0;
    for (std::string value : values) {
        MDataHandle data_handle = array_builder.addElement(i++, &return_status);
        data_handle.setString(MString(value.c_str()));
    }
    array_handle.set(array_builder);

    if (setClean) {
        block.setClean(attribute);
    }
    return MS::kSuccess;
}

MStatus AceAnimationPlayer::setOutput(
    MDataBlock &block, MObject &attribute, MString &value, bool setClean)
{
    MStatus return_status = MS::kFailure;
    MDataHandle handle = block.outputValue(attribute, &return_status);

    handle.set(value);
    if (setClean) {
        block.setClean(attribute);
    }
    return MS::kSuccess;
}

MStatus AceAnimationPlayer::setOutput(
    MDataBlock &block, MObject &attribute, std::string &value, bool setClean)
{
    return setOutput(block, attribute, value.c_str(), setClean);
}

MStatus AceAnimationPlayer::setOutput(MDataBlock &block, MObject &attribute, char *value, bool setClean) {
    return setOutput(block, attribute, MString(value), setClean);
}

MStatus AceAnimationPlayer::setOutput(MDataBlock &block, MObject &attribute, int value, bool setClean) {
    return setOutput(block, attribute, (long)value, setClean);
}

MStatus AceAnimationPlayer::setOutput(MDataBlock &block, MObject &attribute, size_t value, bool setClean) {
    return setOutput(block, attribute, (long)value, setClean);
}

MStatus AceAnimationPlayer::setOutput(MDataBlock &block, MObject &attribute, long value, bool setClean) {
    MStatus return_status = MS::kFailure;
    MDataHandle data_handle = block.outputValue(attribute, &return_status);

    data_handle.set(value);
    if (setClean) {
        block.setClean(attribute);
    }
    return MS::kSuccess;
}

MStatus AceAnimationPlayer::setOutput(MDataBlock &block, MObject &attribute, double value, bool setClean) {
    MStatus return_status = MS::kFailure;
    MDataHandle data_handle = block.outputValue(attribute, &return_status);

    data_handle.set(value);
    if (setClean) {
        block.setClean(attribute);
    }
    return MS::kSuccess;
}

MStatus AceAnimationPlayer::setOutput(MDataBlock &block, MObject &attribute, float value, bool setClean) {
    return setOutput(block, attribute, (double)value, setClean);
}

MStatus AceAnimationPlayer::setOutput(MDataBlock &block, MObject &attribute, bool value, bool setClean) {
    MStatus return_status = MS::kFailure;
    MDataHandle data_handle = block.outputValue(attribute, &return_status);

    data_handle.set(value);
    if (setClean) {
        block.setClean(attribute);
    }
    return MS::kSuccess;
}
