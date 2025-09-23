@REM usage: build.bat <project> <config: release|debug>

@echo off

set "BASE_DIR=%~dp0"
set "BUILD_DIR=%BASE_DIR%_build"
set "CMAKE=%BASE_DIR%_deps\build-deps\cmake\bin\cmake.exe"
set "PATH=%BASE_DIR%_deps\build-deps\ninja;%PATH%"

@REM When MACE_DEV_MODE is ON, use symbolic links for scripts, presets, and python files
@REM When MACE_DEV_MODE is OFF, copy files to the build directory
set MACE_DEV_MODE=0

if not defined TENSORRT_ROOT_DIR (
    echo;
    echo TENSORRT_ROOT_DIR not set. Using default: %BASE_DIR%deps\tensorrt
    echo Please set TENSORRT_ROOT_DIR to the root directory of your TensorRT installation.
    set "TENSORRT_ROOT_DIR=%BASE_DIR%deps\tensorrt"
)

if not defined CUDA_PATH (
    echo;
    echo CUDA_PATH not set. Using default: %BASE_DIR%deps\cuda
    echo Please install CUDA toolkit or set CUDA_PATH to the root directory of your CUDA installation.
    set "CUDA_PATH=%BASE_DIR%deps\cuda"
)

if not defined A2X_SDK_ROOT (
    echo;
    echo A2X_SDK_ROOT not set. Using default: %BASE_DIR%deps\audio2x-sdk
    echo Please set A2X_SDK_ROOT to the root directory of your Audio2X SDK installation.
    set "A2X_SDK_ROOT=%BASE_DIR%deps\audio2x-sdk"
)

set BUILD_PROJECT=all
if not "%1"=="" (
    set BUILD_PROJECT=%1
)

@REM Set the default build configuration to release
set BUILD_CONFIG=release

@REM Check if a build configuration was provided as an argument
if not "%2"=="" (
    set BUILD_CONFIG=%2
)

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

call "%BASE_DIR%fetch_deps.bat" %BUILD_CONFIG%

@REM Find the latest Visual Studio installation path using vswhere
for /f "delims=" %%i in ('"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath') do set "VS_PATH=%%i"

@REM Check if VS_PATH was set successfully
if not defined VS_PATH (
    echo Visual Studio installation not found.
    exit /b 1
)

@REM Call vcvarsall.bat with x64 argument
call "%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat" x64 > "%BUILD_DIR%\vsdevcmd.trace.txt" 2>&1

@REM CUDA_PATH will be read from environment variable by FindCUDA.cmake
@REM     While TENSORRT_ROOT_DIR and A2X_SDK_ROOT will be passed as CMake variables.
%CMAKE% -G Ninja -S %BASE_DIR% -B %BUILD_DIR% -DCMAKE_BUILD_TYPE=%BUILD_CONFIG% -DTENSORRT_ROOT_DIR="%TENSORRT_ROOT_DIR%" -DA2X_SDK_ROOT="%A2X_SDK_ROOT%"
%CMAKE% --build "%BUILD_DIR%" --target "%BUILD_PROJECT%" --parallel
