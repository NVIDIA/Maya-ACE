@echo off
@REM This script is used to generate TensorRT engines for the models.
@REM It is calling the trtgen python module with the provided arguments.
@echo.
@echo TRT Engine Generator for Audio2Face models
@echo.
@echo   Usage:
@echo     trtgen.bat [OPTIONS] COMMAND [MODEL_NAME]
@echo.
@echo   Commands:
@echo     list       - List available ONNX models
@echo     build      - Build TensorRT engines from ONNX models
@echo     clean      - Delete TensorRT engine files
@echo     list_trt   - List existing TensorRT engines
@echo.
@echo   Examples:
@echo     trtgen.bat list                        - List all ONNX models
@echo     trtgen.bat list audio2face-3d-v3.0     - List only audio2face-3d-v3.0 ONNX models
@echo     trtgen.bat build audio2face-3d-v3.0    - Build TensorRT engines for audio2face-3d-v3.0
@echo     trtgen.bat build audio2emotion-v2.2    - Build TensorRT engines for audio2emotion-v2.2
@echo     trtgen.bat build *                     - Build TensorRT engines for all models
@echo     trtgen.bat clean                       - Delete all TensorRT engines
@echo     trtgen.bat clean audio2face-3d-v3.0    - Delete TensorRT engines for audio2face-3d-v3.0
@echo     trtgen.bat list_trt                    - List all TensorRT engines
@echo.
@echo   Advanced Options:
@echo     --trtexec PATH          - Specify custom trtexec path
@echo     --model-dir DIR         - Specify custom model directory
@echo.

@REM enable delayed expansion to handle variables changing during runtime
setlocal EnableDelayedExpansion

set "SCRIPT_DIR=%~dp0"

@REM Convert relative paths to absolute paths
for %%i in ("%SCRIPT_DIR%..") do set "MAYA_ACE_DIR=%%~fi\"
for %%i in ("%MAYA_ACE_DIR%..") do set "SOURCE_DIR=%%~fi\"

@REM repo root
for %%i in ("%SOURCE_DIR%..") do set "ROOT_DIR=%%~fi\"

if not defined A2F_MODEL_DIRS (
    if not defined MODELS_DIR (
        if exist "%MAYA_ACE_DIR%models\" (
            @REM running in maya plugin directory
            set "MODELS_DIR=%MAYA_ACE_DIR%models\"
        ) else (
            @REM running in repo root directory
            set "MODELS_DIR=%ROOT_DIR%sample_project\models\"
        )
    )
    set "A2F_MODEL_DIRS=!MODELS_DIR!audio2face-models;"
)
if not defined A2E_MODEL_DIRS (
    if not defined MODELS_DIR (
        if exist "%MAYA_ACE_DIR%models\" (
            @REM running in maya plugin directory
            set "MODELS_DIR=%MAYA_ACE_DIR%models\"
        ) else (
            @REM running in repo root directory
            set "MODELS_DIR=%ROOT_DIR%sample_project\models\"
        )
    )
    set "A2E_MODEL_DIRS=!MODELS_DIR!audio2emotion-models;"
)

@REM required to build trt files
if not defined TRTEXEC_PATH (
    if exist "%MAYA_ACE_DIR%\tensorrt\trtexec.exe" (
        @REM running in maya plugin directory
        set "TRTEXEC_PATH=%MAYA_ACE_DIR%\tensorrt\trtexec.exe"
    ) else (
        @REM running in repo root directory
        set "TRTEXEC_PATH=%ROOT_DIR%trtexec.bat"
    )
)

echo SCRIPT_DIR: %SCRIPT_DIR%
echo MAYA_ACE_DIR: %MAYA_ACE_DIR%
echo TRTEXEC_PATH: %TRTEXEC_PATH%
echo.

@REM Add script path to PYTHONPATH to import trtgen module
set "PYTHONPATH=%SCRIPT_DIR%;%PYTHONPATH%"

@REM Check if mayapy is available in the system PATH
where mayapy >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    @REM mayapy is available in PATH
    echo running mayapy -m trtgen %*
    call mayapy -m trtgen %*
) else (
    @REM mayapy not found, use system python
    echo running python -m trtgen %*
    call python -m trtgen %*
)
