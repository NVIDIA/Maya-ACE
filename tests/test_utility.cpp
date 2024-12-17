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
#include <string>
#include <vector>

#include "aceclient/logger.h"
#include "aceclient/utility.h"

#include <gtest/gtest.h>

#pragma warning(disable : 4305)

TEST(TestUtility, TestIsSlashOrSpace) {
    EXPECT_EQ(mace::IsSlashOrWhitespace(' '), true);
    EXPECT_EQ(mace::IsSlashOrWhitespace('/'), true);
    EXPECT_EQ(mace::IsSlashOrWhitespace('a'), false);
    EXPECT_EQ(mace::IsSlashOrWhitespace('.'), false);
    EXPECT_EQ(mace::IsSlashOrWhitespace(':'), false);
}

TEST(TestUtility, TestSanitizeString) {
    EXPECT_EQ(mace::SanitizeString(std::string(" ab")), "ab");
    EXPECT_EQ(mace::SanitizeString(std::string("ab ")), "ab");
    EXPECT_EQ(mace::SanitizeString(std::string("ab/ ")), "ab");
    EXPECT_EQ(mace::SanitizeString(std::string("/ab")), "/ab");
    EXPECT_EQ(mace::SanitizeString(std::string("ab$")), "ab$");
    EXPECT_EQ(mace::SanitizeString(std::string("http://test.io:111")), "http://test.io:111");
    EXPECT_EQ(mace::SanitizeString(std::string("http://test.io:111 ")), "http://test.io:111");
    EXPECT_EQ(mace::SanitizeString(std::string("http://test.io:111/")), "http://test.io:111");
    EXPECT_EQ(mace::SanitizeString(std::string(" http://test.io:111")), "http://test.io:111");
}

TEST(TestUtility, TestResolveValue) {
    EXPECT_EQ(mace::ResolveValue(std::string("HOME")), "HOME");
    EXPECT_NE(mace::ResolveValue(std::string("$HOME")), "ab");
}
