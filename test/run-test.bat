@rem Optional: accept VS version as argument (e.g., 2019, 2022), or default to latest
set "VS_VERSION=%1"
set "ARCH=%2"

@rem Path to vswhere
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

@rem Check if vswhere exists
if not exist "%VSWHERE%" (
    echo ERROR: vswhere.exe not found!
    exit /b 1
)

@rem Use vswhere to find the installation path
if defined VS_VERSION (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -version %VS_VERSION% -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VSINSTALL=%%i"
    )
) else (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VSINSTALL=%%i"
    )
)

@rem Fallback check
if not defined VSINSTALL (
    echo ERROR: Could not find a matching Visual Studio installation!
    exit /b 1
)

@rem Call vcvarsall.bat with architecture (e.g., x64, x86, etc.)
call "%VSINSTALL%\VC\Auxiliary\Build\vcvarsall.bat" %ARCH%

nmake /f Makefile.win32 check clean DLL_CFLAGS=%3 EXE_CFLAGS=%4
