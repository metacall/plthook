call "c:\Program Files (x86)\Microsoft Visual Studio\%1\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" %2
nmake /f Makefile.win32 check clean DLL_CFLAGS=%3 EXE_CFLAGS=%4
