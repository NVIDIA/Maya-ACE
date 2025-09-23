@echo off
set "BASE_DIR=%~dp0"

call %BASE_DIR%_venv\Scripts\activate
set "PYTHONPATH=%BASE_DIR%python;%PYTHONPATH%"

@REM run ace_client tests under mock server environment
cd %BASE_DIR%
python -m mock_server "%BASE_DIR%_build\windows-x86_64\release\bin\tests_ace_client.exe" %*
