@REM Run mayapy -m unittest with required settings minimizing noise

@echo off
set "BASE_DIR=%~dp0"

call %BASE_DIR%venv\Scripts\activate

if [%1] == [] (
    call python -m python.mock_server .\env_maya.bat mayapy.exe -W ignore::DeprecationWarning "%BASE_DIR%tools\test_with_coverage.py"
) else (
    call python -m python.mock_server .\env_maya.bat mayapy.exe -W ignore::DeprecationWarning -m unittest discover -v -s "%BASE_DIR%source\maya_aceclient\tests" %*
)
