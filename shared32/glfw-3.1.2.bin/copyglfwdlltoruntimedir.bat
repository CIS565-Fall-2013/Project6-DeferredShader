if "%1"=="vs2013" ( 
	set srcdir=lib-vc2013 
	set destdir=runtime 
) else (
	if "%1"=="vs2015" ( 
		set srcdir=lib-vc2015 
		set destdir=runtime_2015 
	) else (
		echo Incorrect usage. Only vs2013 or vs2015 supported.
		exit 1
	)
)

call robocopy ..\..\..\..\shared32\glfw-3.1.2.bin\%srcdir% ..\..\..\%destdir% glfw3.dll
if %ERRORLEVEL% GTR 8 ( exit 1 ) else ( exit 0 )