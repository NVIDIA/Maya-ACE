@echo off

set BUILD_CONFIG=release
@REM Check if a build configuration was provided as an argument
if not "%1"=="" (
    set BUILD_CONFIG=%1
)
set "BASE_DIR=%~dp0"
if "%PYTHON%"=="" SET "PYTHON=python.exe"
if "%PACKMAN%"=="" SET "PACKMAN=%BASE_DIR%tools\packman\packman"

@REM Check if _venv directory exists, if not create it
if not exist "%BASE_DIR%_venv" (
    echo Creating venv at _venv with %PYTHON%
    %PYTHON% -m pip install virtualenv
    %PYTHON% -m venv "%BASE_DIR%_venv" --upgrade-deps
    if errorlevel 1 (
        echo Failed to create venv at _venv
        exit /b 1
    )
)

@REM Activate the virtual environment
call "%BASE_DIR%_venv\Scripts\activate"
if errorlevel 1 (
    echo Failed to activate _venv
    exit /b 1
)
@REM Install python dependencies (mainly used by mock server for unit test)
call python -m pip install -r "%BASE_DIR%deps\requirements.txt" -q --disable-pip-version-check
if errorlevel 1 (
    echo Failed to install python dependencies
    exit /b 1
)
@REM Download python dependencies for maya python scripts
call python -m pip install -r "%BASE_DIR%deps\python-deps.pip.txt" -q --disable-pip-version-check --target "%BASE_DIR%_deps\python-deps"
if errorlevel 1 (
    echo Failed to pull dependencies in python-deps.pip.txt
    exit /b 1
)
call deactivate

@REM Pull dependencies using packman
call %PACKMAN% pull -t config=%BUILD_CONFIG% --platform windows-x86_64 "%BASE_DIR%deps\build-deps.packman.xml"
if errorlevel 1 (
    echo Failed to pull dependencies in build-deps.packman.xml
    exit /b 1
)
call %PACKMAN% pull -t config=%BUILD_CONFIG% --platform windows-x86_64 "%BASE_DIR%deps\target-deps.packman.xml"
if errorlevel 1 (
    echo Failed to pull dependencies in target-deps.packman.xml
    exit /b 1
)
