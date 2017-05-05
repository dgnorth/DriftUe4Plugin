pushd %~dp0
setlocal
if "%2" == "" (
	set source=%~dp0\Plugins
	set dest=%1\Plugins
	shift
) else (
	set source=%1\Plugins
	set dest=%2\Plugins
	shift
	shift
)
robocopy /mir /l %source% %dest% -xd Binaries -xd Resources -xd Intermediate
popd

