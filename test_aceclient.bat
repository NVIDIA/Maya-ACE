@echo off
set "BASE_DIR=%~dp0"

set "PYTHONPATH=%BASE_DIR%source/maya_aceclient/tests"
call .\venv\Scripts\activate
python -m mock_ace_server.main "%BASE_DIR%_build/windows-x86_64/release/bin/tests-aceclient.exe" %*