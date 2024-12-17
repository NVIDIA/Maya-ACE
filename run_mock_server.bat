@echo off
set "BASE_DIR=%~dp0"

call %BASE_DIR%venv\Scripts\activate
python -m python.mock_server
