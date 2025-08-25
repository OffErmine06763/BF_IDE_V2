@echo off
>nul call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

link /OUT:%~1 /ENTRY:_main /SUBSYSTEM:CONSOLE /NODEFAULTLIB %~2 kernel32.lib
if %errorlevel% neq 0 (
    echo Linking failed with error %errorlevel%
    exit /b %errorlevel%
)