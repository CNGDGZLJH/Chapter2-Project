@echo off
rem ============================================================
rem  Build script for this project (called by VS Code tasks)
rem
rem  Why this file exists:
rem  VS Code "shell" tasks hand the command to the terminal's default
rem  shell (PowerShell 5.1 on this machine), which does NOT support
rem  the && operator. Running the build through this .bat file lets
rem  cmd.exe handle it and avoids that problem entirely.
rem
rem  NOTE: keep this file pure ASCII. cmd.exe reads .bat files using
rem  the ANSI code page (936 here), so non-ASCII bytes in comments
rem  get mangled and executed as bogus commands.
rem
rem  Usage:  build.bat                  compile only (Debug)
rem          build.bat Debug run        compile and run (Debug)
rem          build.bat Release          compile only (Release)
rem          build.bat Release run      compile and run (Release)
rem ============================================================

setlocal
cd /d "%~dp0"

set GXX=C:\msys64\ucrt64\bin\g++.exe
set OUT=build\main.exe

if not exist build mkdir build

set MODE=Debug
set FLAGS=-g3 -O0
if /i "%~1"=="release" (
    set MODE=Release
    set FLAGS=-O2 -static -static-libgcc -static-libstdc++
)

echo [%MODE%] Building project...

rem Wildcard *.cpp is expanded by cmd, not by g++ (g++ does not expand it).
"%GXX%" -std=c++17 %FLAGS% -Wall -Wextra *.cpp -o "%OUT%"
if errorlevel 1 (
    echo.
    echo BUILD FAILED - aborted.
    exit /b 1
)

echo Build OK: %OUT%

rem Pass "run" as the second argument to execute the program.
if /i "%~2"=="run" (
    echo.
    "%OUT%"
    exit /b %errorlevel%
)

exit /b 0
