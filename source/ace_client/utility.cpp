// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#include <algorithm>
#include <map>
#include <string>

#include "utility.h"

namespace mace
{

std::string ResolveValue(std::string &key_or_value) {
    if (key_or_value.rfind("$", 0) == 0) {
        // read key from the environment variable
        std::string key = key_or_value.substr(1);
        auto val = std::getenv(key.c_str());
        if (val != NULL) {
            return std::string(val);
        }
        else {
            return std::string("");
        }
    }
    return key_or_value;
}

std::string SanitizeString(std::string &str) {
    std::string::iterator start = std::find_if_not(
        str.begin(),
        str.end(),
        [](char ch){return std::isspace(static_cast<unsigned char>(ch));}
    );
    if (start == str.end()) {
        return "";
    }

    std::string::iterator end = std::find_if_not(str.rbegin(), str.rend(), IsSlashOrWhitespace).base();

    return std::string(start, end);
}

bool IsSlashOrWhitespace(char ch) {
    return std::isspace(static_cast<unsigned char>(ch)) || ch == '/';
}

} // namespace mace
