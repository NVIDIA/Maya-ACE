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
#include "request_sendaudio.h"

#include <string>

#include "ace_client/status.h"

#include "nodes/ace_player.h"
#include "common/maya_utils.h"

using mace_maya::RemoveArrayAliases;
using mace_maya::CreateArrayAliases;

#pragma warning(disable : 4018)

const char* AceRequestSendAudioCommand::commandName = "AceRequestSendAudio";


MStatus AceRequestSendAudioCommand::doIt(const MArgList& args) {
    MStatus status;

    // Check for the correct number of arguments
    if (args.length() != 1) {
        MGlobal::displayError("Usage: AceRequestSendAudio <nodeName>");
        return MS::kFailure;
    }

    // Get the node name from the arguments
    MString node_name = args.asString(0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Find the node by name
    MSelectionList sel_list;
    sel_list.add(node_name);
    MObject node_obj;
    sel_list.getDependNode(0, node_obj);

    // Ensure the node is of the correct type
    if (!node_obj.hasFn(MFn::kDependencyNode)) {
        MGlobal::displayError("Selected object is not a valid node.");
        return MS::kFailure;
    }

    // Access the node's attribute plug by name
    MFnDependencyNode node_fn(node_obj, &status);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to create MFnDependencyNode.");
        return MS::kFailure;
    }

    MTypeId nodeId = node_fn.typeId(&status);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Cannot retreive node type id.");
        return MS::kFailure;
    }
    if (nodeId != AceAnimationPlayer::id) {
        MGlobal::displayError("Requires an AceAnimationPlayer node as input.");
        return MS::kFailure;
    }

    MPlug plug_request = node_fn.findPlug(AceAnimationPlayer::triggerSendAudio, true, &status);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to find a plug.");
        return MS::kFailure;
    }

    // force triggering request
    // NOTE: DGModifier did not work here.
    status = MGlobal::executeCommand("dgdirty " + plug_request.name());
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to mark the plug dirty.");
        return MS::kFailure;
    }
    status = MGlobal::executeCommand("dgeval " + plug_request.name());
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to evaluate the plug.");
        return MS::kFailure;
    }

    // update output blendshape names
    MPlug plug_bsnames = node_fn.findPlug(AceAnimationPlayer::outputWeightNames, true, &status);
    MGlobal::executeCommand("dgdirty " + plug_bsnames.name());
    MGlobal::executeCommand("dgeval " + plug_bsnames.name());

    // force updating outputs
    MPlug plug_outweights = node_fn.findPlug(AceAnimationPlayer::outputWeights, true, &status);
    MGlobal::executeCommand("dgdirty " + plug_outweights.name());
    MGlobal::executeCommand("dgeval " + plug_outweights.name());

    // update output emotion state names
    MPlug plug_esnames = node_fn.findPlug(AceAnimationPlayer::outputEmotionNames, true, &status);
    MGlobal::executeCommand("dgdirty " + plug_esnames.name());
    MGlobal::executeCommand("dgeval " + plug_esnames.name());

    // force updating output emotion state
    MPlug plug_outemotionstate = node_fn.findPlug(AceAnimationPlayer::outputEmotions, true, &status);
    MGlobal::executeCommand("dgdirty " + plug_outemotionstate.name());
    MGlobal::executeCommand("dgeval " + plug_outemotionstate.name());

    // remove existing aliases
    status = RemoveArrayAliases(plug_outweights, node_obj);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to remove array aliases for output blendshape weights.");
        return status;
    }
    status = RemoveArrayAliases(plug_outemotionstate, node_obj);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to remove array aliases for output emotion state.");
        return status;
    }

    // set alias to the output blendshape weights
    // TODO: move this into the node
    std::vector<std::string> bs_names;
    for (int i = 0; i < plug_bsnames.numElements(); i++) {
        MPlug plug_name1 = plug_bsnames.elementByLogicalIndex(i);
        bs_names.push_back(plug_name1.asString().asChar());
    }
    status = CreateArrayAliases(plug_outweights, node_obj, bs_names, "out_");
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to create array aliases for output blendshape weights.");
        return status;
    }

    // set alias to the output emotion state
    // TODO: move this into the node
    std::vector<std::string> es_names;
    for (int i = 0; i < plug_esnames.numElements(); i++) {
        MPlug plug_name1 = plug_esnames.elementByLogicalIndex(i);
        es_names.push_back(plug_name1.asString().asChar());
    }
    status = CreateArrayAliases(plug_outemotionstate, node_obj, es_names, "out_");

    // update viewport and other maya components
    MGlobal::executeCommand("currentTime `currentTime -q`;");
    return MS::kSuccess;
}
