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
#include "export_config_parameters.h"

#include <string>

#include "nodes/animation_player.h"


const char* AceExportConfigParametersCommand::commandName = "AceExportConfigParameters";

MStatus AceExportConfigParametersCommand::doIt(const MArgList& args) {
    MStatus status;

    // Check for the correct number of arguments
    if (args.length() != 2) {
        MGlobal::displayError("Usage: AceExportConfigParameters <nodeName> <path>");
        return MS::kFailure;
    }

    // Parse args
    MString node_name = args.asString(0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MString export_fpath = args.asString(1, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Forward the args to python since it's easier to handle json and string manipulation there.
    MString pythonCmd = "import ace_config;";
    pythonCmd += "ace_config.ace_export_config_parameters(\"";
    pythonCmd += node_name + "\", r\"";
    pythonCmd += export_fpath + "\")";

    MGlobal::executePythonCommand(pythonCmd);
    return MS::kSuccess;
}
