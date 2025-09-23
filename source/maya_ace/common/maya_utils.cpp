// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#include "maya_utils.h"

#include <sstream>
#include <vector>
#include <string>

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnAttribute.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MString.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>

namespace mace_maya {

std::filesystem::path GetAttributeAsPath(MDataBlock& block, MObject attr, MStatus* status) {
    MStatus status_ = status ? *status : MS::kSuccess;
    MDataHandle pathHandle = block.inputValue(attr, &status_);
    if (status_ != MS::kSuccess) { return std::filesystem::path(); }

    std::string pathString = pathHandle.asString().asUTF8();
    if (pathString.empty()) { return std::filesystem::path(); }

    std::filesystem::path fullpath = ResolvePath(pathString, &status_);
    return fullpath;
}

std::filesystem::path ResolvePath(const std::string& relativePath, MStatus* status) {
    MStatus status_ = status ? *status : MS::kSuccess;
    // If the path is already absolute, return it as is
    std::filesystem::path path(relativePath);
    if (path.is_absolute()) {
        return path;
    }
    
    // Get the current Maya project directory
    MString projectDir;
    status_ = MGlobal::executeCommand("workspace -q -rootDirectory", projectDir);
    if (status_ != MS::kSuccess) {
        MGlobal::displayWarning("Failed to get Maya project directory. Using relative path as is.");
        return path;
    }
    
    // Convert Maya string to std::string
    std::string projectDirStr = projectDir.asUTF8();
    
    // Resolve the relative path against the project directory
    std::filesystem::path projectPath(projectDirStr);
    std::filesystem::path absolutePath = projectPath / path;
    
    // Normalize the path to handle things like ".." and "."
    absolutePath = absolutePath.lexically_normal();
    
    return absolutePath;
}

MStatus RemoveArrayAliases(const MObject& arrayAttr, const MObject& node) {
    MStatus status;
    MFnDependencyNode fnNode(node, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MStringArray aliases;
    fnNode.getAliasList(aliases, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFnAttribute fnAttr(arrayAttr, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MString arrayAttrName = fnAttr.name(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MPlug arrayPlug = MPlug(node, arrayAttr);

    for (unsigned int i = 0; i < aliases.length(); i += 2) {
        // NOTE: Rudimentary parsing, could be made more robust.
        // Checking if the alias is to "arrayAttrName[i]" type of attribute.
        const int startPos = aliases[i+1].index('[');
        const int endPos = aliases[i+1].index(']');
        if (startPos <= 0 || endPos <= startPos) {
            // not array
            continue;
        }
        if (aliases[i+1].substring(0, startPos - 1) != arrayAttrName) {
            // not target attribute
            continue;
        }
        const auto indexString = aliases[i+1].substring(startPos + 1, endPos - 1);
        if (!indexString.isUnsigned()) {
            // something is wrong with the index
            continue;
        }
        const unsigned int index = indexString.asUnsigned();
        MPlug elementPlug = arrayPlug.elementByLogicalIndex(index, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        fnNode.setAlias(
            aliases[i], aliases[i+1], elementPlug, false, &status // remove
        );
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    return MStatus::kSuccess;
}

MStatus RemoveArrayAliases(const MPlug& plug, const MObject& node) {
    return RemoveArrayAliases(plug.attribute(), node);
}

MStatus CreateArrayAliases(
    const MObject& arrayAttr, 
    const MObject& node, 
    const std::vector<std::string>& aliases, 
    const std::string& aliasPrefix
) 
{
    MStatus status;
    MFnDependencyNode fnNode(node, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFnAttribute fnAttr(arrayAttr, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MString arrayAttrName = fnAttr.name(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MPlug arrayPlug(node, arrayAttr);
    for (std::size_t i = 0; i < aliases.size(); ++i) {
        MPlug elementPlug = arrayPlug.elementByLogicalIndex(i, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        std::ostringstream elementName;
        elementName << arrayAttrName << "[" << i << "]";
        std::ostringstream aliasName;
        aliasName << aliasPrefix << aliases[i];

        fnNode.setAlias(
            aliasName.str().c_str(), 
            elementName.str().c_str(), 
            elementPlug, 
            true, // add
            &status
        );
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    return MStatus::kSuccess;
}

MStatus CreateArrayAliases(
    const MPlug& plug, 
    const MObject& node, 
    const std::vector<std::string>& aliases, 
    const std::string& aliasPrefix
)
{
    return CreateArrayAliases(plug.attribute(), node, aliases, aliasPrefix);
}

MStatus GetArrayInput(
    MDataBlock& block, const MObject& attribute, std::vector<float>& out_values) 
{
    MStatus status;
    MArrayDataHandle inputArrayHandle = block.inputArrayValue(attribute, &status);
    do {
        MDataHandle element = inputArrayHandle.inputValue(&status);
        if(status != MS::kSuccess) break;
        out_values.push_back(element.asFloat());

    } while (inputArrayHandle.next() == MS::kSuccess);
    return MS::kSuccess;
}

MStatus GetArrayInput(
    MDataBlock& block, const MObject& attribute, std::vector<std::string>& out_values) 
{
    MStatus status;
    MArrayDataHandle inputArrayHandle = block.inputArrayValue(attribute, &status);
    do {
        MDataHandle element = inputArrayHandle.inputValue(&status);
        if(status != MS::kSuccess) break;
        out_values.push_back(element.asString().asChar());
    } while (inputArrayHandle.next() == MS::kSuccess);
    return MS::kSuccess;
}

template <typename T>
MStatus SetArrayOutput(
    MDataBlock &block, MObject &attribute, std::vector<T> &values, bool setClean)
{
    MStatus return_status = MS::kFailure;
    MArrayDataHandle array_handle = block.outputArrayValue(attribute, &return_status);

    if (return_status != MS::kSuccess) {
        return return_status;
    }

    MArrayDataBuilder array_builder = array_handle.builder();

    // reduce size of the output array if needed
    unsigned int cur_size = array_handle.elementCount();
    unsigned int new_size = values.size();
    if (cur_size > new_size) {
        array_handle.jumpToArrayElement(0);
        for (cur_size; new_size < cur_size; cur_size--) {
            array_builder.removeElement(cur_size - 1);
        }
    }

    // add/update elements
    int i = 0;
    for (auto value : values) {
        MDataHandle data_handle = array_builder.addElement(i++, &return_status);
        SetData(data_handle, value);
    }
    array_handle.set(array_builder);

    if (setClean) {
        array_handle.setAllClean();
        block.setClean(attribute);
    }
    return MS::kSuccess;
}

template <typename T>
MStatus SetOutput(
    MDataBlock &block, MObject &attribute, T value, bool setClean)
{
    MStatus return_status = MS::kFailure;
    MDataHandle handle = block.outputValue(attribute, &return_status);

    if (return_status != MS::kSuccess) {
        return return_status;
    }

    SetData(handle, value);

    if (setClean) {
        block.setClean(attribute);
    }
    return MS::kSuccess;
}

template <typename T>
void SetData(MDataHandle &handle, T value) {
    if constexpr (std::is_same<T, std::string>::value) {
        handle.setString(MString(value.c_str()));
    }
    else if constexpr (std::is_same<T, char *>::value) {
        handle.setString(MString(value));
    }
    else if constexpr (std::is_same<T, size_t>::value) {
        handle.set(int(value));
    }
    else {
        handle.set(value);
    }
}

MStatus SetAttributeDefault(MObject& attribute, float value) {
    // only change the default value and not touching the current value
    MFnNumericAttribute nAttr(attribute);
    nAttr.setDefault(value);
    return MS::kSuccess;
}

MStatus SetAttributeDefault(MObject& attribute, int value) {
    // only change the default value and not touching the current value
    MFnNumericAttribute nAttr(attribute);
    nAttr.setDefault(value);
    return MS::kSuccess;
}

// Explicit template instantiations to fix linker errors
template MStatus SetOutput<int>(
    MDataBlock &block, MObject &attribute, int value, bool setClean);
template MStatus SetOutput<float>(
    MDataBlock &block, MObject &attribute, float value, bool setClean);
template MStatus SetOutput<double>(
    MDataBlock &block, MObject &attribute, double value, bool setClean);
template MStatus SetOutput<bool>(
    MDataBlock &block, MObject &attribute, bool value, bool setClean);
template MStatus SetOutput<size_t>(
    MDataBlock &block, MObject &attribute, size_t value, bool setClean);
template MStatus SetOutput<std::string>(
    MDataBlock &block, MObject &attribute, std::string value, bool setClean);
template MStatus SetOutput<MString>(
    MDataBlock &block, MObject &attribute, MString value, bool setClean);

template MStatus SetArrayOutput<int>(
    MDataBlock &block, MObject &attribute, std::vector<int> &values, bool setClean);
template MStatus SetArrayOutput<float>(
    MDataBlock &block, MObject &attribute, std::vector<float> &values, bool setClean);
template MStatus SetArrayOutput<double>(
    MDataBlock &block, MObject &attribute, std::vector<double> &values, bool setClean);
template MStatus SetArrayOutput<bool>(
    MDataBlock &block, MObject &attribute, std::vector<bool> &values, bool setClean);
template MStatus SetArrayOutput<std::string>(
    MDataBlock &block, MObject &attribute, std::vector<std::string> &values, bool setClean);
template MStatus SetArrayOutput<MString>(
    MDataBlock &block, MObject &attribute, std::vector<MString> &values, bool setClean);

template void SetData<int>(MDataHandle &handle, int value);
template void SetData<float>(MDataHandle &handle, float value);
template void SetData<double>(MDataHandle &handle, double value);
template void SetData<bool>(MDataHandle &handle, bool value);
template void SetData<size_t>(MDataHandle &handle, size_t value);
template void SetData<std::string>(MDataHandle &handle, std::string value);
template void SetData<MString>(MDataHandle &handle, MString value);

}  // namespace mace_maya
