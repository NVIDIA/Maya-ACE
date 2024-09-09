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

newoption {
    trigger = "solution-name",
    description = "Solution name"
}

newoption {
    trigger = "aceclient_log_level",
    description = "aceclient log level",
    allowed = {
        { "1", "ERROR" },
        { "2", "INFO" },
        { "3", "DEBUG" },
    },
    default = "1"
}

workspace(_OPTIONS["solution-name"])
    root = path.getabsolute(".")
    host_deps = "%{root}/_build/host-deps"
    config = '%{cfg.buildcfg}'
    platform = os.target()..'-%{cfg.architecture}'
    bin_dir = '%{root}/_build/%{platform}/%{config}'

    configurations { "debug", "release" }
    location ('%{root}/_compiler')
    targetdir (bin_dir)
    objdir ("%{root}/_build/intermediate/%{platform}/%{prj.name}")

    -- include and lib path to external libs
    audiofile_include_path = "%{root}/_build/target-deps/audiofile/include"
    maya2024_include_path = "%{root}/deps/devkitBase/include"
    maya2024_lib_path = "%{root}/deps/devkitBase/lib"
    absl_include_path = "%{root}/_build/target-deps/abseil/%{config}/include"
    absl_lib_path = "%{root}/_build/target-deps/abseil/%{config}/lib"
    grpc_include_path = "%{root}/_build/target-deps/grpc/%{config}/include"
    grpc_lib_path = "%{root}/_build/target-deps/grpc/%{config}/lib"
    protobuf_include_path = "%{root}/_build/target-deps/protobuf/%{config}/include"
    protobuf_lib_path = "%{root}/_build/target-deps/protobuf/%{config}/lib"
    openssl_lib_path = "%{root}/_build/target-deps/openssl/%{config}/lib"
    re2_lib_path = "%{root}/_build/target-deps/re2/%{config}/lib"
    cares_lib_path = "%{root}/_build/target-deps/c-ares/%{config}/lib"
    zlib_lib_path = "%{root}/_build/target-deps/zlib/%{config}/lib"
    gtest_include_path = "%{root}/_build/target-deps/gtest/%{config}/include"
    gtest_lib_path = "%{root}/_build/target-deps/gtest/%{config}/lib"
    ace_client_source_path = "%{root}/source/"
    source_path = "%{root}/source"

    cppdialect "C++17"
    flags { "MultiProcessorCompile", "NoPCH", "UndefinedIdentifiers", "NoIncrementalLink" }
    platforms { "x86_64" }
    architecture "x86_64"

    -- Debug configuration settings
    filter { "configurations:debug" }
        defines { "DEBUG" }
        optimize "Off"
        symbols "On"

    -- Release configuration settings
    filter { "configurations:release" }
        defines { "NDEBUG" }
        optimize "Speed"
        symbols "Off"

    filter {}

    filter { "system:windows" }
        buildoptions { "/wd4514 /wd4571 /wd4625 /wd4626 \
        /wd4668 /wd4710 /wd4711 /wd4774 \
        /wd4820 /wd5026 /wd5027 /wd5039 \
        /wd5045 /wd4464 /wd4365 /wd4324 \
        /wd4201 /wd4515 /wd4061 /wd4296 \
        /wd4458 /wd5031 /wd4505 /wd4127 /wd4388 /wd4623 /wd4686 /wd4868 /wd4267 \
        /wd4996 \
        /W3 /EHsc /nologo" }

        linkoptions {
            "/NODEFAULTLIB:LIBCMT",
            "/ignore:4099"
        }

        -- TEMP: modify flags to avoid error with repo-build
        removeflags { "NoIncrementalLink" }
        debugformat "c7"
        symbols "On"
        buildoptions {
            -- Macro parameters warning that only occurs when using ccache, which
            -- adds the `/E` parameter to cl.exe to preprocess only. The preprocessor
            -- fails when hitting this warning, resulting in Uncacheable ccache misses.
            "/wd4003",
        }

    filter  { "system:windows", "configurations:release" }
        buildoptions { "/MD" }
    filter  { "system:windows", "configurations:debug" }
        buildoptions { "/MDd" }

    filter { "system:linux" }
        pic "On"
        enablewarnings { "all" }
        linkoptions {
            "-Wl,-rpath,'$$ORIGIN'",
            "-Wl,--disable-new-dtags"
        }
    filter {}

    -- macro
    defines {
        "_GLIBCXX_USE_CXX11_ABI=0", -- OM-81991
        "ACE_CLIENT_LOG_LEVEL=" .. _OPTIONS["aceclient_log_level"],
    }


    -- gRPC-generated library
    dofile("source/ace_grpc_cpp/premake5.lua")

    --- aceclient
    dofile("source/aceclient/premake5.lua")

    --- mace and maya_aceclient
    dofile("source/maya_aceclient/premake5.lua")
    dofile("source/premake5.lua")

    --- test-aceclient
    dofile("tests/premake5.lua")
