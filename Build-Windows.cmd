@echo off
setlocal
cd /d "%~dp0"
where cl >nul 2>nul || (echo MSVC not found & exit /b 1)
cl /nologo /utf-8 /O2 /W4 /TC /Fe:OledBlackScreen.exe OledBlackScreen.c /link /SUBSYSTEM:WINDOWS user32.lib shell32.lib advapi32.lib gdi32.lib
if errorlevel 1 exit /b 1
echo Built OledBlackScreen.exe
