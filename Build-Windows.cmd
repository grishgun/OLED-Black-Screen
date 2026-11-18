@echo off
setlocal
cd /d "%~dp0"
where cl >nul 2>nul || (echo MSVC C compiler not found. Use x64 Native Tools Command Prompt. & exit /b 1)
where rc >nul 2>nul || (echo Windows SDK resource compiler not found. & exit /b 1)
rc /nologo OledBlackScreen.rc || exit /b 1
cl /nologo /utf-8 /O2 /MT /W4 /TC /c OledBlackScreen.c || exit /b 1
link /nologo /MACHINE:X64 /SUBSYSTEM:WINDOWS /OUT:OledBlackScreen.exe OledBlackScreen.obj OledBlackScreen.res user32.lib shell32.lib advapi32.lib gdi32.lib || exit /b 1
echo Built OledBlackScreen.exe
