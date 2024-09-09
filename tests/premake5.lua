-- SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
-- SPDX-License-Identifier: MIT

-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:

-- The above copyright notice and this permission notice shall be included in all
-- copies or substantial portions of the Software.

-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
-- SOFTWARE.

project "tests-aceclient"
    kind "ConsoleApp"
    language "C++"

    filter {}

    dependson {
        "aceclient"
    }

    files {
        "./*.cpp",
        "./*.hpp",
        "./*.h",
    }

    includedirs {
        source_path,
        gtest_include_path,
        absl_include_path,
        grpc_include_path,
        protobuf_include_path,
        audiofile_include_path,
    }

    filter { "system:windows" }
        buildoptions {
            "/wd5219 /wd4574"
        }
    filter {}

    libdirs {
        gtest_lib_path
    }

    links {
        "gtest",
        "gmock",
        "aceclient",
    }

    targetname("tests-aceclient")
    targetdir("%{bin_dir}/bin")
