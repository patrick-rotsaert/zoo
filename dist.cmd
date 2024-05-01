@echo off

::
:: Copyright (C) 2024 Patrick Rotsaert
:: Distributed under the Boost Software License, Version 1.0.
:: (See accompanying file LICENSE or copy at
:: http://www.boost.org/LICENSE_1_0.txt)
::

set THISDIR=%~dp0
set THISDIR=%THISDIR:~0,-1%

:: TODO: support command line options to:
::  - specify the build directory
::  - specify the generator
::  - specify the compiler

:: Check number of arguments
set argC=0
for %%x in (%*) do set /A argC+=1
if not %argC% == 1 (
	echo "Script must be invoked with architecture (x86|amd64) parameter"
	exit /B 1
)
set ARCH=%1

:: Find VS2019's VsDevCmd.bat
:: Unfortunately there is no VSxxxCOMNTOOLS variable like in VS2015 and earlier.
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if "%VSDEVCMD%" == "" (
	:: Try with vswhere.exe
	if exist "%VSWHERE%" (
		REM "%VSWHERE%" -format json -all
		for /f "delims=" %%i in ('"%VSWHERE%" -version 16 -property installationPath') do set "VS2019PATH=%%i"
		if not "%VS2019PATH%" == "" (
			set "VSDEVCMD=%VS2019PATH%\Common7\Tools\VsDevCmd.bat"
		)
	)
)

if "%VSDEVCMD%" == "" (
	:: Search for it
	if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019" (
		pushd "%ProgramFiles(x86)%\Microsoft Visual Studio\2019"
		for /f "delims=" %%i in ('dir /B VsDevCmd.bat /s') do set "VSDEVCMD=%%i"
		popd
	)
)

if "%VSDEVCMD%" == "" (
	echo "ERROR: Could not locate VsDevCmd.bat"
	exit /b 1
)

:: Source VsDevCmd.bat
call "%VSDEVCMD%" -arch=%ARCH%

:: cmake -DGENERATOR=Ninja -P "%THISDIR%"/dist/dist.cmake
cmake -DGENERATOR=Ninja -DCOMMAND_ECHO=STDERR -P "%THISDIR%"/dist/dist.cmake
