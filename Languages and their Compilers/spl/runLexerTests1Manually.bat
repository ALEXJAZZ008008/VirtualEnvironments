@ECHO on

flex spl.l
gcc -o spl.exe -DPRINT -DYY_MAIN lex.yy.c -lfl

@ECHO off

FOR %%i IN (a b c d e helloworld) DO (
	ECHO %%i: spl.exe ^< Tests1\%%i.spl
	
	@ECHO on
	spl.exe < Tests1\%%i.spl
	PAUSE
	ECHO;
	@ECHO off
)