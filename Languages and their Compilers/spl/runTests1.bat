@ECHO on

flex spl.l
bison spl.y
gcc -o spl.exe spl.tab.c spl.c -lfl

@ECHO off

FOR %%i IN (a b c d e helloworld) DO (
	ECHO %%i: spl.exe ^< Tests1\%%i.spl ^> Tests1\%%i.c
	ECHO %%i: gcc -o Tests1\%%i.exe Tests1\%%i.c
	
	@ECHO on
	spl.exe < Tests1\%%i.spl > Tests1\%%i.c
	gcc -o Tests1\%%i.exe Tests1\%%i.c
	Tests1\%%i.exe
	ECHO;
	@ECHO off
)