@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64

if exist qjulia.exe del qjulia.exe
if exist *.pdb del *.pdb
if exist *.obj del *.obj

fasm\fasm.exe qjulia_asmlib.asm
if errorlevel 1 goto eof

:: /Fm generates linker map file
:: /Fa generates assembly listing
cl /Zi /Ox /arch:AVX2 /Gm- /nologo /WX /W3 /GS- /Gs999999 /Gy /Gw /EHa- qjulia.c ^
   /link qjulia_asmlib.obj kernel32.lib user32.lib gdi32.lib /OPT:REF /INCREMENTAL:NO /SUBSYSTEM:WINDOWS /ENTRY:start /NODEFAULTLIB

if exist vc140.pdb del vc140.pdb
if exist *.obj del *.obj

:eof
