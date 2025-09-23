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
#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include <maya/MDataBlock.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MPlug.h>


#define A2F_CHECK(test) \
    if (!(test)) { return {}; }
#define A2F_CHECK_WARNING_RETURN(test, warning, returnValue) \
    if (!(test)) { MGlobal::displayWarning(warning); return returnValue; }
#define A2F_CHECK_WARNING(test, warning) \
    A2F_CHECK_WARNING_RETURN(test, warning, {})
#define A2F_CHECK_WARNING_FAILURE(test, warning) \
    A2F_CHECK_WARNING_RETURN(test, warning, MStatus::kFailure)

namespace mace_maya{

// Helpers to deal with array attributes.
MStatus CreateArrayAliases(
    const MObject& arrayAttr, 
    const MObject& node,
    const std::vector<std::string>& aliases, 
    const std::string& aliasPrefix=""
);
MStatus CreateArrayAliases(
    const MPlug& plug, 
    const MObject& node,
    const std::vector<std::string>& aliases, 
    const std::string& aliasPrefix=""
);

MStatus RemoveArrayAliases(const MObject& arrayAttr, const MObject& node);
MStatus RemoveArrayAliases(const MPlug& plug, const MObject& node);

MStatus GetArrayInput(MDataBlock& block, const MObject& attribute, std::vector<float>& out_values);
MStatus GetArrayInput(MDataBlock& block, const MObject& attribute, std::vector<std::string>& out_values);

template <typename T> 
MStatus SetArrayOutput(MDataBlock &block, MObject &attribute, std::vector<T> &values, bool setClean=true);

template <typename T>
MStatus SetOutput(MDataBlock &block, MObject &attribute, T value, bool setClean=true);

template <typename T>
void SetData(MDataHandle &handle, T value);

MStatus SetAttributeDefault(MObject& attribute, float value);
MStatus SetAttributeDefault(MObject& attribute, int value);

std::filesystem::path GetAttributeAsPath(MDataBlock& block, MObject attr, MStatus* status=nullptr);
std::filesystem::path ResolvePath(const std::string& relativePath, MStatus* status=nullptr);

}  // namespace mace_maya
