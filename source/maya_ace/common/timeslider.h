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

#include <maya/MTypes.h>

#ifndef ENABLE_TIME_SLIDER

// Backward compatibility with older versions of Maya without the time slider API.
#if MAYA_API_VERSION >= 20230000
    // Time slider API is available since Maya 2023.
    // https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=MAYA_API_REF_cpp_ref_changelog_2023_0_html
    #if __has_include(<maya/MTimeSliderCustomDrawManager.h>)
        #include <maya/MTimeSliderCustomDrawManager.h>
        #define ENABLE_TIME_SLIDER 1
    #else
        #pragma message("Warning: Time slider API is available since Maya 2023. But <maya/MTimeSliderCustomDrawManager.h> is not found!")
        #define ENABLE_TIME_SLIDER 0
    #endif
#else
    #define ENABLE_TIME_SLIDER 0
#endif

#endif

namespace TimeSliderProgress {

#if ENABLE_TIME_SLIDER == 1
using CustomDrawID = MTimeSliderCustomDrawManager::MCustomDrawID;
#else
using CustomDrawID = int; // dummy type if time slider is disabled. The functions below will be no-op.
#endif

const CustomDrawID kInvalidContext = -1;

CustomDrawID registerCustomDraw();

void deregisterCustomDraw(CustomDrawID ctx);

void draw(CustomDrawID ctx, double seconds, bool isPending=false, bool done=false);

void show(CustomDrawID ctx);

void hide();

void clean();

} // namespace TimeSliderProgress
