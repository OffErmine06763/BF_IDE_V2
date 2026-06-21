@echo off

where nasm >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: NASM not found in PATH
    exit /b %errorlevel%
)