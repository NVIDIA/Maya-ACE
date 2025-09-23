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
#pragma once

#include <memory>
#include <vector>
#include <string>

#include <maya/MGlobal.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MPxNode.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnCompoundAttribute.h>

#include "ace_client/frame_receiver.h"
#include "ace_client/parameters.h"
#include "ace_client/animation.h"
#include "ace_client/audio.h"


class AceAnimationPlayer: public MPxNode
{
public:
    AceAnimationPlayer();
    ~AceAnimationPlayer() override;

    static  void*		creator();
    static  MStatus		initialize();
    MStatus	compute(const MPlug&, MDataBlock&) override;

    MStatus updateClientParameters(MDataBlock &block);
    void displayClientParameters();

    MStatus loadAudio(MDataBlock &block, bool force=false);
    MStatus sendAudio(MDataBlock &block);
    MStatus updateFrameOutput(MDataBlock &block);

    MStatus getBlendshapeWeights(
        MDataBlock &block, size_t frame_index, std::vector<float> &out_weights);
    MStatus getoutputEmotions(
        MDataBlock &block, size_t frame_index, std::vector<float> &out_emotion);
    std::vector<std::string> getBlendshapeNames();

    MTime getCurrentAudioTime(MDataBlock &block);
    double getTimeAsSeconds(MDataBlock &block, MObject &time_attribute);

    void lockAttribute(MObject attribute);

    static MTypeId id;
    static const char* typeName;

    // Time Inputs
    static MObject time;

    // Audio Inputs
    static MObject audioParams;
    static MObject audiofile;
    static MObject audioOffset;
    static MObject audioStart;
    static MObject audioEnd;

    // Service Configs
    static MObject serviceParams;
    static MObject networkAddress;
    static MObject apiKey;
    static MObject functionId;

    // Emotion Parameters
    static MObject preferredEmotions;

    static MObject emotionParams;
    static MObject emotionStrength;
    static MObject emotionContrast;
    static MObject maxEmotions;
    static MObject liveBlendCoef;
    static MObject enablePreferredEmotion;
    static MObject preferredEmotionStrength;

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

    // tongue parameters (future)
    // static MObject tongueParams;
    // static MObject tongueStrength;
    // static MObject tongueHeightOffset;
    // static MObject tongueDepthOffset;

    static MObject blendshapeParams;
    // blendshape parameters
    static MObject blendshapeFaceMultipliers;
    static MObject blendshapeFaceOffsets;
    // static MObject blendshapeTongueMultipliers;
    // static MObject blendshapeTongueOffsets;

    // Outputs
    static MObject outputAnimationResults;
    static MObject outputWeights;
    static MObject outputWeightNames;

    static MObject outputEmotionStates;
    static MObject outputEmotions;
    static MObject outputEmotionNames;

    // Internal and Status attributes
    static MObject status;
    static MObject statusLoaded;  // flag that audio file is loaded
    static MObject statusLoadedAudio;  // currently loaded file
    static MObject statusAudioSamples;  // number of audio samples
    static MObject statusReceived;  // flag that animation data is received
    static MObject statusReceivedTime;  // tick time of received animation
    static MObject statusReceivedFrames;  // number of received animation frames
    static MObject statusCurrentFrame;  // the current frame number
    static MObject statusNeedsUpdate;  // the dirty status needing new download

    static MObject triggerSendAudio; // attribute to control service request
    static MObject triggerLoad; // attribute to control service request
    static MObject triggerUpdateParameters; // attribute to trigger parameter update per service config

private:
    std::vector<int16_t> audioSamples;
    size_t audioSamplerate = DEFAULT_SAMPLE_RATE;

    MString currentAudiofile = "";
    MString currentUrl = "";

    mace::NetworkedAnimation animClient;
    long long lastUpdatedTime = 0;
    int lastUpdatedFrame = -(1 << 15);

    mace::AceFaceParameters getFaceParameters(MDataBlock &block);
    mace::AceEmotionParameters getEmotionParameters(MDataBlock &block);
    mace::AceEmotionState getEmotionState(MDataBlock &block);

    MStatus computeLoadAudio(MDataBlock &block);
    MStatus synchronizeArrayAttributes(MDataBlock &block);

    MStatus loadAudioFile(MString &audiofile, std::vector<int16_t> &outBuffer);

    static void setAffectsNeedsUpdate(MObject &source);
};
