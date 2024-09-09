@echo off

if not exist venv (
    echo Creating venv...
    python -m venv venv
)

call .\venv\Scripts\activate
pip install -r requirements.txt
python .\scripts\pull_deps.py