flex spl.l
bison spl.y
gcc -o spl.exe -DOPTIMISATION spl.tab.c spl.c -lfl