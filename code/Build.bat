@echo off

if not defined DevEnvDir (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
)

if not exist ..\build mkdir ..\build
pushd ..\build
cl -DMAUSTRAP_RC=0 -DMAUSTRAP_SLOW=1 -FC -Zi ..\code\win32_MalEngine.cpp user32.lib Gdi32.lib
popd