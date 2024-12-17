@REM usage: build.bat <project> <config: release|debug>

@echo off

set BUILD_DIR=%~dp0_build
set CMAKE=%~dp0_build\build-deps\cmake\bin\cmake.exe
set PATH=%~dp0_build\build-deps\ninja;%PATH%

set BUILD_PROJECT=mace-all
if not "%1"=="" (
    set BUILD_PROJECT=%1
)

@REM Set the default build configuration to release
set BUILD_CONFIG=release

@REM Check if a build configuration was provided as an argument
if not "%2"=="" (
    set BUILD_CONFIG=%2
)

call %~dp0fetch_deps.bat %BUILD_CONFIG%

@REM Find the latest Visual Studio installation path using vswhere
for /f "delims=" %%i in ('"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath') do set "VS_PATH=%%i"

@REM Check if VS_PATH was set successfully
if not defined VS_PATH (
    echo Visual Studio installation not found.
    exit /b 1
)

@REM Call vcvarsall.bat with x64 argument
call "%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat" x64 > %BUILD_DIR%\vsdevcmd.trace.txt 2>&1

%CMAKE% -G Ninja -S %~dp0 -B %BUILD_DIR% -DCMAKE_BUILD_TYPE=%BUILD_CONFIG% 
%CMAKE% --build %BUILD_DIR% --parallel