@ECHO on

flex spl.l
bison spl.y
gcc -o spl.exe -DDEBUG spl.tab.c spl.c -lfl

@ECHO off

FOR /L %%i IN (1,1,9) DO (
	ECHO %%i: spl.exe ^< Tests2\Test00%%i.spl
	
	@ECHO on
	spl.exe < Tests2\Test00%%i.spl
	PAUSE
	ECHO;
	@ECHO off
)

FOR /L %%i IN (10,1,99) DO (
	ECHO %%i: spl.exe ^< Tests2\Test0%%i.spl
	
	@ECHO on
	spl.exe < Tests2\Test0%%i.spl
	PAUSE
	ECHO;
	@ECHO off
)

FOR /L %%i IN (100,1,160) DO (
	ECHO %%i: spl.exe ^< Tests2\Test%%i.spl
	
	@ECHO on
	spl.exe < Tests2\Test%%i.spl
	PAUSE
	ECHO;
	@ECHO off
)