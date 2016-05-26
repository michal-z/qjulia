@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64

if exist qjulia.exe del qjulia.exe
if exist *.pdb del *.pdb
if exist *.obj del *.obj

cl /Zi /Ox /DNDEBUG /nologo /WX /D_CRT_SECURE_NO_WARNINGS /W3 /EHa- qjulia_windows.cpp ^
   /link kernel32.lib user32.lib gdi32.lib ^
   /OPT:REF /INCREMENTAL:NO /SUBSYSTEM:CONSOLE /OUT:qjulia.exe

if exist vc140.pdb del vc140.pdb
if exist *.obj del *.obj

:eof
