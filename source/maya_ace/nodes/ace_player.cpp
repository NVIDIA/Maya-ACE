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

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <maya/MGlobal.h>
#include <maya/MComputation.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MStringArray.h>
#include <maya/MFnDependencyNode.h>

#include "ace_client/status.h"
#include "ace_client/audio.h"
#include "ace_client/animation.h"
#include "ace_client/a2f_client_interface.h"
#include "ace_client/frame_receiver.h"
#include "ace_client/logger.h"

#include "common/names.h"
#include "common/nvcloud.h"
#include "common/maya_utils.h"

#include "profiler.h"

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

using mace_maya::GetArrayInput;
using mace_maya::SetOutput;
using mace_maya::SetArrayOutput;
using mace_maya::CreateArrayAliases;
using mace_maya::RemoveArrayAliases;

AceAnimationPlayer::AceAnimationPlayer() {}
AceAnimationPlayer::~AceAnimationPlayer(){}

MStatus AceAnimationPlayer::compute(const MPlug& plug, MDataBlock& block) {
    // input networkAddress
    MString url_string = block.inputValue(networkAddress).asString();

    MStatus return_status = MS::kUnknownParameter;

    // update parameters, especially arrays and aliases
    if (plug == triggerUpdateParameters || lastUpdatedTime < 1) {
        return_status = synchronizeArrayAttributes(block);
        if (return_status != MS::kSuccess) {
            MGlobal::displayError("Failed to update parameters.");
            return return_status;
        }

        SetOutput(block, triggerUpdateParameters, (double)mace::GetTick());
        if (lastUpdatedTime < 1) {
            // manage flag to avoid re-initializing inputs too often
            lastUpdatedTime = 1;
        }
    }

    // may load audio from disk
    if (plug == triggerLoad || plug == triggerSendAudio) {
        return_status = computeLoadAudio(block);
        if (return_status != MS::kSuccess) {
            MGlobal::displayError("Failed to load audio.");
            return return_status;
        }
        SetOutput(block, triggerLoad, (double)mace::GetTick());
    }

    // send audio and may receive animation (streaming client)
    if (plug == triggerSendAudio && url_string.length() > 1) {
        if (mace::IsPrefix("https://grpc.nvcf.nvidia.com", url_string.asChar())) {
            // Check consent for nvcf endpoint
            if (!nvcloud::acquireAgreement()) {
                SetOutput(block, statusReceived, false);
                return MS::kFailure;
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
        SetOutput(block, statusReceivedFrames, animClient.GetFramesCount());
        SetOutput(block, statusReceivedTime, (double)animClient.GetLastUpdated());

        if (return_status == MS::kSuccess) {
            SetOutput(block, statusReceived, true);
            SetOutput(block, statusNeedsUpdate, false);
            SetOutput(block, triggerSendAudio, (double)mace::GetTick());
            block.setClean(triggerSendAudio);
            std::string msg = "Received animation of ";
            msg += std::to_string(animClient.GetFramesCount()) + " frames";
            MGlobal::displayInfo(msg.c_str());
        }
        else {
            SetOutput(block, statusReceived, false);
            MGlobal::displayError("Failed to receive animation");
        }

        synchronizeArrayAttributes(block);
    }

    // update outputWeightNames attribute with downloaded blendshape names
    if (plug == outputWeightNames) {
        if (animClient.GetFramesCount() < 1) {
            // no animation yet received or failed
            return MS::kFailure;
        }

        std::vector<std::string> names = animClient.GetBlendshapeNames();
        return_status = SetArrayOutput(block, outputWeightNames, names);
        if (return_status != MS::kSuccess) {
            MGlobal::displayError("Cannot update output blendshape names");
        }
    }

    // update outputEmotionNames attribute with downloaded emotion names
    if (plug == outputEmotionNames) {
        if (animClient.GetFramesCount() < 1) {
            // no animation yet received or failed
            return MS::kFailure;
        }

        std::vector<std::string> names = animClient.GetEmotionStateNames();
        return_status = SetArrayOutput(block, outputEmotionNames, names);
        if (return_status != MS::kSuccess) {
            MGlobal::displayError("Cannot update output emotion state names");
        }
    }

    // main block. update output weights/emotion with downloaded data
    if (plug == triggerSendAudio
        || plug == outputAnimationResults
        || plug == outputWeights 
        || plug == outputEmotionStates
        || plug == outputEmotions 
        || plug == statusCurrentFrame 
        || plug == statusReceived 
        || plug == statusNeedsUpdate
        || plug == status
    )
    {
        // NOTE: This section is usually pulled by "output" plug.
        if (animClient.GetFramesCount() < 1) {
            // no animation is received or there was a communication failure
            return MS::kFailure;
        }

        if (!block.isClean(triggerSendAudio)) {
            // set status dirty if any parameter changed and it needs new animation.
            // this will be cleared after sending audio.
            SetOutput(block, statusNeedsUpdate, true);
        }

        return_status = updateFrameOutput(block);
    }
    block.setClean(plug.attribute());

    return return_status;
}

MStatus AceAnimationPlayer::synchronizeArrayAttributes(MDataBlock &block) 
{
    MStatus status = MS::kFailure;

    // TODO: read this from server or configs
    auto emo_state = animClient.GetEmotionState();
    std::vector<float> last_emotions = {
        // read from animation client
        emo_state.amazement,  // 0
        emo_state.anger,      // 1
        emo_state.cheekiness, // 2
        emo_state.disgust,    // 3
        emo_state.fear,       // 4
        emo_state.grief,      // 5
        emo_state.joy,        // 6
        emo_state.outofbreath,// 7
        emo_state.pain,       // 8
        emo_state.sadness     // 9
    };
    std::vector<float> cur_emotions;
    status = GetArrayInput(block, preferredEmotions, cur_emotions);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Cannot get preferred emotions.");
        return status;
    }
    if (cur_emotions.size() > 0) {
        // read from maya scene
        for (int i=0; i < std::min(cur_emotions.size(), last_emotions.size()); i++) {
            last_emotions[i] = cur_emotions[i];
        }
    }
    SetArrayOutput(block, preferredEmotions, last_emotions);

    // NOTE: This needs to be data-driven some day
    std::vector<std::string> emotionNames;
    for (size_t i = 0; i < EMOTION_COUNT; ++i) {
        emotionNames.emplace_back(A2F_EMOTION_NAMES[i]);
    }
    // Set up array aliases for preferredEmotions
    status = RemoveArrayAliases(preferredEmotions, thisMObject());
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to remove array aliases for preferredEmotions.");
        return status;
    }
    status = RemoveArrayAliases(blendshapeFaceMultipliers, thisMObject());
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to remove array aliases for faceMultipliers.");
        return status;
    }
    status = RemoveArrayAliases(blendshapeFaceOffsets, thisMObject());
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to remove array aliases for faceOffsets.");
        return status;
    }
    status = CreateArrayAliases(
        preferredEmotions, thisMObject(), emotionNames, "preferred_");
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to create array aliases for preferredEmotions.");
        return status;
    }

    // TODO: This should be data-driven from the server config
    std::vector<std::string> facePoseNames(
        ARKIT_FACE_EXPRESSIONS, ARKIT_FACE_EXPRESSIONS + BLENDSHAPE_COUNT);
    status = CreateArrayAliases(
        blendshapeFaceMultipliers, thisMObject(), facePoseNames, "faceMultiplier_");
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to create array aliases for blendshapeFaceMultipliers.");
        return status;
    }
    status = CreateArrayAliases(
        blendshapeFaceOffsets, thisMObject(), facePoseNames, "faceOffset_");
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to create array aliases for faceOffsets.");
        return status;
    }

    return MS::kSuccess;
}

