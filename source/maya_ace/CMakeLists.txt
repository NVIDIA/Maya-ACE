add_custom_target(maya_ace_shared)
add_dependencies(maya_ace_shared ace_client ace_grpc_cpp)


add_custom_command(
    TARGET maya_ace_shared
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIR}/plugins/mace/bin"
)

add_custom_command(
    TARGET maya_ace_shared
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${A2X_SDK_BIN_DIR}" "${OUTPUT_DIR}/plugins/mace/a2x"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIR}/plugins/mace/cuda"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIR}/plugins/mace/tensorrt"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CUDAToolkit_BIN_DIR}/cublasLt64_12.dll" "${OUTPUT_DIR}/plugins/mace/cuda/"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CUDAToolkit_BIN_DIR}/cublas64_12.dll" "${OUTPUT_DIR}/plugins/mace/cuda/"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CUDAToolkit_BIN_DIR}/cudart64_12.dll" "${OUTPUT_DIR}/plugins/mace/cuda/"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CUDAToolkit_BIN_DIR}/curand64_10.dll" "${OUTPUT_DIR}/plugins/mace/cuda/"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${TENSORRT_LIB_DIR}/nvinfer_10.dll" "${OUTPUT_DIR}/plugins/mace/tensorrt/"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${TENSORRT_LIB_DIR}/nvinfer_plugin_10.dll" "${OUTPUT_DIR}/plugins/mace/tensorrt/"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${TENSORRT_LIB_DIR}/nvonnxparser_10.dll" "${OUTPUT_DIR}/plugins/mace/tensorrt/"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${TENSORRT_LIB_DIR}/nvinfer_builder_resource_10.dll" "${OUTPUT_DIR}/plugins/mace/tensorrt/"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${TENSORRT_BIN_DIR}/trtexec.exe" "${OUTPUT_DIR}/plugins/mace/tensorrt/"
)


set(MACE_SRC "${PROJECT_SOURCE_DIR}/source/maya_ace")
set(MACE_DST "${OUTPUT_DIR}/plugins/mace")

message(STATUS "${MACE_SRC}")
if(WIN32)
    add_custom_command(
        TARGET maya_ace_shared
        POST_BUILD
        # Remove directories - if symlink/junction just remove link, else remove recursively
        # Using fsutil to check if directory is a reparse point (junction/symlink)
        COMMAND (if exist "\"${MACE_DST}/scripts\"" (
            fsutil reparsepoint query "\"${MACE_DST}/scripts\"" >nul 2>&1 && (
                rmdir "\"${MACE_DST}/scripts\""
            ) || (
                rmdir /s /q "\"${MACE_DST}/scripts\""
            )
        ))
        COMMAND (if exist "\"${MACE_DST}/presets\"" (
            fsutil reparsepoint query "\"${MACE_DST}/presets\"" >nul 2>&1 && (
                rmdir "\"${MACE_DST}/presets\""
            ) || (
                rmdir /s /q "\"${MACE_DST}/presets\""
            )
        ))
        COMMAND (if exist "\"${MACE_DST}/python\"" (
            fsutil reparsepoint query "\"${MACE_DST}/python\"" >nul 2>&1 && (
                rmdir "\"${MACE_DST}/python\""
            ) || (
                rmdir /s /q "\"${MACE_DST}/python\""
            )
        ))
    )

    if(MACE_DEV_MODE)
        add_custom_command(
            TARGET maya_ace_shared
            POST_BUILD
            # Create symbolic links
            COMMAND mklink /J "\"${MACE_DST}/scripts\"" "\"${MACE_SRC}/scripts\"" > NUL
            COMMAND mklink /J "\"${MACE_DST}/presets\"" "\"${MACE_SRC}/presets\"" > NUL
            COMMAND mklink /J "\"${MACE_DST}/python\"" "\"${PYTHON_DEPS_ROOT}\"" > NUL
        )
    else()
        add_custom_command(
            TARGET maya_ace_shared
            POST_BUILD
            # Copy directories
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${MACE_SRC}/scripts" "${MACE_DST}/scripts"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${MACE_SRC}/presets" "${MACE_DST}/presets"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${PYTHON_DEPS_ROOT}" "${MACE_DST}/python"
        )
    endif()

    add_custom_command(
        TARGET maya_ace_shared
        POST_BUILD
        # create a placeholder for the models directory
        COMMAND ${CMAKE_COMMAND} -E copy
        "\"${PROJECT_SOURCE_DIR}/sample_project/models/README.md\"" "\"${MACE_DST}/models/README.md\""

        # cmake copy doesn't support wildcard
        COMMAND ${CMAKE_COMMAND} -E copy
        "${OUTPUT_DIR}/bin/cares.dll" "${MACE_DST}/bin/cares.dll"
        COMMAND ${CMAKE_COMMAND} -E copy
        "${OUTPUT_DIR}/bin/re2.dll" "${MACE_DST}/bin/re2.dll"
        COMMAND ${CMAKE_COMMAND} -E copy
        "${OUTPUT_DIR}/bin/ace_grpc_cpp.lib" "${MACE_DST}/bin/ace_grpc_cpp.lib"
        COMMAND ${CMAKE_COMMAND} -E copy
        "${OUTPUT_DIR}/bin/ace_client.lib" "${MACE_DST}/bin/ace_client.lib"
    )
