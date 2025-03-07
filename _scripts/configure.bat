@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION
CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

IF NOT EXIST _build MKDIR _build > NUL 2> NUL
PUSHD _build
cmake.exe -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DOBJECT_SYSTEM_TESTS=ON ..
MOVE compile_commands.json ..\compile_commands.json > NUL 2> NUL
POPD
