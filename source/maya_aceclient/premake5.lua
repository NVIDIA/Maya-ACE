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

project "maya_aceclient_shared"
    kind "Utility"

    filter {}

    dependson {
        "aceclient",
    }

    postbuildcommands {
        "{COPYFILE} %{bin_dir}/bin/*.lib %{bin_dir}/plugins/mace/bin",
        "{COPYFILE} %{bin_dir}/bin/*.dll %{bin_dir}/plugins/mace/bin",
    }
    filter { "system:windows" }
        bin_dir_win = string.gsub(bin_dir, "%%{root}", root)
        plugin_folder = string.gsub(bin_dir_win.."/plugins/mace/bin", "/", "\\")
        scripts_folder = string.gsub(bin_dir_win.."/plugins/mace/scripts", "/", "\\")
        src = string.gsub(root.."/source/maya_aceclient/scripts", "/", "\\")
        prebuildcommands {
            "if not exist "..plugin_folder.." mkdir "..plugin_folder,
            "if exist " ..scripts_folder .. " del " ..scripts_folder, --ignore error if file is not there
            "{COPY} %{src} %{scripts_folder}"
        }
    filter {}

project "maya_aceclient_2024"
    kind "SharedLib"
    language "C++"

    filter {}

    dependson {
        "maya_aceclient_shared",
    }

    files {
        "*.cpp",
        "common/*.cpp",
        "nodes/*.cpp",
        "commands/*.cpp",
    }

    includedirs {
        maya2024_include_path,
        absl_include_path,
        grpc_include_path,
        protobuf_include_path,
        source_path,
        ".",
    }

    libdirs {
        maya2024_lib_path,
    }

    links {
        "aceclient",
        "Foundation",
        "OpenMaya",
        "OpenMayaAnim",
    }

    targetdir("%{bin_dir}/plugins/mace/plug-ins/2024")
    targetname("maya_aceclient")

    filter { "system:windows" }
        bin_dir_win = string.gsub(bin_dir, "%%{root}", root)
        bin_dir_win = string.gsub(bin_dir_win, "/", "\\")

        mll = bin_dir_win .. "\\plugins\\mace\\plug-ins\\2024\\maya_aceclient.mll"
        dll = bin_dir_win .. "\\plugins\\mace\\plug-ins\\2024\\maya_aceclient.dll"

        postbuildcommands {
            "if exist " ..mll .. " del " ..mll, --ignore error if file is not there
            "mklink /H " .. mll .. " ".. dll
        }
    filter {}

