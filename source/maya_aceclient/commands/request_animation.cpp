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
#include "request_animation.h"

#include <string>

#include "aceclient/aceclient.h"
#include "nodes/animation_player.h"

#pragma warning(disable : 4018)

const char* AceRequestAnimationCommand::commandName = "AceRequestAnimation";


MStatus AceRequestAnimationCommand::doIt(const MArgList& args) {
    MStatus status;

    // Check for the correct number of arguments
    if (args.length() != 1) {
        MGlobal::displayError("Usage: AceRequestAnimation <nodeName>");
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

    MPlug plug_request = node_fn.findPlug(AceAnimationPlayer::triggerRequest, true, &status);
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
    MPlug plug_bsnames = node_fn.findPlug(AceAnimationPlayer::outputBlendshapeNames, true, &status);
    MGlobal::executeCommand("dgdirty " + plug_bsnames.name());
    MGlobal::executeCommand("dgeval " + plug_bsnames.name());

    // force updating outputs
    MPlug plug_outweights = node_fn.findPlug(AceAnimationPlayer::outputWeights, true, &status);
    MGlobal::executeCommand("dgdirty " + plug_outweights.name());
    MGlobal::executeCommand("dgeval " + plug_outweights.name());

    // update output emotion state names
    MPlug plug_esnames = node_fn.findPlug(AceAnimationPlayer::outputEmotionStateNames, true, &status);
    MGlobal::executeCommand("dgdirty " + plug_esnames.name());
    MGlobal::executeCommand("dgeval " + plug_esnames.name());

    // force updating output emotion state
    MPlug plug_outemotionstate = node_fn.findPlug(AceAnimationPlayer::outputEmotionState, true, &status);
    MGlobal::executeCommand("dgdirty " + plug_outemotionstate.name());
    MGlobal::executeCommand("dgeval " + plug_outemotionstate.name());

    // set alias to the output blendshape weights
    MString py_script = "import ace_tools;ace_tools.remove_attr_aliases('" + node_name + "');";
    MGlobal::executePythonCommand(py_script);
    for (int i = 0; i < plug_bsnames.numElements(); i++) {
        MPlug plug_name1 = plug_bsnames.elementByLogicalIndex(i);
        MGlobal::executeCommand(
            "aliasAttr out_" + plug_name1.asString() + " " + plug_outweights.name() + "[" + i + "]");
    }
    for (int i = 0; i < plug_esnames.numElements(); i++) {
        MPlug plug_name1 = plug_esnames.elementByLogicalIndex(i);
        MGlobal::executeCommand(
            "aliasAttr out_" + plug_name1.asString() + " " + plug_outemotionstate.name() + "[" + i + "]");
    }

    return MS::kSuccess;
}
