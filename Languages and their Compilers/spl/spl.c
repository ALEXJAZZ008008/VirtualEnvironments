#include <stdio.h>
#include <stdlib.h>

int yyparse(void);

int main(void)
{
	#if YYDEBUG == 1
		extern int yydebug;
		yydebug = 1;
	#endif
	
	return(yyparse());
}

void yyerror(char * error)
{
	fprintf(stderr, "Error %s: Exiting\n", error);
	exit(0);
}