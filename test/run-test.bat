if "%1" == "2019" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" %2
) else (
    call "C:\Program Files\Microsoft Visual Studio\%1\Community\VC\Auxiliary\Build\vcvarsall.bat" %2
)
nmake /f Makefile.win32 check clean DLL_CFLAGS=%3 EXE_CFLAGS=%4
