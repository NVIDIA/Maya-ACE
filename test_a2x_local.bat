@echo off
set "BASE_DIR=%~dp0"
set "OUTPUT_DIR=%BASE_DIR%_build\windows-x86_64\release"

@REM Set CUDA path - use CUDA_PATH if defined, otherwise use default
if defined CUDA_PATH (
    set "CUDA_LIB_PATH=%CUDA_PATH%\bin"
) else (
    set "CUDA_LIB_PATH=%BASE_DIR%deps\cuda\bin"
)

@REM Set TensorRT path - use TENSORRT_ROOT_DIR if defined, otherwise use default
if defined TENSORRT_ROOT_DIR (
    set "TENSORRT_LIB_PATH=%TENSORRT_ROOT_DIR%\lib"
) else (
    set "TENSORRT_LIB_PATH=%BASE_DIR%deps\tensorrt\lib"
)

@REM Set A2X_SDK path - use A2X_SDK_ROOT if defined, otherwise use default
if defined A2X_SDK_ROOT (
    set "A2X_SDK_LIB_PATH=%A2X_SDK_ROOT%\bin"
) else (
    set "A2X_SDK_LIB_PATH=%BASE_DIR%deps\audio2x-sdk\bin"
)

set PATH=%CUDA_LIB_PATH%;%PATH%
set PATH=%TENSORRT_LIB_PATH%;%PATH%
set PATH=%A2X_SDK_LIB_PATH%;%PATH%

@REM run a2x_local tests with dependencies in the path
cd %BASE_DIR%
call "%BASE_DIR%_build\windows-x86_64\release\bin\tests_a2x_local.exe" %*
