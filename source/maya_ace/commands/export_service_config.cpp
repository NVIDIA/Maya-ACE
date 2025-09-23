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
#include "export_service_config.h"

#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>

#include <maya/MArgDatabase.h>
#include <maya/MArgParser.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MGlobal.h>
#include <maya/MStringArray.h>

#include "ace_client/logger.h"
#include "ace_client/status.h"
#include "ace_client/a2x_config_client.h"

#include "nodes/ace_player.h"

#pragma warning(disable : 4018)

const char* AceExportServiceConfigCommand::commandName = "AceExportServiceConfig";


MStatus AceExportServiceConfigCommand::doIt(const MArgList& args) {
    /* Read config data from an ACE service.
    */
    MStatus status;

    // Parse arguments
    MArgDatabase argData(syntax(), args, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    if (argData.isFlagSet("filePrefix")) {
        argData.getFlagArgument("filePrefix", 0, filePrefix);
    }
    if (filePrefix.length() < 3) {
        MGlobal::displayError("Invalid File Path: '" + filePrefix + "'");
        return MS::kFailure;
    }

    mace::ConfigType request_config_type;
    if (argData.isFlagSet("configType")) {
        argData.getFlagArgument("configType", 0, configType);
    }
    MString configTypeUpper = configType.toUpperCase();
    if (configTypeUpper == "JSON") {
        request_config_type = mace::ConfigType::JSON;
    } else if (configTypeUpper == "YAML") {
        request_config_type = mace::ConfigType::YAML;
    } else {
        MGlobal::displayError("Config Type is not supported: '" + configType + "'");
        return MS::kFailure;
    }

    if (argData.isFlagSet("address")) {
        argData.getFlagArgument("address", 0, address);
    }

    if (argData.isFlagSet("apiKey")) {
        argData.getFlagArgument("apiKey", 0, apiKey);
    }

    if (argData.isFlagSet("functionId")) {
        argData.getFlagArgument("functionId", 0, functionId);
    }

    std::vector<mace::ConfigContent> configs;

    LOG_DEBUG("Getting configs from " << address.asChar());
    // Check if apiKey starts with $ and read from environment if so
    if (apiKey.length() > 0 && apiKey.asChar()[0] == '$') {
        // Get environment variable name by removing the $
        MString envVarName = apiKey.substring(1, apiKey.length()-1);
        const char* envValue = std::getenv(envVarName.asChar());
        if (envValue == nullptr) {
            MGlobal::displayError("Environment variable not found: " + envVarName);
            return MS::kFailure;
        }
        apiKey = envValue;
    }
    auto result = mace::GetA2XConfig(
        request_config_type,
        &configs,
        address.asChar(),
        apiKey.asChar(),
        functionId.asChar()
    );

    if (result.status != AceClientStatus::OK) {
        MGlobal::displayError(result.message.c_str());
        return MS::kFailure;
    }

    MGlobal::displayInfo(MString("Acquired config files: ") + configs.size());
    MStringArray  config_names;
    for (auto config: configs) {
        config_names.append(config.name.c_str());
        std::string file_path = filePrefix.asChar();
        if (std::filesystem::is_directory(file_path)) {
            file_path += std::filesystem::path::preferred_separator;
        }
        file_path += config.name;
        CHECK_MSTATUS_AND_RETURN_IT(WriteFile(file_path, config.content));
    }

    setResult(config_names);

    return MS::kSuccess;
}

MSyntax AceExportServiceConfigCommand::newSyntax() {
    MSyntax syntax;
    syntax.addFlag("a", "address", MSyntax::kString);
    syntax.addFlag("k", "apiKey", MSyntax::kString);
    syntax.addFlag("id", "functionId", MSyntax::kString);
    syntax.addFlag("t", "configType", MSyntax::kString);
    syntax.addFlag("f", "filePrefix", MSyntax::kString);
    return syntax;
}

MStatus AceExportServiceConfigCommand::WriteFile(
    std::string const file_path, std::string const content)
{
    std::ofstream out_file(file_path);

    // Check if the file was opened successfully
    if (out_file.is_open()) {
        // Write text to the file
        out_file << content << std::endl;

        // Close the file
        out_file.close();
    } else {
        MString msg = MString("Unable to open file for writing: ");
        MGlobal::displayError(msg + file_path.c_str());
        return MS::kFailure;
    }

    return MS::kSuccess;
}
