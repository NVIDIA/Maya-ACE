add_custom_target(maya_aceclient_shared)
add_dependencies(maya_aceclient_shared aceclient ace-grpc-cpp)

add_custom_command(
    TARGET maya_aceclient_shared
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIR}/plugins/mace/bin"
)

set(MACE_SRC "${CMAKE_SOURCE_DIR}/source/maya_aceclient")
set(MACE_DST "${OUTPUT_DIR}/plugins/mace")

message(STATUS "${MACE_SRC}")
if(WIN32)
    add_custom_command(
        TARGET maya_aceclient_shared
        POST_BUILD
        # adding double quotes around C:/path/ to avoid being interpreted as flag in cmd
        COMMAND (if exist "\"${MACE_DST}/scripts\"" (rmdir "\"${MACE_DST}/scripts\""))
        COMMAND mklink /J "\"${MACE_DST}/scripts\"" "\"${MACE_SRC}/scripts\"" > NUL

        COMMAND (if exist "\"${MACE_DST}/presets\"" (rmdir "\"${MACE_DST}/presets\""))
        COMMAND mklink /J "\"${MACE_DST}/presets\"" "\"${MACE_SRC}/presets\"" > NUL

        COMMAND (if exist "\"${MACE_DST}/python\"" (rmdir "\"${MACE_DST}/python\""))
        COMMAND mklink /J "\"${MACE_DST}/python\"" "\"${PYTHON_DEPS_ROOT}\"" > NUL

        # cmake copy doesn't support wildcard
        COMMAND ${CMAKE_COMMAND} -E copy
        "${OUTPUT_DIR}/bin/cares.dll" "${OUTPUT_DIR}/plugins/mace/bin/cares.dll"
        COMMAND ${CMAKE_COMMAND} -E copy
        "${OUTPUT_DIR}/bin/re2.dll" "${OUTPUT_DIR}/plugins/mace/bin/re2.dll"
        COMMAND ${CMAKE_COMMAND} -E copy
        "${OUTPUT_DIR}/bin/ace-grpc-cpp.lib" "${OUTPUT_DIR}/plugins/mace/bin/ace-grpc-cpp.lib"
        COMMAND ${CMAKE_COMMAND} -E copy
        "${OUTPUT_DIR}/bin/aceclient.lib" "${OUTPUT_DIR}/plugins/mace/bin/aceclient.lib"
    )
endif()


file(GLOB SRC
    "*.cpp"
    "common/*.cpp"
    "nodes/*.cpp"
    "commands/*.cpp"
)

# maya_aceclient_2024
add_library(maya_aceclient_2024 SHARED ${SRC})
add_dependencies(maya_aceclient_2024 maya_aceclient_shared)

