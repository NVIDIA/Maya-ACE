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

#include "profiler.h"

#include <cassert>

int maceProfilingCategory::sCategoryIndex = -1;

namespace {

    inline constexpr char categoryName[] = "mace";
    inline constexpr char categoryInfo[] = "mace profiling";

} // Anonymous namespace.

bool maceProfilingCategory::InitializeProfiler() {
    // Can't add a category while profiling.
    if (MProfiler::recordingActive()) {
        MProfiler::setRecordingActive(false);
    }

    sCategoryIndex = MProfiler::addCategory(categoryName, categoryInfo);
    return sCategoryIndex >= 0;
}

bool maceProfilingCategory::UninitializeProfiler() {
    [[maybe_unused]] const auto categoryIndex = MProfiler::removeCategory(categoryName);
    assert(sCategoryIndex == categoryIndex);
    sCategoryIndex = 0;
    return categoryIndex >= 0;
}
