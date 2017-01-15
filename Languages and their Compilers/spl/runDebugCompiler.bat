flex spl.l
bison spl.y
gcc -o spl.exe -DDEBUG spl.tab.c spl.c -lfl