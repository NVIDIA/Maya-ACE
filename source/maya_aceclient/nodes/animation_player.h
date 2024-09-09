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
#pragma once

#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MGlobal.h>
#include <maya/MStringArray.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnNumericData.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnPluginData.h>

#include <memory>
#include <vector>
#include <string>

#include "aceclient/frame_receiver.h"
#include "aceclient/parameters.h"
#include "aceclient/animation.h"
#include "aceclient/audio.h"

#include "common/names.h"


class AceAnimationPlayer: public MPxNode
{
public:
    AceAnimationPlayer();
    ~AceAnimationPlayer() override;

    static  void*		creator();
    static  MStatus		initialize();
    MStatus	compute(const MPlug&, MDataBlock&) override;
    void postConstructor();

    MStatus updateClientParameters(MDataBlock &block);
    void displayClientParameters();
    MStatus updateAudioBuffer(MDataBlock &block, bool force=false);
    MStatus updateAnimation(MDataBlock &block);
    MStatus updateFrame(MDataBlock &block);
    std::vector<float> getBlendshapeWeights(MDataBlock &block, size_t frame_index);
    std::vector<float> getOutputEmotionState(MDataBlock &block, size_t frame_index);
    std::vector<std::string> getBlendshapeNames();

    MTime getCurrentAudioTime(MDataBlock &block);
    double getTimeAsSeconds(MDataBlock &block, MObject &time_attribute);

    void lockAttribute(MObject attribute);

    static MObject audio;
    static MObject audiofile;
    static MObject audioOffset;
    static MObject audioStart;
    static MObject audioEnd;

    static MObject time;
    static MObject networkAddress;
    static MObject apiKey;
    static MObject functionId;

    static MObject faceParams;
    // skin parameters
    static MObject lowerFaceSmoothing;
    static MObject upperFaceSmoothing;
    static MObject lowerFaceStrength;
    static MObject upperFaceStrength;
    static MObject faceMaskLevel;
    static MObject faceMaskSoftness;
    static MObject skinStrength;
    static MObject eyelidOpenOffset;
    static MObject lipOpenOffset;

    static MObject emotions;
    static MObject emotionList[EMOTION_COUNT];

    static MObject emotionParams;
    // emotion paramters
    static MObject emotionStrength;
    static MObject emotionContrast;
    static MObject maxEmotion;
    static MObject liveBlendCoef;
    static MObject enablePreferredEmotion;
    static MObject preferredEmotionStrength;

    static MObject blendshapeParams;
    // blendshape parameters
    static MObject blendshapeMultipliers[BLENDSHAPE_COUNT];
    static MObject blendshapeOffsets[BLENDSHAPE_COUNT];
    // static MObject blendshapeNames;

    static MObject output;
    static MObject outputWeights;
    static MObject outputBlendshapeNames;
    static MObject outputEmotionState;
    static MObject outputEmotionStateNames;

    static MObject status;
    static MObject statusLoaded;  // flag that audio file is loaded
    static MObject statusLoadedAudio;  // currently loaded file
    static MObject statusAudioSamples;  // number of audio samples
    static MObject statusReceived;  // flag that animation data is received
    static MObject statusReceivedTime;  // tick time of received animation
    static MObject statusReceivedFrames;  // number of received animation frames
    static MObject statusCurrentFrame;  // the current frame number

    static MObject triggerRequest; // attribute to control service request
    static MObject triggerLoad; // attribute to control service request

    static MTypeId id;
    static const char* typeName;

private:
    std::vector<int16_t> audioSamples;
    size_t audioSamplerate = DefaultSampleRate;
    MString currentFile = "";
    MString currentUrl = "";

    mace::AnimationClient client;
    long long lastUpdatedTime = 0;
    int lastUpdatedFrame = -(1 << 15);

    mace::AceFaceParameters getFaceParameters(MDataBlock &block);
    mace::AceEmotionParameters getEmotionParameters(MDataBlock &block);
    mace::AceEmotionState getEmotionState(MDataBlock &block);

    MStatus loadAudioFile(MString &audiofile, std::vector<int16_t> &outBuffer);

    std::vector<float> getInputArray(MDataBlock &block, MObject &attribute, MStatus *return_status=nullptr);

    MStatus setOutput(MDataBlock &block, MObject &attribute, MString &value, bool setClean=true);
    MStatus setOutput(MDataBlock &block, MObject &attribute, std::string &value, bool setClean=true);
    MStatus setOutput(MDataBlock &block, MObject &attribute, char *value, bool setClean=true);
    MStatus setOutput(MDataBlock &block, MObject &attribute, long value, bool setClean=true);
    MStatus setOutput(MDataBlock &block, MObject &attribute, int value, bool setClean=true);
    MStatus setOutput(MDataBlock &block, MObject &attribute, size_t value, bool setClean=true);
    MStatus setOutput(MDataBlock &block, MObject &attribute, double value, bool setClean=true);
    MStatus setOutput(MDataBlock &block, MObject &attribute, float value, bool setClean=true);
    MStatus setOutput(MDataBlock &block, MObject &attribute, bool value, bool setClean=true);
    MStatus setOutputArray(
        MDataBlock &block, MObject &attribute, std::vector<float> &values, bool setClean=true);
    MStatus setOutputArray(
        MDataBlock &block, MObject &attribute, std::vector<std::string> &values, bool setClean=true);
};