MStatus AceAnimationPlayer::computeLoadAudio(MDataBlock &block) {
    // dirty audiofile input
    MStatus return_status = loadAudio(block);

    // set status no matter success or failure
    SetOutput(block, statusAudioSamples, audioSamples.size());
    SetOutput(block, statusLoadedAudio, currentAudiofile);

    if (return_status == MS::kSuccess) {
        // succeeded to load new audio, or re-using the existing audio
        MString msg("Audio data is loaded: ");
        msg += (int)audioSamples.size();
        msg += " samples.";
        MGlobal::displayInfo(msg);
        SetOutput(block, statusLoaded, true);
    }
    else {
        SetOutput(block, statusLoaded, false);
    }

    return return_status;
}

MStatus AceAnimationPlayer::updateFrameOutput(MDataBlock &block) {
    /* Update output to represent the current time context.

    Streaming client: read downloaded animation for the given frame
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

    return_status = SetArrayOutput(block, outputWeights, weights);
    if (return_status != MS::kSuccess) {
        MGlobal::displayError("Cannot update output blendshape weights.");
        return return_status;
    }

    std::vector<float> emotion_state = {};
    getoutputEmotions(block, frame_idx, emotion_state);
    return_status = SetArrayOutput(block, outputEmotions, emotion_state);
    if (return_status != MS::kSuccess) {
        MGlobal::displayError("Cannot update output emotion state.");
        return return_status;
    }

    // update timestamps
    SetOutput(block, statusCurrentFrame, frame_idx);
    lastUpdatedTime = animClient.GetLastUpdated();
    lastUpdatedFrame = frame_idx;

    return return_status;
}

MStatus AceAnimationPlayer::getBlendshapeWeights(
    MDataBlock &block, size_t frame_index, std::vector<float> &out_weights)
{
    // read blendshapes from animation frames
    // std::vector<float> weights = animClient.GetBlendshapeWeights(frame_index);
    for (auto x: animClient.GetBlendshapeWeights(frame_index)) {
        out_weights.emplace_back(x);
    }

    return MStatus::kSuccess;
}

MStatus AceAnimationPlayer::getoutputEmotions(
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

    // TODO: use server-specific blendshape definitions when it is ready.
    std::vector<float> multipliers;
    std::vector<float> offsets;
    std::vector<std::string> blendshape_names(
        ARKIT_FACE_EXPRESSIONS, ARKIT_FACE_EXPRESSIONS + BLENDSHAPE_COUNT);
    CHECK_MSTATUS_AND_RETURN_IT(GetArrayInput(block, blendshapeFaceMultipliers, multipliers));
    CHECK_MSTATUS_AND_RETURN_IT(GetArrayInput(block, blendshapeFaceOffsets, offsets));

    for (size_t i = 0; i < std::min(multipliers.size(), blendshape_names.size()); i++) {
        animClient.SetBlendshapeMultiplier(blendshape_names[i], multipliers[i]);
    }
    for (size_t i = 0; i < std::min(offsets.size(), blendshape_names.size()); i++) {
        animClient.SetBlendshapeOffset(blendshape_names[i], offsets[i]);
    }

    return MS::kSuccess;
}

namespace {

    template <typename... Args>
    void safeDisplayInfo(const char* fmt, Args&&... args) {
        // First, determine the size needed (excluding null terminator)
        const int size = std::snprintf(nullptr, 0, fmt, args...);
        if (size <= 0) {
            // Fallback: just display the format string
            assert(!"Should not happen, error in format string");
            MGlobal::displayInfo(MString(fmt));
            return;
        }
        // Allocate buffer (+1 for null terminator)
        std::vector<char> buf(size + 1);
        std::snprintf(buf.data(), buf.size(), fmt, args...);
        MGlobal::displayInfo(MString(buf.data()));
    }

}

void AceAnimationPlayer::displayClientParameters() {
    MGlobal::displayInfo("  Sending with Parameters:");
    auto face_params = animClient.GetFaceParameters();
    for (auto const& entry : face_params.GetParameterMap()) {
        safeDisplayInfo("%30s: %1.4f", entry.first.c_str(), entry.second);
    }
    auto emo_params = animClient.GetEmotionParameters();
    for (auto const& entry : emo_params.GetParameterMap()) {
        safeDisplayInfo("%30s: %1.4f", entry.first.c_str(), entry.second);
    }
    auto pref_emo = animClient.GetEmotionState();
    for (auto const& entry : pref_emo.GetParameterMap()) {
        safeDisplayInfo("%30s: %1.4f", entry.first.c_str(), entry.second);
    }

    MGlobal::displayInfo(MString("  Blendshape Multipliers and Offsets:"));
    auto multipliers = animClient.GetBlendshapeMultipliers();
    auto offsets = animClient.GetBlendshapeOffsets();
    for (size_t i = 0; i < BLENDSHAPE_COUNT; i++) {
        auto key = ARKIT_FACE_EXPRESSIONS[i];
        safeDisplayInfo("%30s: %1.4f  %1.4f", key, multipliers[key], offsets[key]);
    }
}

MStatus AceAnimationPlayer::sendAudio(MDataBlock &block) {
    AceClientResult svc_result = AceClientResult{AceClientStatus::ERROR_UNKNOWN};

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
        reason += " Please verify that your server URL, API key,";
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
    params.max_emotions = block.inputValue(maxEmotions).asInt();
    params.enable_preferred_emotion = block.inputValue(enablePreferredEmotion).asBool();
    params.preferred_emotion_strength = block.inputValue(preferredEmotionStrength).asFloat();

    return params;
}

mace::AceEmotionState AceAnimationPlayer::getEmotionState(MDataBlock &block) {
    mace::AceEmotionState params;

    std::vector<float> preferredEmotion;
    auto status = GetArrayInput(block, preferredEmotions, preferredEmotion);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Cannot get preferred emotions.");
        return params;
    }

    // same order as A2F_EMOTION_NAMES
    // NOTE: This needs to be data-driven from the server config
    for (int i = 0; i < preferredEmotion.size(); i++) {
        switch (i) {
            case 0: params.amazement = preferredEmotion[i]; break;
            case 1: params.anger = preferredEmotion[i]; break;
            case 2: params.cheekiness = preferredEmotion[i]; break;
            case 3: params.disgust = preferredEmotion[i]; break;
            case 4: params.fear = preferredEmotion[i]; break;
            case 5: params.grief = preferredEmotion[i]; break;
            case 6: params.joy = preferredEmotion[i]; break;
            case 7: params.outofbreath = preferredEmotion[i]; break;
            case 8: params.pain = preferredEmotion[i]; break;
            case 9: params.sadness = preferredEmotion[i]; break;
        }
    }
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

void AceAnimationPlayer::lockAttribute(MObject attribute) {
    MObject thisNode = thisMObject();
    MFnDependencyNode fnNode(thisNode);
    MPlug plug = fnNode.findPlug(attribute, true);
    if (!plug.isNull()) {
        plug.setLocked(true);
    }
}
