@REM Run mayapy -m unittest with required settings minimizing noise

@echo off
set "BASE_DIR=%~dp0"

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

set "PYTHONPATH=%BASE_DIR%python;%PYTHONPATH%"

@REM run maya_ace tests under mock server environment and required dependencies
if [%1] == [] (
    @REM default: run test with coverage report
    call python -m mock_server %BASE_DIR%env_maya.bat mayapy.exe -W ignore::DeprecationWarning "%BASE_DIR%tools\test_with_coverage.py"
) else (
    @REM run test with custom arguments
    call python -m mock_server %BASE_DIR%env_maya.bat mayapy.exe -W ignore::DeprecationWarning -m unittest discover -v -s "%BASE_DIR%source\maya_ace\tests" %*
)
