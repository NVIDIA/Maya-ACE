@echo off

set "BASE_DIR=%~dp0"
set "BUILD_DIR=%BASE_DIR%_build\"

if not defined TENSORRT_ROOT_DIR (
    echo;
    echo TENSORRT_ROOT_DIR not set. Using default: %~dp0deps\tensorrt
    echo Please set TENSORRT_ROOT_DIR to the root directory of your TensorRT installation.
    set TENSORRT_ROOT_DIR=%BASE_DIR%deps\tensorrt
)

if not exist %TENSORRT_ROOT_DIR% (
    echo TensorRT not found in %TENSORRT_ROOT_DIR%
    exit /b 1
)

REM Update PATH with necessary directories
@REM set "PATH=%BUILD_DIR%a2x-deps\cuda\bin;%PATH%"
set "PATH=%TENSORRT_ROOT_DIR%\lib;%PATH%"
set "PATH=%TENSORRT_ROOT_DIR%\bin;%PATH%"

call %TENSORRT_ROOT_DIR%\bin\trtexec.exe %*
