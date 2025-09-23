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

#include <maya/MProfiler.h>

class maceProfilingCategory {
public:
    static bool InitializeProfiler();
    static bool UninitializeProfiler();

    static inline int GetCategoryIndex() { return sCategoryIndex; }

private:
    static int sCategoryIndex;
};


// This profiling scope is meant to alias to no-op.
class maceProfilingScope_Empty {
public:
    inline maceProfilingScope_Empty(
        const char*,
        const MObject& = MObject::kNullObj
    )
    {
    }
};


// This profiling scope enables profiling
class maceProfilingScope_Impl {
public:
    inline maceProfilingScope_Impl(
        const char* eventName,
        const MObject& associatedNode = MObject::kNullObj
    ) : mScope(maceProfilingCategory::GetCategoryIndex(), MProfiler::kColorG_L3, eventName, nullptr, associatedNode)
    {
    }

private:
    MProfilingScope mScope;
};

// Switch to a different profiling scope by aliasing.
using maceProfilingScope = maceProfilingScope_Empty; // Set to empty by default to avoid performance impact. Please change as needed.
// using maceProfilingScope = maceProfilingScope_Impl;
