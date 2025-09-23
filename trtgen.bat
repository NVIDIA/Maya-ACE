@REM It is calling the trtgen.bat script in the source/maya_ace/scripts directory.
@REM 
@REM Usage:
@REM     trtgen.bat [options]
@echo off

set ROOT_DIR=%~dp0

set MODELS_DIR=%ROOT_DIR%sample_project\models\
set TRTEXEC_PATH=%ROOT_DIR%trtexec.bat

@REM finally, run the trtgen script
call %ROOT_DIR%source\maya_ace\scripts\trtgen.bat %*
