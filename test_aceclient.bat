@echo off
set "BASE_DIR=%~dp0"

call %BASE_DIR%venv\Scripts\activate
python -m python.mock_server "%BASE_DIR%_build\windows-x86_64\release\bin\tests-aceclient.exe" %*