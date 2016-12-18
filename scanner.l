%{

#include <stdlib.h>
#include <string.h>
#include "Node.h"
#include "parser.tab.h"

%}
%%
"const" return(tconst);
"else" return(telse);
"if" return(tif);
"int" return(tint);
"return" return(treturn);
"void" return(tvoid);
"while" return(twhile);
"==" return(tequal);
"!=" return(tnotequ);
"<=" return(tlesse);
">=" return(tgreate);
"&&" return(tand);
"||" return(tor);
"++" return(tinc);
"--" return(tdec);
"+=" return(taddAssign);
"-=" return(tsubAssign);
"*=" return(tmulAssign);
"/=" return(tdivAssign);
"%=" return(tmodAssign);
[A-Za-z_][A-Za-z0-9_]* {
		int length = strlen(yytext);
		char* temp = (char*) malloc(length + 1);
		memset(temp, 0, length+1);
		strcpy(temp, yytext);
		yylval.string = temp;
		return(tident);
	}
[1-9][0-9]*|0([0-7]+|(x|X)[0-9A-Fa-f]*)? {
		int length = strlen(yytext);
		char* temp = (char*) malloc(length + 1);
		memset(temp, 0, length+1);
		strcpy(temp, yytext);
		yylval.string = temp;
		return(tnumber);
	}
"/*"([^*]|\*+[^*/])*\**"*/" ;
"//".* ;
[ \t\n] ;
. return(yytext[0]);
%%
int yywrap(){
  return 1;
}
