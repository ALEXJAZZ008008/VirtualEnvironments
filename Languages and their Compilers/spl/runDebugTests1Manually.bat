@ECHO on

flex spl.l
bison spl.y
gcc -o spl.exe -DDEBUG spl.tab.c spl.c -lfl

@ECHO off

FOR %%i IN (a b c d e helloworld) DO (
	ECHO %%i: spl.exe ^< Tests1\%%i.spl
	
	@ECHO on
	spl.exe < Tests1\%%i.spl
	PAUSE
	ECHO;
	@ECHO off
)