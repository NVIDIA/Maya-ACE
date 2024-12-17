cmake_minimum_required(VERSION 3.16)

project(maya-ace)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")

include(${PROJECT_SOURCE_DIR}/cmake/compiler_settings.cmake)

file(READ ${CMAKE_SOURCE_DIR}/source/VERSION.md MACE_VERSION)
string(STRIP ${MACE_VERSION} MACE_VERSION)
configure_file(${CMAKE_SOURCE_DIR}/source/mace.mod.in ${OUTPUT_DIR}/plugins/mace.mod @ONLY)

find_package(Absl)
find_package(Audiofile)
find_package(Cares)
find_package(Grpc)
find_package(Gtest)
find_package(Maya)
find_package(Openssl)
find_package(Protobuf)
find_package(Re2)
find_package(Zlib)
find_package(PythonDeps)

add_subdirectory(source)
add_subdirectory(tests)
