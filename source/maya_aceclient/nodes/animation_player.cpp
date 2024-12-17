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

#include "aceclient/status.h"
#include "aceclient/audio.h"
#include "aceclient/animation.h"
#include "aceclient/a2f_client_interface.h"
#include "aceclient/frame_receiver.h"
#include "aceclient/logger.h"

#include "common/names.h"

#include "profiler.h"

MTypeId AceAnimationPlayer::id(0x9011f);
const char* AceAnimationPlayer::typeName = "AceAnimationPlayer";

MObject AceAnimationPlayer::time;

MObject AceAnimationPlayer::audio;
MObject AceAnimationPlayer::audiofile;
MObject AceAnimationPlayer::audioOffset;
MObject AceAnimationPlayer::audioStart;
MObject AceAnimationPlayer::audioEnd;

MObject AceAnimationPlayer::clientType;
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
MObject AceAnimationPlayer::statusNeedsUpdate;

MObject AceAnimationPlayer::triggerSendAudio;
MObject AceAnimationPlayer::triggerLoad;

const std::unordered_map<AceClientStatus, MString> ACECLIENT_ERROR_MESSAGE_MAP = {
    {AceClientStatus::ERROR_UNAUTHENTICATED,
    "Invalid or empty API key provided. Please check and try again."},
    {AceClientStatus::ERROR_SSL_HANDSHAKE,
    "The remote server does not support SSL. Try using HTTP instead of HTTPS."},
    {AceClientStatus::ERROR_CONNECTION,
    "The remote server is unreachable or is using HTTPS when HTTP is expected."},
    {AceClientStatus::ERROR_DNS_RESOLUTION,
    "DNS resolution failed. Please ensure that the server URL is correct."},
    {AceClientStatus::ERROR_CREDITS_EXPIRED,
    "Your API key has run out of cloud credits;"
    " Please get in touch with NVIDIA representatives for assistance."}
};

