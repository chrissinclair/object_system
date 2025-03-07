@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION
CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

CALL "%~dp0\build.bat"
PUSHD _build
cmake --build . --target test
SET result=%ERRORLEVEL%
IF %result% NEQ 0 (
    TYPE "Testing\Temporary\LastTest.log"
)
POPD
EXIT %result%
