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

#include <maya/MPxGeometryFilter.h>
#include <maya/MItGeometry.h>

#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MGlobal.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnNumericData.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnPluginData.h>
#include <maya/MDGMessage.h>
#include <maya/MCallbackIdArray.h>

#include "nodes/ace_player.h"
#include "nodes/a2f_player.h"
#include "nodes/a2x_plugin_data.h"
#include "commands/request_sendaudio.h"
#include "commands/export_service_config.h"
#include "profiler.h"
#include "common/timeslider.h"

MCallbackIdArray gCallbackIds;

void addNodeCB(MObject& node, void* clientData) {
    MFnDependencyNode nodeFn(node);
    //MGlobal::displayInfo(MString("add callback node: ") + nodeFn.name());
    if (nodeFn.typeId() == A2FAnimationPlayer::id) {
        A2FAnimationPlayer* a2fNode = dynamic_cast<A2FAnimationPlayer*>(nodeFn.userNode());
        assert(a2fNode);
        a2fNode->setDeleted(false);
    }
}

void removeNodeCB(MObject& node, void* clientData) {
    MFnDependencyNode nodeFn(node);
    //MGlobal::displayInfo(MString("Remove callback node: ") + nodeFn.name());
    if (nodeFn.typeId() == A2FAnimationPlayer::id) {
        A2FAnimationPlayer* a2fNode = dynamic_cast<A2FAnimationPlayer*>(nodeFn.userNode());
        assert(a2fNode);
        a2fNode->setDeleted(true);
    }
}

MStatus initializePlugin( MObject obj )
{
	MStatus result;
	MFnPlugin plugin(obj, "NVIDIA", "2.0", "Any");

    if (!maceProfilingCategory::InitializeProfiler()) { return MStatus::kFailure; }

    result = plugin.registerData(MayaExecutorResultsData::Name(), MayaExecutorResultsData::Id(), &MayaExecutorResultsData::creator);
    CHECK_MSTATUS_AND_RETURN_IT(result);
    result = plugin.registerData(MayaAudioAccumulatorData::Name(), MayaAudioAccumulatorData::Id(), &MayaAudioAccumulatorData::creator);
    CHECK_MSTATUS_AND_RETURN_IT(result);
    result = plugin.registerData(MayaEmotionAccumulatorData::Name(), MayaEmotionAccumulatorData::Id(), &MayaEmotionAccumulatorData::creator);
    CHECK_MSTATUS_AND_RETURN_IT(result);
    result = plugin.registerData(MayaCudaStreamData::Name(), MayaCudaStreamData::Id(), &MayaCudaStreamData::creator);
    CHECK_MSTATUS_AND_RETURN_IT(result);
    result = plugin.registerData(MayaA2EExecutorData::Name(), MayaA2EExecutorData::Id(), &MayaA2EExecutorData::creator);
    CHECK_MSTATUS_AND_RETURN_IT(result);
    result = plugin.registerData(MayaA2FExecutorData::Name(), MayaA2FExecutorData::Id(), &MayaA2FExecutorData::creator);
    CHECK_MSTATUS_AND_RETURN_IT(result);
    result = plugin.registerData(MayaBackgroundEvaluationData::Name(), MayaBackgroundEvaluationData::Id(), &MayaBackgroundEvaluationData::creator);
    CHECK_MSTATUS_AND_RETURN_IT(result);

    result = plugin.registerNode(
        AceAnimationPlayer::typeName, AceAnimationPlayer::id,
        AceAnimationPlayer::creator, AceAnimationPlayer::initialize, MPxNode::kDependNode
    );
    result = plugin.registerNode(
        A2FAnimationPlayer::typeName, A2FAnimationPlayer::id,
        A2FAnimationPlayer::creator, A2FAnimationPlayer::initialize, MPxNode::kDependNode
    );
    // Register the custom command
    plugin.registerCommand(
        AceRequestSendAudioCommand::commandName, AceRequestSendAudioCommand::creator);
    plugin.registerCommand(
        AceExportServiceConfigCommand::commandName, AceExportServiceConfigCommand::creator, AceExportServiceConfigCommand::newSyntax);

    gCallbackIds.append( MDGMessage::addNodeAddedCallback(addNodeCB, A2FAnimationPlayer::typeName, NULL, &result));
    CHECK_MSTATUS_AND_RETURN_IT(result);
    gCallbackIds.append( MDGMessage::addNodeRemovedCallback(removeNodeCB, A2FAnimationPlayer::typeName, NULL, &result));

	return result;
}

MStatus uninitializePlugin( MObject obj)
{
    MStatus result;
    MFnPlugin plugin(obj);

    MMessage::removeCallbacks(gCallbackIds);
    gCallbackIds.clear();

    TimeSliderProgress::clean();
    
    result = plugin.deregisterNode(AceAnimationPlayer::id);
    result = plugin.deregisterNode(A2FAnimationPlayer::id);

    plugin.deregisterCommand(AceRequestSendAudioCommand::commandName);
    plugin.deregisterCommand(AceExportServiceConfigCommand::commandName);

    result = plugin.deregisterData( MayaExecutorResultsData::Id() );
    result = plugin.deregisterData( MayaAudioAccumulatorData::Id() );
    result = plugin.deregisterData( MayaEmotionAccumulatorData::Id() );
    result = plugin.deregisterData( MayaCudaStreamData::Id() );
    result = plugin.deregisterData( MayaA2EExecutorData::Id() );
    result = plugin.deregisterData( MayaA2FExecutorData::Id() );
    result = plugin.deregisterData( MayaBackgroundEvaluationData::Id() );

    if (!maceProfilingCategory::UninitializeProfiler()) { result = MStatus::kFailure; }

    return result;
}
