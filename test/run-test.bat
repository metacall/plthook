@echo off
setlocal enabledelayedexpansion

@rem Accept VS version (e.g., 16 for 2019, 17 for 2022)
set "VS_VERSION=%1"
set "ARCH=%2"

@rem Path to vswhere
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

@rem Check if vswhere exists
if not exist "%VSWHERE%" (
    echo ERROR: vswhere.exe not found!
    exit /b 1
)

@rem Initialize VSINSTALL to empty
set "VSINSTALL="

@rem Get Visual Studio install path
if defined VS_VERSION (
    for /f "usebackq delims=" %%i in (`"%VSWHERE%" -products * -version [%VS_VERSION%.0,%VS_VERSION%.999] -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath -nologo`) do (
        set "VSINSTALL=%%i"
    )
) else (
    for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath -nologo`) do (
        set "VSINSTALL=%%i"
    )
)

@rem Check if we found anything
if not defined VSINSTALL (
    echo ERROR: Could not find Visual Studio installation with required components.
    exit /b 1
)

@rem Show what we found
echo Found Visual Studio at: %VSINSTALL%

@rem Call vcvarsall.bat
call "%VSINSTALL%\VC\Auxiliary\Build\vcvarsall.bat" %ARCH%

@rem Build the test
nmake /f Makefile.win32 check clean DLL_CFLAGS=%3 EXE_CFLAGS=%4

endlocal
