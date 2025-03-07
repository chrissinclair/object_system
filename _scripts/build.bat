@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION
CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

IF NOT EXIST _build (
    CALL "%~dp0\configure.bat"
)

PUSHD _build
cmake --build .
POPD

