@echo off

set THISDIR=%~dp0
set THISDIR=%THISDIR:~0,-1%

call "%THISDIR%"\dist.cmd x86

set EXITCODE=%ERRORLEVEL%
if %0 == "%~0" pause
exit /b %EXITCODE%
