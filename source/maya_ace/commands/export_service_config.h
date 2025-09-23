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

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MSyntax.h>


struct ConnectionInfo {
    MString address;
    MString apikey;
    MString functionid;
};

class AceExportServiceConfigCommand : public MPxCommand {
    /* Command to download configs from ACE A2X services.
    */

public:
    AceExportServiceConfigCommand() {}
    virtual ~AceExportServiceConfigCommand() {}
    static void* creator() { return new AceExportServiceConfigCommand(); }
    static MSyntax newSyntax();

    MStatus doIt(const MArgList& args) override;

    static const char* commandName;

protected:
    MStatus GetConnectionInfo(MString const node_name, ConnectionInfo &out_info);
    MStatus WriteFile(std::string const file_path, std::string const content);

    MString configType = "YAML";  // default to YAML. Also support JSON.
    MString filePrefix = "";
    MString address = "";
    MString apiKey = "";
    MString functionId = "";
};
