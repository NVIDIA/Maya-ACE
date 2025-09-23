@echo off

set "BASE_DIR=%~dp0"
set "ALL_ARGS=%*"

if "%1"=="debug" (
    echo Running in DEBUG mode
    SET "VARIANT=debug"
    set "ALL_ARGS=%ALL_ARGS:debug=%"
)

%BASE_DIR%env_maya.bat call maya.exe %ALL_ARGS%
