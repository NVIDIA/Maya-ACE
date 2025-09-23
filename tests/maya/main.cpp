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

#include <gtest/gtest.h>
#include <maya/MLibrary.h>
#include <maya/MStatus.h>
#include <maya/MGlobal.h>
#include <iostream>

int main(int argc, char *argv[]) {
    // Initialize Maya in standalone mode
    MStatus status = MLibrary::initialize("maya_tests", true);
    if (!status) {
        std::cerr << "Failed to initialize Maya standalone: " << status.errorString() << std::endl;
        // Continue anyway - some tests might not need Maya
    }
    
    // Initialize GoogleTest
    ::testing::InitGoogleTest(&argc, argv);
    
    // Run all tests
    int ret = RUN_ALL_TESTS();
    
    // Cleanup Maya
    if (status) {
        MLibrary::cleanup(0);
    }
    
    return ret;
} 
