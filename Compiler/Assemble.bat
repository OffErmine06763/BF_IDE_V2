@echo off

:loop
if "%~1"=="" goto end
if "%~2"=="" goto end

nasm -f win64 %~1 -o %~2 -Wno-pp-macro-params-legacy
if %errorlevel% neq 0 (
    echo Assembly failed with error %errorlevel%
    exit /b %errorlevel%
)

shift
shift
goto loop

:end