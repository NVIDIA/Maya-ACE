@REM Setup the environment to use libraries and plugins
@echo off
setlocal EnableDelayedExpansion

:: load_env.bat
set "ENV_FILE=.env"

:: Check if .env exists
if exist "%ENV_FILE%" (
    echo Loading environment variables from: %ENV_FILE%
    :: First pass: Load raw variables
    for /f "usebackq tokens=*" %%a in ("%ENV_FILE%") do (
        set "line=%%a"
        if not "!line:~0,1!"=="#" if not "!line!"=="" (
            for /f "tokens=1,* delims==" %%b in ("!line!") do (
                set "%%b=%%c"
            )
        )
    )

    :: Second pass: Resolve embedded variables
    for /f "usebackq tokens=*" %%a in ("%ENV_FILE%") do (
        set "line=%%a"
        if not "!line:~0,1!"=="#" if not "!line!"=="" (
            for /f "tokens=1,* delims==" %%b in ("!line!") do (
                call set "%%b=%%c"
            )
        )
    )
)


if "%VARIANT%"=="" SET "VARIANT=release"
@REM SET VARIANT=debug

@REM set the base directories
set "BASE_DIR=%~dp0"
set "TARGET_DEPS=%BASE_DIR%_build\target-deps"
set "BUILD_DIR=%BASE_DIR%_build\windows-x86_64"

@REM data locations for dev environment
set "A2F_MODEL_DIRS=%BASE_DIR%sample_project\models\audio2face-models;%A2F_MODEL_DIRS%"
set "A2E_MODEL_DIRS=%BASE_DIR%sample_project\models\audio2emotion-models;%A2E_MODEL_DIRS%"

set "PYTHONPATH=%BASE_DIR%_venv\Lib\site-packages;%PYTHONPATH%"
set "MAYA_MODULE_PATH=%BUILD_DIR%\%VARIANT%\plugins\"

IF "%MAYA_LOCATION%"=="" SET "MAYA_LOCATION=C:\Program Files\Autodesk\Maya2024"
echo MAYA_LOCATION=%MAYA_LOCATION%

@REM preserve exit code when mayapy terminates
set MAYA_NO_STANDALONE_ATEXIT=1

@REM cd /D "%MAYA_LOCATION%"\bin\
set "PATH=%MAYA_LOCATION%\bin;%PATH%"

call %*

cd /D "%BASE_DIR%"
