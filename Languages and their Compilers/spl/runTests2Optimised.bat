@ECHO on

flex spl.l
bison spl.y
gcc -o spl.exe -DOPTIMISATION spl.tab.c spl.c -lfl

@ECHO off

FOR /L %%i IN (1,1,9) DO (
	ECHO %%i: spl.exe ^< Tests2\Test00%%i.spl ^> Tests2\Test00%%i.c
	ECHO %%i: gcc -o Tests2\Test00%%i.exe Tests2\Test00%%i.c
	
	@ECHO on
	spl.exe < Tests2\Test00%%i.spl > Tests2\Test00%%i.c
	gcc -o Tests2\Test00%%i.exe Tests2\Test00%%i.c
	Tests2\Test00%%i.exe
	ECHO;
	@ECHO off
)

FOR /L %%i IN (10,1,99) DO (
	ECHO %%i: spl.exe ^< Tests2\Test0%%i.spl ^> Tests2\Test0%%i.c
	ECHO %%i: gcc -o Tests2\Test0%%i.exe Tests2\Test0%%i.c
	
	@ECHO on
	spl.exe < Tests2\Test0%%i.spl > Tests2\Test0%%i.c
	gcc -o Tests2\Test0%%i.exe Tests2\Test0%%i.c
	Tests2\Test0%%i.exe
	ECHO;
	@ECHO off
)

FOR /L %%i IN (100,1,160) DO (
	ECHO %%i: spl.exe ^< Tests2\Test%%i.spl ^> Tests2\Test%%i.c
	ECHO %%i: gcc -o Tests2\Test%%i.exe Tests2\Test%%i.c
	
	@ECHO on
	spl.exe < Tests2\Test%%i.spl > Tests2\Test%%i.c
	gcc -o Tests2\Test%%i.exe Tests2\Test%%i.c
	Tests2\Test%%i.exe
	ECHO;
	@ECHO off
)