target_include_directories(
    maya_aceclient_2024
    PRIVATE
    ${PROJECT_SOURCE_DIR}/source
    ${PROJECT_SOURCE_DIR}/source/maya_aceclient
    ${ABSL_INCLUDE_DIRS}
    ${AUDIOFILE_INCLUDE_DIRS}
    ${CARES_INCLUDE_DIRS}
    ${GRPC_INCLUDE_DIRS}
    ${MAYA2024_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${RE2_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
)

target_link_directories(
    maya_aceclient_2024
    PRIVATE
    ${ABSL_LIBRARIES}
    ${CARES_LIBRARIES}
    ${GRPC_LIBRARIES}
    ${MAYA2024_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
    ${RE2_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

target_link_libraries(
    maya_aceclient_2024
    PRIVATE
    "aceclient"
    "Foundation"
    "OpenMaya"
    "OpenMayaAnim"
)

set(MAYA2024_BIN_DIR "${OUTPUT_DIR}/plugins/mace/plug-ins/2024")
set_target_properties(maya_aceclient_2024 PROPERTIES OUTPUT_NAME maya_aceclient)
set_target_properties(maya_aceclient_2024 PROPERTIES SUFFIX ".mll")
set_target_properties(maya_aceclient_2024 PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${MAYA2024_BIN_DIR})
set_target_properties(maya_aceclient_2024 PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${MAYA2024_BIN_DIR})
set_target_properties(maya_aceclient_2024 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${MAYA2024_BIN_DIR})

# # maya_aceclient_2025
# add_library(maya_aceclient_2025 SHARED ${SRC})
# add_dependencies(maya_aceclient_2025 maya_aceclient_shared)

# target_include_directories(
#     maya_aceclient_2025
#     PRIVATE
#     ${PROJECT_SOURCE_DIR}/source
#     ${PROJECT_SOURCE_DIR}/source/maya_aceclient
#     ${ABSL_INCLUDE_DIRS}
#     ${AUDIOFILE_INCLUDE_DIRS}
#     ${CARES_INCLUDE_DIRS}
#     ${GRPC_INCLUDE_DIRS}
#     ${MAYA2025_INCLUDE_DIRS}
#     ${OPENSSL_INCLUDE_DIRS}
#     ${PROTOBUF_INCLUDE_DIRS}
#     ${RE2_INCLUDE_DIRS}
#     ${ZLIB_INCLUDE_DIRS}
# )

# target_link_directories(
#     maya_aceclient_2025
#     PRIVATE
#     ${ABSL_LIBRARIES}
#     ${CARES_LIBRARIES}
#     ${GRPC_LIBRARIES}
#     ${MAYA2025_LIBRARIES}
#     ${OPENSSL_LIBRARIES}
#     ${PROTOBUF_LIBRARIES}
#     ${RE2_LIBRARIES}
#     ${ZLIB_LIBRARIES}
# )

# target_link_libraries(
#     maya_aceclient_2025
#     PRIVATE
#     "aceclient"
#     "Foundation"
#     "OpenMaya"
#     "OpenMayaAnim"
# )

# set(MAYA2025_BIN_DIR "${OUTPUT_DIR}/plugins/mace/plug-ins/2025")
# set_target_properties(maya_aceclient_2025 PROPERTIES OUTPUT_NAME maya_aceclient)
# set_target_properties(maya_aceclient_2025 PROPERTIES SUFFIX ".mll")
# set_target_properties(maya_aceclient_2025 PROPERTIES
#     ARCHIVE_OUTPUT_DIRECTORY ${MAYA2025_BIN_DIR})
# set_target_properties(maya_aceclient_2025 PROPERTIES
#     LIBRARY_OUTPUT_DIRECTORY ${MAYA2025_BIN_DIR})
# set_target_properties(maya_aceclient_2025 PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${MAYA2025_BIN_DIR})

# # maya_aceclient_2023
# add_library(maya_aceclient_2023 SHARED ${SRC})
# add_dependencies(maya_aceclient_2023 maya_aceclient_shared)

# target_include_directories(
#     maya_aceclient_2023
#     PRIVATE
#     ${PROJECT_SOURCE_DIR}/source
#     ${PROJECT_SOURCE_DIR}/source/maya_aceclient
#     ${ABSL_INCLUDE_DIRS}
#     ${AUDIOFILE_INCLUDE_DIRS}
#     ${CARES_INCLUDE_DIRS}
#     ${GRPC_INCLUDE_DIRS}
#     ${MAYA2023_INCLUDE_DIRS}
#     ${OPENSSL_INCLUDE_DIRS}
#     ${PROTOBUF_INCLUDE_DIRS}
#     ${RE2_INCLUDE_DIRS}
#     ${ZLIB_INCLUDE_DIRS}
# )

# target_link_directories(
#     maya_aceclient_2023
#     PRIVATE
#     ${ABSL_LIBRARIES}
#     ${CARES_LIBRARIES}
#     ${GRPC_LIBRARIES}
#     ${MAYA2023_LIBRARIES}
#     ${OPENSSL_LIBRARIES}
#     ${PROTOBUF_LIBRARIES}
#     ${RE2_LIBRARIES}
#     ${ZLIB_LIBRARIES}
# )

# target_link_libraries(
#     maya_aceclient_2023
#     PRIVATE
#     "aceclient"
#     "Foundation"
#     "OpenMaya"
#     "OpenMayaAnim"
# )

# set(MAYA2023_BIN_DIR "${OUTPUT_DIR}/plugins/mace/plug-ins/2023")
# set_target_properties(maya_aceclient_2023 PROPERTIES OUTPUT_NAME maya_aceclient)
# set_target_properties(maya_aceclient_2023 PROPERTIES SUFFIX ".mll")
# set_target_properties(maya_aceclient_2023 PROPERTIES
#     ARCHIVE_OUTPUT_DIRECTORY ${MAYA2023_BIN_DIR})
# set_target_properties(maya_aceclient_2023 PROPERTIES
#     LIBRARY_OUTPUT_DIRECTORY ${MAYA2023_BIN_DIR})
# set_target_properties(maya_aceclient_2023 PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${MAYA2023_BIN_DIR})

# # maya_aceclient_2022
# add_library(maya_aceclient_2022 SHARED ${SRC})
# add_dependencies(maya_aceclient_2022 maya_aceclient_shared)

# target_include_directories(
#     maya_aceclient_2022
#     PRIVATE
#     ${PROJECT_SOURCE_DIR}/source
#     ${PROJECT_SOURCE_DIR}/source/maya_aceclient
#     ${ABSL_INCLUDE_DIRS}
#     ${AUDIOFILE_INCLUDE_DIRS}
#     ${CARES_INCLUDE_DIRS}
#     ${GRPC_INCLUDE_DIRS}
#     ${MAYA2022_INCLUDE_DIRS}
#     ${OPENSSL_INCLUDE_DIRS}
#     ${PROTOBUF_INCLUDE_DIRS}
#     ${RE2_INCLUDE_DIRS}
#     ${ZLIB_INCLUDE_DIRS}
# )

# target_link_directories(
#     maya_aceclient_2022
#     PRIVATE
#     ${ABSL_LIBRARIES}
#     ${CARES_LIBRARIES}
#     ${GRPC_LIBRARIES}
#     ${MAYA2022_LIBRARIES}
#     ${OPENSSL_LIBRARIES}
#     ${PROTOBUF_LIBRARIES}
#     ${RE2_LIBRARIES}
#     ${ZLIB_LIBRARIES}
# )

# target_link_libraries(
#     maya_aceclient_2022
#     PRIVATE
#     "aceclient"
#     "Foundation"
#     "OpenMaya"
#     "OpenMayaAnim"
# )

# set(MAYA2022_BIN_DIR "${OUTPUT_DIR}/plugins/mace/plug-ins/2022")
# set_target_properties(maya_aceclient_2022 PROPERTIES OUTPUT_NAME maya_aceclient)
# set_target_properties(maya_aceclient_2022 PROPERTIES SUFFIX ".mll")
# set_target_properties(maya_aceclient_2022 PROPERTIES
#     ARCHIVE_OUTPUT_DIRECTORY ${MAYA2022_BIN_DIR})
# set_target_properties(maya_aceclient_2022 PROPERTIES
#     LIBRARY_OUTPUT_DIRECTORY ${MAYA2022_BIN_DIR})
# set_target_properties(maya_aceclient_2022 PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${MAYA2022_BIN_DIR})
