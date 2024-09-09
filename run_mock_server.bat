@echo off
set "BASE_DIR=%~dp0"

set "PYTHONPATH=%BASE_DIR%source/maya_aceclient/tests"
python -m mock_ace_server.main