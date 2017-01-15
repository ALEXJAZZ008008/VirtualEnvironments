delim	[ \t\r]
ws	{delim}+
digit	[0-9]
number	{digit}+

%%

{ws}	;
\n	return(NEWLINE);

"+"	return(PLUS);
"-"	return(MINUS);

"*"	return(TIMES);
"/"	return(DIVIDE);

"("	return(BRA);
")"	return(KET);

{number}	{
				yylval.iVal = atoi(yytext);
				return(NUMBER);
			}

%%