namespace {
/* Display NVIDIA Cloud Agreement terms, and acquire an acceptance to proceed.
*/
const MString NVCF_AGREEMENT_OPTIONVAR = "NVCF_AGREEMENT_ACCEPTED";
const MString NVCF_AGREENENT_HTML =
"NVIDIA Audio2Face-3D Authoring Microservice and NVIDIA Audio2Face-3D Microservice NIM ('Services')"
" allow you to upload audio files to drive an animation. NVIDIA will only use and store the audio files"
" to provide you with the Services. For more information about our data processing practices, see our"
" <a href='https://www.nvidia.com/en-us/about-nvidia/privacy-policy/'>Privacy Policy</a>."
" By clicking 'Agree', you consent to the processing of your data in accordance with the"
" <a href='https://www.nvidia.com/en-us/agreements/cloud-services/nvidia-cloud-agreement/'>NVIDIA Cloud Agreement</a>"
" and"
" <a href='https://developer.download.nvidia.com/licenses/Service_Specific_Terms_for_NVIDIA_Audio2Face_3D_Authoring_Microservice_and_NVIDIA_Audio2Face_3D_Microservice_NIM.pdf'>"
"Service-Specific Terms for NVIDIA Audio2Face-3D Authoring Microservice and NVIDIA Audio2Face-3D Microservice NIM</a>"
".";
const MString NVCF_AGREEMENT =
"NVIDIA Audio2Face-3D Authoring Microservice and NVIDIA Audio2Face-3D Microservice NIM ('Services')"
" allow you to upload audio files to drive an animation. NVIDIA will only use and store the audio files"
" to provide you with the Services. For more information about our data processing practices, see our"
" 'Privacy Policy'<https://www.nvidia.com/en-us/about-nvidia/privacy-policy/>."
" By proceeding, you consent to the processing of your data in accordance with the"
" 'NVIDIA Cloud Agreement'<https://www.nvidia.com/en-us/agreements/cloud-services/nvidia-cloud-agreement>"
" and"
" 'Service-Specific Terms for NVIDIA Audio2Face-3D Authoring Microservice and NVIDIA Audio2Face-3D Microservice NIM'"
"<https://developer.download.nvidia.com/licenses/Service_Specific_Terms_for_NVIDIA_Audio2Face_3D_Authoring_Microservice_and_NVIDIA_Audio2Face_3D_Microservice_NIM.pdf>"
".";
const MString NVCF_AGREEMENT_INSTRUCTIONS =
"To agree and proceed, Please set an optionVar " + NVCF_AGREEMENT_OPTIONVAR + " as 1.\n"
" MEL Script: optionVar -iv \"" + NVCF_AGREEMENT_OPTIONVAR + "\" 1;\n"
" Python: maya.cmds.optionVar(iv=['" + NVCF_AGREEMENT_OPTIONVAR + "', 1])\n";

bool acquireAgreement() {
    // OPTME: utilize Maya's preference system to store the consent
    bool exists;
    int agreed = MGlobal::optionVarIntValue(NVCF_AGREEMENT_OPTIONVAR, &exists);

    if (exists && agreed) {
        MGlobal::displayInfo("You agreed to the terms of the NVIDIA Cloud Agreement. Proceeding.");
        return true;
    }

    // detect if we are running in batch mode
    if (MGlobal::mayaState() != MGlobal::kInteractive) {
        MGlobal::displayInfo("\n" + NVCF_AGREEMENT + "\n");
        MGlobal::displayError(NVCF_AGREEMENT_INSTRUCTIONS);
        return false;
    }

    // pop up dialog
    MString command = "confirmDialog -title \"Maya-ACE Prominent Disclosure\" "
                    "-message \"" + NVCF_AGREENENT_HTML + "\" "
                    "-button \"Agree\" "
                    "-button \"Disagree\" "
                    "-defaultButton \"Agree\" "
                    "-dismissString \"Disagree\";";
    MString response;
    MGlobal::executeCommand(command, response);

    // Check user response
    if (response == "Agree") {
        MGlobal::displayInfo("Thank you for agreeing to the terms!");
        MGlobal::setOptionVarValue(NVCF_AGREEMENT_OPTIONVAR, 1);
        return true;
    }

    MGlobal::displayError("You must agree to the terms to proceed.");
    return false;
}
}

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
    MFnEnumAttribute e_attr;
    MFnStringData defaultStringData;

    // ace server configs
    clientType = e_attr.create(
        "clientType", "ct", (short)mace::A2FClientType::eA2FControllerClient, &return_status);
    CHECK_MSTATUS_AND_RETURN_IT(return_status);
    e_attr.addField("Streaming", (short)mace::A2FClientType::eA2FControllerClient);
    e_attr.addField("Authoring", (short)mace::A2FClientType::eA2FAuthoringClient);
    e_attr.setStorable(true); // Attribute will be stored with the scene data
    e_attr.setKeyable(false); // Attribute will appear in the channel box when true
    e_attr.setDefault("Streaming");
    return_status = addAttribute(clientType);
    CHECK_MSTATUS_AND_RETURN_IT(return_status);

    networkAddress = t_attr.create("networkAddress", "na", MFnData::kString, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    t_attr.setStorable(true); // Attribute will be stored with the scene data
    t_attr.setKeyable(false); // Attribute will appear in the channel box when true
    t_attr.setDefault(defaultStringData.create("https://grpc.nvcf.nvidia.com:443"));
    addAttribute(networkAddress);

    apiKey = t_attr.create("apiKey", "apiKey", MFnData::kString, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    t_attr.setStorable(true); // Attribute will be stored with the scene data
    t_attr.setKeyable(false); // Attribute will appear in the channel box when true
    t_attr.setDefault(defaultStringData.create("$NVCF_API_KEY")); // read from env or empty
    addAttribute(apiKey);

    functionId = t_attr.create("functionId", "functionId", MFnData::kString, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    t_attr.setStorable(true); // Attribute will be stored with the scene data
    t_attr.setKeyable(false); // Attribute will appear in the channel box when true
    // obtained from https://build.nvidia.com/nvidia/audio2face-3d/api
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
    t_attr.setKeyable(false); // Attribute will appear in the channel box when true

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

    maxEmotion = n_attr.create("maxEmotion", "mxe", MFnNumericData::kInt);
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
    n_attr.setDefault(1.0);

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
        emotionList[i]  = n_attr.create(EMOTION_NAMES[i], "", MFnNumericData::kFloat);
        n_attr.setKeyable(false);  // NOTE: need to be keyable
        n_attr.setReadable(true);
        n_attr.setWritable(true);
        n_attr.setMin(0.0);
        n_attr.setSoftMax(1.0);
        n_attr.setDefault(0.0);

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
        blendshapeMultipliers[i]  = n_attr.create(
            "multiply_" + bs_name, "", MFnNumericData::kFloat);
        n_attr.setKeyable(false);
        n_attr.setReadable(true);
        n_attr.setWritable(true);
        n_attr.setSoftMin(0.0);
        n_attr.setSoftMax(2.0);
        n_attr.setDefault(1.0);
    }
    for(int i =0; i < BLENDSHAPE_COUNT; ++i) {
        MString bs_name = MString(ARKIT_FACE_EXPRESSIONS[i]);
        blendshapeOffsets[i]  = n_attr.create(
            "offset_" + bs_name, "", MFnNumericData::kFloat);
        n_attr.setKeyable(false);
        n_attr.setReadable(true);
        n_attr.setWritable(true);
        n_attr.setSoftMin(-1.0);
        n_attr.setSoftMax(1.0);
        n_attr.setDefault(0.0);
    }
    blendshapeParams = c_attr.create(
        "blendshapeParameters", "blendshapeParameters", &return_status);
    if(return_status != MS::kSuccess) return return_status;
    for(int i =0; i < BLENDSHAPE_COUNT; ++i){
        c_attr.addChild(blendshapeMultipliers[i] );
        c_attr.addChild(blendshapeOffsets[i] );
    }
    addAttribute(blendshapeParams);

    ///
    // Output
    //
    outputWeights = n_attr.create("outputWeights", "ow", MFnNumericData::kFloat);
    n_attr.setArray(true);
    n_attr.setUsesArrayDataBuilder(true);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(false);
    n_attr.setIndexMatters(true);
    n_attr.setDefault(0.0);

    outputEmotionState = n_attr.create("outputEmotionState", "oes", MFnNumericData::kFloat);
    n_attr.setArray(true);
    n_attr.setUsesArrayDataBuilder(true);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(false);
    n_attr.setIndexMatters(true);
    n_attr.setDefault(0.0);

    output = c_attr.create("output", "output", &return_status);
    if(return_status != MS::kSuccess) return return_status;
    c_attr.addChild(outputWeights);
    c_attr.addChild(outputEmotionState);
    addAttribute(output);

    ///
    // Hidden Outputs
    //
    outputBlendshapeNames = t_attr.create(
        "outputBlendshapeNames", "obsn", MFnData::kString, &return_status);
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

    outputEmotionStateNames = t_attr.create(
        "outputEmotionStateNames", "oesn", MFnData::kString, &return_status);
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
    statusLoaded = n_attr.create("loaded", "sl", MFnNumericData::kBoolean);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);
    n_attr.setDefault(0.0);

    statusLoadedAudio = t_attr.create("loadedAudio", "sla", MFnData::kString);
    t_attr.setReadable(true);
    t_attr.setWritable(false);
    t_attr.setStorable(false);
    t_attr.setCached(true);

    statusReceived = n_attr.create("received", "sr", MFnNumericData::kBoolean);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);
    n_attr.setDefault(false);

    statusAudioSamples  = n_attr.create("audioSamples", "sas", MFnNumericData::kLong);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);
    n_attr.setDefault(0);

    statusReceivedTime  = n_attr.create("receivedTime", "srt", MFnNumericData::kDouble);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);
    n_attr.setDefault(0.0);

    statusReceivedFrames  = n_attr.create("receivedFrames", "srf", MFnNumericData::kLong, 0);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);
    n_attr.setDefault(0);

    statusCurrentFrame  = n_attr.create("currentFrame", "scf", MFnNumericData::kLong);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);
    n_attr.setDefault(0);

    statusNeedsUpdate = n_attr.create("needsUpdate", "snu", MFnNumericData::kBoolean);
    n_attr.setReadable(true);
    n_attr.setWritable(false);
    n_attr.setStorable(false);
    n_attr.setCached(true);
    n_attr.setDefault(true);

    status = c_attr.create("status", "status", &return_status);
    if(return_status != MS::kSuccess) return return_status;
    c_attr.addChild(statusLoaded);
    c_attr.addChild(statusLoadedAudio);
    c_attr.addChild(statusAudioSamples);
    c_attr.addChild(statusReceived);
    c_attr.addChild(statusReceivedTime);
    c_attr.addChild(statusReceivedFrames);
    c_attr.addChild(statusCurrentFrame);
    c_attr.addChild(statusNeedsUpdate);
    addAttribute(status);

    ///
    // Controllers
    //
    triggerSendAudio = n_attr.create(
        "triggerSendAudio", "tr", MFnNumericData::kDouble, 0, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    n_attr.setStorable(false);
    n_attr.setWritable(false);
    n_attr.setHidden(true);
    addAttribute(triggerSendAudio);

    triggerLoad = n_attr.create(
        "triggerLoad", "tl", MFnNumericData::kDouble, 0, &return_status);
    if(return_status != MS::kSuccess) return return_status;
    n_attr.setStorable(false);
    n_attr.setWritable(false);
    n_attr.setHidden(true);
    addAttribute(triggerLoad);

    // ... code to affect the attribute, if necessary ...
    attributeAffects(AceAnimationPlayer::audiofile, AceAnimationPlayer::triggerLoad);

    attributeAffects(AceAnimationPlayer::audiofile, AceAnimationPlayer::triggerSendAudio);
    attributeAffects(AceAnimationPlayer::audiofile, AceAnimationPlayer::outputBlendshapeNames);
    attributeAffects(AceAnimationPlayer::audiofile, AceAnimationPlayer::outputEmotionStateNames);
    attributeAffects(AceAnimationPlayer::clientType, AceAnimationPlayer::triggerSendAudio);
    attributeAffects(AceAnimationPlayer::clientType, AceAnimationPlayer::outputBlendshapeNames);
    attributeAffects(AceAnimationPlayer::clientType, AceAnimationPlayer::outputEmotionStateNames);
    attributeAffects(AceAnimationPlayer::networkAddress, AceAnimationPlayer::triggerSendAudio);
    attributeAffects(
        AceAnimationPlayer::networkAddress, AceAnimationPlayer::outputBlendshapeNames);
    attributeAffects(
        AceAnimationPlayer::networkAddress, AceAnimationPlayer::outputEmotionStateNames);
    attributeAffects(AceAnimationPlayer::apiKey, AceAnimationPlayer::triggerSendAudio);
    attributeAffects(AceAnimationPlayer::apiKey, AceAnimationPlayer::outputBlendshapeNames);
    attributeAffects(AceAnimationPlayer::apiKey, AceAnimationPlayer::outputEmotionStateNames);
    attributeAffects(AceAnimationPlayer::functionId, AceAnimationPlayer::triggerSendAudio);
    attributeAffects(AceAnimationPlayer::functionId, AceAnimationPlayer::outputBlendshapeNames);
    attributeAffects(AceAnimationPlayer::functionId, AceAnimationPlayer::outputEmotionStateNames);

    attributeAffects(time, AceAnimationPlayer::outputWeights);
    attributeAffects(time, AceAnimationPlayer::outputEmotionState);
    attributeAffects(time, AceAnimationPlayer::statusCurrentFrame);
    attributeAffects(time, AceAnimationPlayer::statusReceived);
    attributeAffects(audioOffset, AceAnimationPlayer::outputWeights);
    attributeAffects(audioOffset, AceAnimationPlayer::outputEmotionState);
    attributeAffects(audioOffset, AceAnimationPlayer::statusCurrentFrame);
    attributeAffects(audioOffset, AceAnimationPlayer::statusReceived);
    attributeAffects(audioStart, AceAnimationPlayer::outputWeights);
    attributeAffects(audioStart, AceAnimationPlayer::outputEmotionState);
    attributeAffects(audioStart, AceAnimationPlayer::statusCurrentFrame);
    attributeAffects(audioStart, AceAnimationPlayer::statusReceived);
    attributeAffects(audioEnd, AceAnimationPlayer::outputWeights);
    attributeAffects(audioEnd, AceAnimationPlayer::outputEmotionState);
    attributeAffects(audioEnd, AceAnimationPlayer::statusCurrentFrame);
    attributeAffects(audioEnd, AceAnimationPlayer::statusReceived);

    // a2f parameters
    MFnCompoundAttribute fn_params_a2f(faceParams);
    for (int i=0; i < fn_params_a2f.numChildren(); i++) {
        setAffectsAnimationOutput(fn_params_a2f.child(i));
    }

    // a2e parameters
    MFnCompoundAttribute fn_params_a2e(emotionParams);
    for (int i=0; i < fn_params_a2e.numChildren(); i++) {
        setAffectsAnimationOutput(fn_params_a2e.child(i));
    }

    // preferred emotion
    MFnCompoundAttribute fn_params_emo(emotions);
    for (int i=0; i < fn_params_emo.numChildren(); i++) {
        setAffectsAnimationOutput(fn_params_emo.child(i));
    }

    // blendshape parameters
    MFnCompoundAttribute fn_params_bs(blendshapeParams);
    for (int i=0; i < fn_params_bs.numChildren(); i++) {
        setAffectsAnimationOutput(fn_params_bs.child(i));
    }

    attributeAffects(AceAnimationPlayer::audiofile, AceAnimationPlayer::status);
    attributeAffects(AceAnimationPlayer::clientType, AceAnimationPlayer::status);
    attributeAffects(AceAnimationPlayer::networkAddress, AceAnimationPlayer::status);
    attributeAffects(AceAnimationPlayer::apiKey, AceAnimationPlayer::status);
    attributeAffects(AceAnimationPlayer::functionId, AceAnimationPlayer::status);

    return MStatus::kSuccess;
}

