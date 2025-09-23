cmake_minimum_required(VERSION 3.16)

project(maya-ace)
set(USER_MAYA_DIR "$ENV{HOME}/maya")
if(WIN32)
    set(USER_MAYA_DIR "$ENV{USERPROFILE}/Documents/maya")
endif()
set(CMAKE_INSTALL_PREFIX "${USER_MAYA_DIR}/modules")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")

# Development mode flag - controls whether to use symbolic links or copy files
# Can be set via CMake option or environment variable
if(DEFINED ENV{MACE_DEV_MODE})
    set(MACE_DEV_MODE $ENV{MACE_DEV_MODE})
else()
    option(MACE_DEV_MODE "Use links instead of copying files for scripts (development mode)" OFF)
endif()

# Disable manifest generation as maya plugin does not require it.
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:NO")
endif()

include(${PROJECT_SOURCE_DIR}/cmake/compiler_settings.cmake)

file(READ ${PROJECT_SOURCE_DIR}/source/VERSION.md MACE_VERSION)
string(STRIP ${MACE_VERSION} MACE_VERSION)
configure_file(${PROJECT_SOURCE_DIR}/source/mace.mod.in ${OUTPUT_DIR}/plugins/mace.mod @ONLY)

set(GTEST_ROOT ${PROJECT_SOURCE_DIR}/_deps/target-deps/gtest/${CMAKE_BUILD_TYPE})
set(ZLIB_ROOT ${PROJECT_SOURCE_DIR}/_deps/target-deps/zlib/${CMAKE_BUILD_TYPE})

find_package(Absl REQUIRED)
find_package(Audiofile REQUIRED)
find_package(Cares REQUIRED)
find_package(Grpc REQUIRED)
find_package(GTest REQUIRED)
find_package(Maya REQUIRED)
find_package(Openssl REQUIRED)
find_package(Protobuf REQUIRED)
find_package(Re2 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PythonDeps REQUIRED)
find_package(CUDAToolkit REQUIRED)
find_package(TensorRT REQUIRED)
find_package(A2X_SDK REQUIRED)
find_package(RapidJson REQUIRED)

add_subdirectory(source)
add_subdirectory(tests)
