@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64

del qjulia.exe *.pdb *.obj

cl /Zi /Ox /nologo /WX /D_CRT_SECURE_NO_WARNINGS /W3 /EHa- qjulia_windows.cpp ^
   /link kernel32.lib user32.lib gdi32.lib ^
   /OPT:REF /INCREMENTAL:NO /SUBSYSTEM:CONSOLE /OUT:qjulia.exe

del vc140.pdb *.obj

:eof
