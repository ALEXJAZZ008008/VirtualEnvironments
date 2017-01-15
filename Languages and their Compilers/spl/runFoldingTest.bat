@ECHO on

flex spl.l
bison spl.y
gcc -o spl.exe -DOPTIMISATION spl.tab.c spl.c -lfl

@ECHO off

ECHO foldingTest: spl.exe ^< foldingTest.spl
ECHO foldingTest: spl.exe ^< foldingTest.spl ^> foldingTest.c
ECHO foldingTest: gcc -o foldingTest.exe foldingTest.c

@ECHO on
spl.exe < foldingTest.spl
spl.exe < foldingTest.spl > foldingTest.c
gcc -o foldingTest.exe foldingTest.c
foldingTest.exe
@ECHO;
@ECHO off