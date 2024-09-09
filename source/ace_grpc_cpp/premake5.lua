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

project "ace-grpc-cpp"
    kind "StaticLib"
    language "C++"

    filter {}

    dependson {}

    files { "*.cc" }


    includedirs {
        audiofile_include_path,
        absl_include_path,
        grpc_include_path,
        protobuf_include_path,
    }

    libdirs {
        absl_lib_path,
        openssl_lib_path,
        re2_lib_path,
        cares_lib_path,
        zlib_lib_path,
        grpc_lib_path,
        protobuf_lib_path,
    }

    -- Define common libraries
    local common_libs = {
        -- absl
        "absl_bad_optional_access",
        "absl_bad_variant_access",
        "absl_base",
        "absl_city",
        "absl_cord",
        "absl_cordz_handle",
        "absl_cordz_info",
        "absl_cord_internal",
        "absl_crc32c",
        "absl_crc_cord_state",
        "absl_crc_internal",
        "absl_examine_stack",
        "absl_flags_commandlineflag",
        "absl_flags_commandlineflag_internal",
        "absl_flags_config",
        "absl_flags_internal",
        "absl_flags_marshalling",
        "absl_flags_parse",
        "absl_flags_private_handle_accessor",
        "absl_flags_program_name",
        "absl_flags_reflection",
        "absl_graphcycles_internal",
        "absl_hash",
        "absl_int128",
        "absl_kernel_timeout_internal",
        "absl_log_globals",
        "absl_log_internal_check_op",
        "absl_log_internal_conditions",
        "absl_log_internal_fnmatch",
        "absl_log_internal_format",
        "absl_log_internal_globals",
        "absl_log_internal_log_sink_set",
        "absl_log_internal_message",
        "absl_log_internal_nullguard",
        "absl_log_internal_proto",
        "absl_log_sink",
        "absl_low_level_hash",
        "absl_malloc_internal",
        "absl_random_internal_platform",
        "absl_random_internal_pool_urbg",
        "absl_random_internal_randen",
        "absl_random_internal_randen_hwaes",
        "absl_random_internal_randen_hwaes_impl",
        "absl_random_internal_randen_slow",
        "absl_random_internal_seed_material",
        "absl_random_seed_gen_exception",
        "absl_raw_hash_set",
        "absl_raw_logging_internal",
        "absl_spinlock_wait",
        "absl_stacktrace",
        "absl_status",
        "absl_statusor",
        "absl_strerror",
        "absl_strings",
        "absl_strings_internal",
        "absl_string_view",
        "absl_str_format_internal",
        "absl_symbolize",
        "absl_synchronization",
        "absl_throw_delegate",
        "absl_time",
        "absl_time_zone",
        "absl_vlog_config_internal",

        -- grpc
        "address_sorting",
        "gpr",
        "grpc++",
        "grpc",
        "upb_base_lib",
        "upb_json_lib",
        "upb_mem_lib",
        "upb_message_lib",
        "upb_textformat_lib",

        -- re2
        "re2",

        -- zlib
        "zlib",

        -- c-ares
        "cares",

        -- protobuf
        "utf8_validity",
    }

    -- Link libraries based on configuration
    filter { "configurations:debug" }
        links {
            -- openssl
            "libcryptod",
            "libssld",
            -- protobuf
            "libprotobufd",
            table.unpack(common_libs)
        }
    filter { "configurations:release" }
        links {
            -- openssl
            "libcrypto",
            "libssl",
            -- protobuf
            "libprotobuf",
            table.unpack(common_libs)
        }
    filter {}

    linkoptions { "/ignore:4006" }

    targetdir("%{bin_dir}/bin")
    filter { "system:windows" }
        postbuildcommands {
            "{COPYFILE} %{root}/_build/target-deps/re2/%{config}/bin/*.dll %{bin_dir}/bin",
            "{COPYFILE} %{root}/_build/target-deps/c-ares/%{config}/bin/*.dll %{bin_dir}/bin",
        }
    filter {}
