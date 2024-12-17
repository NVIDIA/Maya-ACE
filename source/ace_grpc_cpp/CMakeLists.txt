
set(TARGET_NAME ace-grpc-cpp)

file(GLOB SRC "*.cc")
add_library(${TARGET_NAME} STATIC ${SRC})

target_include_directories(
    ${TARGET_NAME}
    PRIVATE
    ${ABSL_INCLUDE_DIRS}
    ${AUDIOFILE_INCLUDE_DIRS}
    ${CARES_INCLUDE_DIRS}
    ${GRPC_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${RE2_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
)

target_link_directories(
    ${TARGET_NAME}
    PRIVATE
    ${ABSL_LIBRARIES}
    ${CARES_LIBRARIES}
    ${GRPC_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
    ${RE2_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

set(COMMON_LIBS
    # absl
    "absl_bad_optional_access"
    "absl_bad_variant_access"
    "absl_base"
    "absl_city"
    "absl_cord"
    "absl_cordz_handle"
    "absl_cordz_info"
    "absl_cord_internal"
    "absl_crc32c"
    "absl_crc_cord_state"
    "absl_crc_internal"
    "absl_examine_stack"
    "absl_flags_commandlineflag"
    "absl_flags_commandlineflag_internal"
    "absl_flags_config"
    "absl_flags_internal"
    "absl_flags_marshalling"
    "absl_flags_parse"
    "absl_flags_private_handle_accessor"
    "absl_flags_program_name"
    "absl_flags_reflection"
    "absl_graphcycles_internal"
    "absl_hash"
    "absl_int128"
    "absl_kernel_timeout_internal"
    "absl_log_globals"
    "absl_log_internal_check_op"
    "absl_log_internal_conditions"
    "absl_log_internal_fnmatch"
    "absl_log_internal_format"
    "absl_log_internal_globals"
    "absl_log_internal_log_sink_set"
    "absl_log_internal_message"
    "absl_log_internal_nullguard"
    "absl_log_internal_proto"
    "absl_log_sink"
    "absl_low_level_hash"
    "absl_malloc_internal"
    "absl_random_internal_platform"
    "absl_random_internal_pool_urbg"
    "absl_random_internal_randen"
    "absl_random_internal_randen_hwaes"
    "absl_random_internal_randen_hwaes_impl"
    "absl_random_internal_randen_slow"
    "absl_random_internal_seed_material"
    "absl_random_seed_gen_exception"
    "absl_raw_hash_set"
    "absl_raw_logging_internal"
    "absl_spinlock_wait"
    "absl_stacktrace"
    "absl_status"
    "absl_statusor"
    "absl_strerror"
    "absl_strings"
    "absl_strings_internal"
    "absl_string_view"
    "absl_str_format_internal"
    "absl_symbolize"
    "absl_synchronization"
    "absl_throw_delegate"
    "absl_time"
    "absl_time_zone"
    "absl_vlog_config_internal"

    # grpc
    "address_sorting"
    "gpr"
    "grpc++"
    "grpc"
    "upb_base_lib"
    "upb_json_lib"
    "upb_mem_lib"
    "upb_message_lib"
    "upb_textformat_lib"

    # re2
    "re2"

    # zlib
    "zlib"

    # c-ares
    "cares"

    # protobuf
    "utf8_validity"
)

# Link libraries based on configuration
if(CMAKE_BUILD_TYPE STREQUAL "debug")
    target_link_libraries(${TARGET_NAME}
        # openssl
        "libcryptod"
        "libssld"
        # protobuf
        "libprotobufd"
        ${COMMON_LIBS}
    )
else()
    target_link_libraries(${TARGET_NAME}
        # openssl
        "libcrypto"
        "libssl"
        # protobuf
        "libprotobuf"
        ${COMMON_LIBS}
    )
endif()

if(WIN32)
    string(TOLOWER "${CMAKE_BUILD_TYPE}" LOWER_CASE_BUILD_TYPE)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/_build/target-deps/re2/${LOWER_CASE_BUILD_TYPE}/bin/re2.dll
            $<TARGET_FILE_DIR:${TARGET_NAME}>/re2.dll
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/_build/target-deps/c-ares/${LOWER_CASE_BUILD_TYPE}/bin/cares.dll
            $<TARGET_FILE_DIR:${TARGET_NAME}>/cares.dll
    )
endif()