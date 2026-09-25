@echo off
rem ============================================================
rem  Build script for the Win32 GUI version of the project.
rem
rem  Keep this file pure ASCII: cmd.exe reads .bat files using the
rem  ANSI code page, so non-ASCII bytes get mangled and executed.
rem
rem  Flags:
rem    -municode  : Unicode entry point (wWinMain)
rem    -mwindows  : GUI subsystem, no console window
rem    -static    : no external libstdc++/libgcc DLL needed
rem    -lcomctl32 : InitCommonControlsEx / ListView
rem    -lgdi32    : CreateFontW / DeleteObject / BitBlt
rem
rem  app.res is compiled from app.rc + app.manifest and embedded via
rem  the link command (windres itself is not needed at runtime); it
rem  enables ComCtl32 v6 visual styles and UTF-8 active code page.
rem
rem  Usage:  build_gui.bat
rem ============================================================

setlocal
cd /d "%~dp0"

set GXX=C:\msys64\ucrt64\bin\g++.exe
set WINDRES=C:\msys64\ucrt64\bin\windres.exe
set OUT=book_gui.exe

echo Building GUI version...

rem ??????? + ???
if exist app.rc (
    "%WINDRES%" -I. app.rc -O coff -o app.res
    if errorlevel 1 (
        echo WARNING: windres failed, building without manifest.
    )
) else (
    echo WARNING: app.rc not found, building without manifest.
)

set RES=
if exist app.res set RES=app.res

"%GXX%" -std=c++17 -O2 -Wall -Wextra -municode -mwindows -static -static-libgcc -static-libstdc++ *.cpp %RES% -o "%OUT%" -lcomctl32 -lgdi32
if errorlevel 1 (
    echo.
    echo BUILD FAILED.
    exit /b 1
)

echo Build OK: %OUT%
exit /b 0
