@echo off

rem sync.cmd target
rem sync.cmd source target
rem example: sync.cmd C:\Projects\MyGame
rem example: sync.cmd C:\Projects\DriftUe4Plugin C:\Projects\MyGame

pushd %~dp0
setlocal
if "%2" == "" (
	set source=%~dp0
	set dest=%1
	shift
) else (
	set source=%1
	set dest=%2
	shift
	shift
)
robocopy /mir /l %source%\Plugins %dest%\Plugins -xd Binaries -xd Resources -xd Intermediate
robocopy /mir /l %source%\Source\Drift %dest%\Source\Drift
popd
