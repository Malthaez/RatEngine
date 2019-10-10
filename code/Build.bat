@echo off

if not defined DevEnvDir (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x86
)

REM TODO - Can we just build both with one exe?

if not exist ..\build mkdir ..\build
pushd ..\build
cl -nologo -GR- -EHa- -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -D_DEBUG=1 -DRATENGINE_SLOW=1 -FC -Z7 ..\code\win32_MalEngine.cpp user32.lib Gdi32.lib
popd