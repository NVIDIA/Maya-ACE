@REM Setup the environment to use libraries and plugins

@echo off

if "%VARIANT%"=="" SET "VARIANT=release"
@REM SET VARIANT=debug

@REM set the base directories
set "BASE_DIR=%~dp0"
set "TARGET_DEPS=%BASE_DIR%_build\target-deps"
set "BUILD_DIR=%BASE_DIR%_build\windows-x86_64"

set "PYTHONPATH=%BASE_DIR%venv\Lib\site-packages"
set "MAYA_MODULE_PATH=%BUILD_DIR%\%VARIANT%\plugins\"

IF "%MAYA_LOCATION%"=="" SET "MAYA_LOCATION=C:\Program Files\Autodesk\Maya2024"
echo MAYA_LOCATION=%MAYA_LOCATION%

cd /D "%MAYA_LOCATION%"\bin\

call %*

cd /D "%BASE_DIR%"
