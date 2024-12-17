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

#include "nodes/animation_player.h"
#include "commands/request_sendaudio.h"
#include "commands/export_config_parameters.h"
#include "commands/import_config_parameters.h"
#include "commands/export_service_config.h"

#include "profiler.h"


MStatus initializePlugin( MObject obj )
{
	MStatus result;
	MFnPlugin plugin(obj, "NVIDIA", "1.2", "Any");

    if (!maceProfilingCategory::InitializeProfiler()) { return MStatus::kFailure; }

    result = plugin.registerNode(
        AceAnimationPlayer::typeName, AceAnimationPlayer::id,
        AceAnimationPlayer::creator, AceAnimationPlayer::initialize, MPxNode::kDependNode
    );
    // Register the custom command
    plugin.registerCommand(
        AceRequestSendAudioCommand::commandName, AceRequestSendAudioCommand::creator);
    plugin.registerCommand(
        AceExportConfigParametersCommand::commandName, AceExportConfigParametersCommand::creator);
    plugin.registerCommand(
        AceImportConfigParametersCommand::commandName, AceImportConfigParametersCommand::creator);
    plugin.registerCommand(
        AceExportServiceConfigCommand::commandName, AceExportServiceConfigCommand::creator, AceExportServiceConfigCommand::newSyntax);
	return result;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus result;
	MFnPlugin plugin(obj);
    result = plugin.deregisterNode(AceAnimationPlayer::id);

    plugin.deregisterCommand(AceRequestSendAudioCommand::commandName);
    plugin.deregisterCommand(AceExportConfigParametersCommand::commandName);
    plugin.deregisterCommand(AceImportConfigParametersCommand::commandName);
    plugin.deregisterCommand(AceExportServiceConfigCommand::commandName);

    if (!maceProfilingCategory::UninitializeProfiler()) { result = MStatus::kFailure; }

    return result;
}
