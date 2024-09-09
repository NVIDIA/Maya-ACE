@REM Run mayapy -m unittest with required settings minimizing noise

@echo off
set "BASE_DIR=%~dp0"

@REM preserve exit code when mayapy terminates
set MAYA_NO_STANDALONE_ATEXIT=1

@REM disable any user-setup side effects
set MAYA_SKIP_USERSETUP_PY=1

call .\env_maya.bat mayapy.exe -W ignore::DeprecationWarning -m unittest discover -v -s "%BASE_DIR%source\maya_aceclient\tests" %*