void AceAnimationPlayer::setAffectsAnimationOutput(MObject &source) {
    attributeAffects(source, AceAnimationPlayer::outputWeights);
    attributeAffects(source, AceAnimationPlayer::outputEmotionState);
    attributeAffects(source, AceAnimationPlayer::statusCurrentFrame);
    attributeAffects(source, AceAnimationPlayer::statusReceived);
    attributeAffects(source, AceAnimationPlayer::statusNeedsUpdate);
    attributeAffects(source, AceAnimationPlayer::triggerSendAudio);
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
    short clientTypeVal = block.inputValue(clientType).asShort();

    MStatus return_status = MS::kUnknownParameter;

    // may load audio from disk
    if (plug == triggerLoad || plug == triggerSendAudio) {
        // dirty audiofile input
        return_status = loadAudio(block);

        setOutput(block, statusAudioSamples, audioSamples.size());
        setOutput(block, statusLoadedAudio, currentAudiofile);
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
        setOutput(block, triggerLoad, (double)mace::GetTick());
    }

    // send audio and may receive animation (streaming client)
    if (plug == triggerSendAudio && url_string.length() > 1) {
        if (mace::IsPrefix("https://grpc.nvcf.nvidia.com", url_string.asChar())) {
            // Check consent for nvcf endpoint
            if (!acquireAgreement()) {
                setOutput(block, statusReceived, false);
                return MStatus::kFailure;
            }
        }

        if (audioSamples.size() < DEFAULT_BUFFER_LENGTH) {
            LOG_ERROR("Not enough audio samples: " << audioSamples.size() << " samples");
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
        return_status = sendAudio(block);

        // set frames count, whether success or failure
        setOutput(block, statusReceivedFrames, animClient.GetFramesCount());
        setOutput(block, statusReceivedTime, (double)animClient.GetLastUpdated());

        if (return_status == MS::kSuccess) {
            setOutput(block, statusReceived, true);
            setOutput(block, statusNeedsUpdate, false);
            setOutput(block, triggerSendAudio, (double)mace::GetTick());
            block.setClean(triggerSendAudio);
            MString msg("Received animation of ");
            msg += (unsigned int)animClient.GetFramesCount() + " frames";
            MGlobal::displayInfo(msg);
        }
        else {
            setOutput(block, statusReceived, false);
            MGlobal::displayError("Failed to receive animation");
        }
    }

    // update output attribute with downloaded blendshape names
    if (plug == outputBlendshapeNames) {
        if (animClient.GetFramesCount() < 1) {
            // no animation yet received or failed
            return MS::kFailure;
        }

        std::vector<std::string> names = animClient.GetBlendshapeNames();
        return_status = setOutputArray(block, outputBlendshapeNames, names);
        if (return_status != MS::kSuccess) {
            MGlobal::displayError("Cannot update output blendshape names");
        }
    }

    // update output attribute with downloaded emotion names
    if (plug == outputEmotionStateNames) {
        if (animClient.GetFramesCount() < 1) {
            // no animation yet received or failed
            return MS::kFailure;
        }

        std::vector<std::string> names = animClient.GetEmotionStateNames();
        return_status = setOutputArray(block, outputEmotionStateNames, names);
        if (return_status != MS::kSuccess) {
            MGlobal::displayError("Cannot update output emotion state names");
        }
    }

    // main block. update output weights/emotion with downloaded data
    if (
        plug == triggerSendAudio
        || plug == outputWeights || plug == outputEmotionState
        || plug == statusCurrentFrame || plug == statusReceived
        || plug == output || plug == status
    )
    {
        // NOTE: This section is usually pulled by "output" plug.
        if (animClient.GetFramesCount() < 1) {
            // no animation is received or there was a communication failure
            return MS::kFailure;
        }

        if (!block.isClean(triggerSendAudio)) {
            // set status dirty if any parameter changed and it needs new animation
            // this will be cleared after sending audio, or fetch frame on authoring
            setOutput(block, statusNeedsUpdate, true);
        }

        return_status = updateFrameOutput(block);
    }

    block.setClean(plug.attribute());

    return return_status;
}

MStatus AceAnimationPlayer::updateFrameOutput(MDataBlock &block) {
    /* Update output to represent the current time context.

    Streaming client: read downloaded animation for the given frame
    Authoring client: fetch a new frame for the frame time
    */
    MStatus return_status = MS::kFailure;

    // read time info
    MTime t = getCurrentAudioTime(block);
    double audio_position = t.asUnits(MTime::kSeconds);
    size_t frame_idx = animClient.GetFrameIndex((float)audio_position);

    // else: dirty current time
    std::vector<float> weights = {};
    return_status = getBlendshapeWeights(block, frame_idx, weights);
    if (return_status != MS::kSuccess) {
        MGlobal::displayError("Cannot acquire blendshape weights.");
        return return_status;
    }

    return_status = setOutputArray(block, outputWeights, weights);
    if (return_status != MS::kSuccess) {
        MGlobal::displayError("Cannot update output blendshape weights.");
        return return_status;
    }

    std::vector<float> emotion_state = {};
    getOutputEmotionState(block, frame_idx, emotion_state);
    return_status = setOutputArray(block, outputEmotionState, emotion_state);
    if (return_status != MS::kSuccess) {
        MGlobal::displayError("Cannot update output emotion state.");
        return return_status;
    }

    // update timestamps
    setOutput(block, statusCurrentFrame, frame_idx);
    lastUpdatedTime = animClient.GetLastUpdated();
    lastUpdatedFrame = frame_idx;

    return return_status;
}

MStatus AceAnimationPlayer::getBlendshapeWeights(
    MDataBlock &block, size_t frame_index, std::vector<float> &out_weights)
{
    // fetch on authoring service
    short clientTypeVal = block.inputValue(clientType).asShort();

    if (clientTypeVal == (short)mace::A2FClientType::eA2FAuthoringClient) {
        // Fetch frame when using authoring service
        updateClientParameters(block);
        {
            const maceProfilingScope nodeScope(
                "AceAnimationPlayer_FetchAnimationFrame", thisMObject());
            AceClientResult result = animClient.FetchAnimationFrame(frame_index);
            if (result.status != AceClientStatus::OK) {
                setOutput(block, statusReceived, false);
                MGlobal::displayError(result.message.c_str());
                return MStatus::kFailure;
            }
            setOutput(block, statusReceived, true);
            setOutput(block, statusNeedsUpdate, false);
        }
    }

    // read blendshapes from animation frames
    // std::vector<float> weights = animClient.GetBlendshapeWeights(frame_index);
    for (auto x: animClient.GetBlendshapeWeights(frame_index)) {
        out_weights.emplace_back(x);
    }

    return MStatus::kSuccess;
}

MStatus AceAnimationPlayer::getOutputEmotionState(
    MDataBlock &block, size_t frame_index, std::vector<float> &out_emotion)
{
    for (auto x: animClient.GetEmotionState(frame_index)) {
        out_emotion.emplace_back(x);
    }
    return MStatus::kSuccess;
}

std::vector<std::string> AceAnimationPlayer::getBlendshapeNames() {
    std::vector<std::string> names = animClient.GetBlendshapeNames();
    return names;
}

MTime AceAnimationPlayer::getCurrentAudioTime(MDataBlock &block) {
    // get the adjusted audio time
    double t = getTimeAsSeconds(block, time);

    double audio_length = audioSamples.size() / (double)audioSamplerate;
    // +offset: padding in front of the audio
    double audio_offset = getTimeAsSeconds(block, audioOffset);
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

MStatus AceAnimationPlayer::loadAudio(MDataBlock &block, bool force) {
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

    if (audiofile_mstring == currentAudiofile && !force) {
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
    currentAudiofile = audiofile_mstring;
    return MS::kSuccess;
}

MStatus AceAnimationPlayer::updateClientParameters(MDataBlock &block) {
    // set parameters
    auto face_params = getFaceParameters(block);
    auto emo_params = getEmotionParameters(block);
    auto pref_emotion = getEmotionState(block);

    animClient.SetFaceParameters(face_params);
    animClient.SetEmotionParameters(emo_params);
    animClient.SetEmotionState(pref_emotion);
    for (size_t i = 0; i < BLENDSHAPE_COUNT; i++) {
        // set blendshape multipliers and offsets
        MDataHandle bs_gain = block.inputValue(blendshapeMultipliers[i]);
        animClient.SetBlendshapeMultiplier(ARKIT_FACE_EXPRESSIONS[i], bs_gain.asFloat());
        MDataHandle bs_offset = block.inputValue(blendshapeOffsets[i]);
        animClient.SetBlendshapeOffset(ARKIT_FACE_EXPRESSIONS[i], bs_offset.asFloat());
    }

    return MS::kSuccess;
}

void AceAnimationPlayer::displayClientParameters() {
    char buffer[120];

    MGlobal::displayInfo("  Sending with Parameters:");
    auto face_params = animClient.GetFaceParameters();
    for (auto const& entry : face_params.GetParameterMap()) {
        std::sprintf(buffer, "%30s: %1.4f", entry.first.c_str(), entry.second);
        MGlobal::displayInfo(MString(buffer));
    }
    auto emo_params = animClient.GetEmotionParameters();
    for (auto const& entry : emo_params.GetParameterMap()) {
        std::sprintf(buffer, "%30s: %1.4f", entry.first.c_str(), (float)entry.second);
        MGlobal::displayInfo(MString(buffer));
    }
    auto pref_emo = animClient.GetEmotionState();
    for (auto const& entry : pref_emo.GetParameterMap()) {
        std::sprintf(buffer, "%30s: %1.4f", entry.first.c_str(), entry.second);
        MGlobal::displayInfo(MString(buffer));
    }

    MGlobal::displayInfo(MString("  Blendshape Multipliers and Offsets:"));
    auto multipliers = animClient.GetBlendshapeMultipliers();
    auto offsets = animClient.GetBlendshapeOffsets();
    for (size_t i = 0; i < BLENDSHAPE_COUNT; i++) {
        auto key = ARKIT_FACE_EXPRESSIONS[i];
        std::sprintf(buffer, "%30s: %1.4f  %1.4f", key, multipliers[key], offsets[key]);
        MGlobal::displayInfo(MString(buffer));
    }
}

MStatus AceAnimationPlayer::sendAudio(MDataBlock &block) {
    AceClientResult svc_result = AceClientResult{AceClientStatus::ERROR_UNKNOWN};

    short clientTypeVal = block.inputValue(clientType).asShort();
    svc_result = animClient.SetClientType(clientTypeVal);
    if (svc_result.status != AceClientStatus::OK) {
        MGlobal::displayError("Cannot set clientType as " + clientTypeVal);
        return MS::kFailure;
    }
    MString url_mstring = block.inputValue(networkAddress).asString();
    animClient.SetAddress(url_mstring.asChar());
    MString apiKey_mstring = block.inputValue(apiKey).asString();
    animClient.SetAPIKey(apiKey_mstring.asChar());
    MString functionId_mstring = block.inputValue(functionId).asString();
    animClient.SetFunctionId(functionId_mstring.asChar());

    // start communication
    MComputation computation;
    computation.beginComputation();
    svc_result = animClient.ConnectAndSendAudio(audioSamples);
    computation.endComputation();

    if (svc_result.status != AceClientStatus::OK) {
        MString reason = "An unknown error occurred.";
        reason += " Please verify that your client Type, server URL, API key,";
        reason += " or function ID is correct.";
        auto iter = ACECLIENT_ERROR_MESSAGE_MAP.find(svc_result.status);
        if (iter != ACECLIENT_ERROR_MESSAGE_MAP.end()) {
            reason = iter->second;
        }
        MGlobal::displayError(
            "Cannot retrieve data from " + url_mstring
            + "\nPossible reason: " + reason + "\nDetail: "
            + MString{svc_result.message.c_str()});
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
    params.outofbreath = block.inputValue(emotionList[7]).asFloat();
    params.pain = block.inputValue(emotionList[8]).asFloat();
    params.sadness = block.inputValue(emotionList[9]).asFloat();

    return params;
}

MStatus AceAnimationPlayer::loadAudioFile(MString &filepath, std::vector<int16_t> &outBuffer) {
    std::vector<int16_t> padding_before(DEFAULT_BUFFER_OFFSET, 0);  // (4160, 0);
    std::vector<int16_t> padding_after(DEFAULT_BUFFER_OFFSET, 0);  // (4160, 0);

    std::vector<float> buffer_f = get_file_wav_content(filepath.asUTF8(), audioSamplerate);
    std::vector<int16_t> buffer = convert_float_to_int16(buffer_f);

    if(buffer.size() == 0) return MStatus::kFailure;

    outBuffer.insert(outBuffer.end(), buffer.begin(), buffer.end());

    return MS::kSuccess;
}

std::vector<float> AceAnimationPlayer::getInputArray(
    MDataBlock &block, MObject &attribute, MStatus *ptr_status)
{
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

template <typename T>
MStatus AceAnimationPlayer::setOutputArray(
    MDataBlock &block, MObject &attribute, std::vector<T> &values, bool setClean)
{
    MStatus return_status = MS::kFailure;
    MArrayDataHandle array_handle = block.outputArrayValue(attribute, &return_status);

    if (return_status != MS::kSuccess) {
        return return_status;
    }

    MArrayDataBuilder array_builder = array_handle.builder();

    // reduce size of the output array if needed
    unsigned int cur_size = array_handle.elementCount();
    unsigned int new_size = values.size();
    if (cur_size > new_size) {
        array_handle.jumpToArrayElement(0);
        for (cur_size; new_size < cur_size; cur_size--) {
            array_builder.removeElement(cur_size - 1);
        }
    }

    // add/update elements
    int i = 0;
    for (auto value : values) {
        MDataHandle data_handle = array_builder.addElement(i++, &return_status);
        setData(data_handle, value);
    }
    array_handle.set(array_builder);

    if (setClean) {
        // array_handle.setAllClean();
        block.setClean(attribute);
    }
    return MS::kSuccess;
}

template <typename T>
MStatus AceAnimationPlayer::setOutput(
    MDataBlock &block, MObject &attribute, T value, bool setClean)
{
    MStatus return_status = MS::kFailure;
    MDataHandle handle = block.outputValue(attribute, &return_status);

    if (return_status != MS::kSuccess) {
        return return_status;
    }

    setData(handle, value);

    if (setClean) {
        block.setClean(attribute);
    }
    return MS::kSuccess;
}

template <typename T>
void AceAnimationPlayer::setData(MDataHandle &handle, T value) {
    if constexpr (std::is_same<T, std::string>::value) {
        handle.setString(MString(value.c_str()));
    }
    else if constexpr (std::is_same<T, char *>::value) {
        handle.setString(MString(value));
    }
    else if constexpr (std::is_same<T, size_t>::value) {
        handle.set(int(value));
    }
    else {
        handle.set(value);
    }
}
