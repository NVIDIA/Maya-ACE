@echo off
set "BASE_DIR=%~dp0"
set "PYTHONPATH=%BASE_DIR%python;%PYTHONPATH%"

@REM Check if _venv directory exists, if not create it
if not exist %BASE_DIR%_venv (
    echo Creating venv at _venv with %PYTHON%
    %PYTHON% -m pip install virtualenv
    %PYTHON% -m venv %BASE_DIR%_venv --upgrade-deps
    if errorlevel 1 (
        echo Failed to create venv at _venv
        exit /b 1
    )
)

@REM Activate the virtual environment
call %BASE_DIR%_venv\Scripts\activate
if errorlevel 1 (
    echo Failed to activate _venv
    exit /b 1
)

python -m mock_server
