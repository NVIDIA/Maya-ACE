@echo off

set "BASE_DIR=%~dp0"

@REM Activate the virtual environment
if "%PYTHON%"=="" SET "PYTHON=python.exe"

@REM Check if _venv directory exists, if not create it
if not exist "%BASE_DIR%_venv" (
    echo Creating venv at _venv with %PYTHON%
    %PYTHON% -m pip install virtualenv
    %PYTHON% -m venv "%BASE_DIR%_venv" --upgrade-deps
    if errorlevel 1 (
        echo Failed to create venv at _venv
        exit /b 1
    )
)
call "%BASE_DIR%_venv\Scripts\activate"
if errorlevel 1 (
    echo Failed to activate _venv
    exit /b 1
)
call python -m pip install -r "%BASE_DIR%deps\requirements.txt" -q --disable-pip-version-check


@REM Download A2F models
if exist "%BASE_DIR%sample_project\models" (
    set MODEL_DIR="%BASE_DIR%sample_project\models"
) else (
    set MODEL_DIR="%BASE_DIR%models"
)

hf download nvidia/Audio2Face-3D-v3.0          --local-dir %MODEL_DIR%/audio2face-models/audio2face-3d-v3.0
hf download nvidia/Audio2Face-3D-v2.3.1-Claire --local-dir %MODEL_DIR%/audio2face-models/audio2face-3d-v2.3.1-claire
hf download nvidia/Audio2Face-3D-v2.3.1-James  --local-dir %MODEL_DIR%/audio2face-models/audio2face-3d-v2.3.1-james
hf download nvidia/Audio2Face-3D-v2.3-Mark     --local-dir %MODEL_DIR%/audio2face-models/audio2face-3d-v2.3-mark

@REM Download A2E models
hf download nvidia/Audio2Emotion-v2.2          --local-dir %MODEL_DIR%/audio2emotion-models/audio2emotion-v2.2

call deactivate