endif()


file(GLOB SRC
    "*.cpp"
    "common/*.cpp"
    "nodes/*.cpp"
    "commands/*.cpp"
)

# define a list of supported Maya versions and their default enabled status
set(MAYA_YEAR_OPTIONS
    2022 OFF
    2024 ON
    2025 ON
    2026 ON
)
set(MAYA_YEARS) # list of supported Maya versions parsed from MAYA_YEAR_OPTIONS
while(MAYA_YEAR_OPTIONS)
    list(POP_FRONT MAYA_YEAR_OPTIONS year onoff)
    list(APPEND MAYA_YEARS ${year})
    option(WANT_MAYA_${year} "Build Maya ${year} compatible plugin" ${onoff})
endwhile()

foreach(year IN LISTS MAYA_YEARS)
    # only define targets for Maya versions that are set to ON AND found by FindMaya.cmake
    if (WANT_MAYA_${year})
        if (MAYA${year}_FOUND)
            message(STATUS "Building Maya ${year} compatible plugin")
            add_library(maya_ace_${year} SHARED ${SRC})
        add_dependencies(maya_ace_${year} maya_ace_shared)

        target_include_directories(
            maya_ace_${year}
            PRIVATE
            ${PROJECT_SOURCE_DIR}/source
            ${PROJECT_SOURCE_DIR}/source/maya_ace
            ${MAYA${year}_INCLUDE_DIRS}
            ${ABSL_INCLUDE_DIRS}
            ${AUDIOFILE_INCLUDE_DIRS}
            ${CARES_INCLUDE_DIRS}
            ${GRPC_INCLUDE_DIRS}
            ${OPENSSL_INCLUDE_DIRS}
            ${PROTOBUF_INCLUDE_DIRS}
            ${RE2_INCLUDE_DIRS}
            ${A2X_SDK_INCLUDE_DIRS}
            ${TENSORRT_INCLUDE_DIRS}
            ${RAPIDJSON_INCLUDE_DIRS}
        )

        target_link_directories(
            maya_ace_${year}
            PRIVATE
            ${MAYA${year}_LIBRARIES}
            ${ABSL_LIBRARIES}
            ${CARES_LIBRARIES}
            ${GRPC_LIBRARIES}
            ${OPENSSL_LIBRARIES}
            ${PROTOBUF_LIBRARIES}
            ${RE2_LIBRARIES}
            ${A2X_SDK_LIBRARIES}
        )

        target_link_libraries(
            maya_ace_${year}
            PRIVATE
            "ace_client"
            "Foundation"
            "OpenMaya"
            "OpenMayaAnim"
            "OpenMayaUI"
            "audio2x"
        )

        set(MAYA${year}_BIN_DIR "${OUTPUT_DIR}/plugins/mace/plug-ins/${year}")
        set_target_properties(maya_ace_${year} PROPERTIES OUTPUT_NAME maya_ace)
        set_target_properties(maya_ace_${year} PROPERTIES SUFFIX ".mll")
        set_target_properties(maya_ace_${year} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY ${MAYA${year}_BIN_DIR})
        set_target_properties(maya_ace_${year} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${MAYA${year}_BIN_DIR})
        set_target_properties(maya_ace_${year} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${MAYA${year}_BIN_DIR})
        else()
            message(STATUS "Skipping Maya ${year} plugin - Maya ${year} DevKit not found (wanted but not available)")
        endif()
    endif()
endforeach()

# set an alias target mace for the default Maya version
add_custom_target(mace)
if(TARGET maya_ace_2024) # default Maya version
    add_dependencies(mace
        maya_ace_2024
    )
else()
    # otherwise, use the first target that is set to ON and found
    set(found_default_maya_target FALSE)
    foreach(year IN LISTS MAYA_YEARS)
        if (WANT_MAYA_${year} AND MAYA${year}_FOUND)
            add_dependencies(mace
                maya_ace_${year}
            )
            set(found_default_maya_target TRUE)
            break()
        endif()
    endforeach()
    if (NOT found_default_maya_target)
        message(FATAL_ERROR "No default Maya target found. Please ensure Maya DevKit is installed and set WANT_MAYA_{year} to ON for an available Maya version.")
    endif()
endif()
