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
#include <gmock/gmock.h>
#include <filesystem>

#include "maya_ace/common/maya_utils.h"

// ==========================
// Maya Utils Tests
// ==========================

class MayaUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }
};

// ==========================
// ResolvePath Tests
// ==========================

TEST_F(MayaUtilsTest, ResolvePath_AbsolutePath_ReturnsAsIs) {
    // Test with absolute paths on different platforms
#ifdef _WIN32
    std::string absolutePath = "C:\\Users\\test\\file.txt";
#else
    std::string absolutePath = "/home/test/file.txt";
#endif
    
    auto result = mace_maya::ResolvePath(absolutePath);
    EXPECT_EQ(result.string(), absolutePath);
}

TEST_F(MayaUtilsTest, ResolvePath_RelativePath_ResolvesAgainstProjectDir) {
    std::string relativePath = "scenes/test.ma";
    auto result = mace_maya::ResolvePath(relativePath);
    
    // Since we can't control the Maya project directory in tests,
    // we just verify that the result is an absolute path
    EXPECT_TRUE(result.is_absolute());
    EXPECT_TRUE(result.string().find("scenes/test.ma") != std::string::npos ||
                result.string().find("scenes\\test.ma") != std::string::npos);
}

TEST_F(MayaUtilsTest, ResolvePath_RelativePathWithDotDot_NormalizesPath) {
    std::string relativePath = "../data/audio.wav";
    auto result = mace_maya::ResolvePath(relativePath);
    
    // Verify the path is normalized (no ".." in the final path)
    EXPECT_TRUE(result.is_absolute());
    EXPECT_EQ(result.string().find(".."), std::string::npos);
}

TEST_F(MayaUtilsTest, ResolvePath_RelativePathWithDot_NormalizesPath) {
    std::string relativePath = "./scenes/./test.ma";
    auto result = mace_maya::ResolvePath(relativePath);
    
    // Verify the path is normalized (no "." in the final path except file extensions)
    EXPECT_TRUE(result.is_absolute());
    std::string pathStr = result.string();
    // Check that there's no "./" or "/." in the path (but allow ".ma" extension)
    EXPECT_EQ(pathStr.find("./"), std::string::npos);
    EXPECT_EQ(pathStr.find("/."), std::string::npos);
}

TEST_F(MayaUtilsTest, ResolvePath_EmptyPath_ReturnsProjectDir) {
    std::string emptyPath = "";
    auto result = mace_maya::ResolvePath(emptyPath);
    
    // Should return the project directory
    EXPECT_TRUE(result.is_absolute());
    EXPECT_FALSE(result.empty());
}

// ==========================
// Platform-specific Path Tests
// ==========================

TEST_F(MayaUtilsTest, ResolvePath_CrossPlatformPaths) {
    // Test that paths are handled correctly on the current platform
    std::filesystem::path testPath = std::filesystem::current_path() / "test" / "file.txt";
    std::string absolutePath = testPath.string();
    
    auto result = mace_maya::ResolvePath(absolutePath);
    EXPECT_EQ(result.string(), absolutePath);
}

// ==========================
// Integration Test Ideas
// ==========================
// Note: These tests would require specific Maya environment setup
// They are commented out but show how to test with real Maya API

/*
TEST_F(MayaUtilsTest, Integration_WithRealMayaProject) {
    // This test would require Maya to be running
    // and would test with actual Maya project settings
    
    // Set up a Maya project
    MGlobal::executeCommand("workspace -create /tmp/test_project");
    MGlobal::executeCommand("workspace -openWorkspace /tmp/test_project");
    
    // Test path resolution
    auto result = mace_maya::ResolvePath("scenes/test.ma");
    EXPECT_EQ(result.string(), "/tmp/test_project/scenes/test.ma");
}

TEST_F(MayaUtilsTest, GetAttributeAsPath_ValidAttribute_ReturnsResolvedPath) {
    // This would require setting up Maya nodes and attributes
    // Example structure:
    // 1. Create a node with a string attribute
    // 2. Set the attribute to a relative path
    // 3. Get the attribute using GetAttributeAsPath
    // 4. Verify it returns the resolved absolute path
}
*/ 
