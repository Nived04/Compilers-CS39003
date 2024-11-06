%{

#include <stdlib.h>
#include <string.h>
#include "y.tab.h"
void yyerror();
int yylex();

%}

WS          [ \t]
newLine     [\n]
PLUS        "+"
MINUS       "-"
EXPO        "^"
ZERO        "0"
ONE         "1"
DIGIT       [2-9]
VAR         "x"

%option noyywrap

%%

{PLUS}      { yylval.num = (int)yytext[0];  return PLUS; }
{MINUS}     { yylval.num = (int)yytext[0]; return MINUS; }
{EXPO}      { yylval.num = (int)yytext[0]; return EXPO; }
{ZERO}      { yylval.num = 0; return ZERO; }
{ONE}       { yylval.num = 1; return ONE; }
{DIGIT}     { yylval.num = atoi(yytext); return DIGIT; }
{VAR}       { yylval.num = (int)yytext[0]; return VAR; }

{WS}        {}
{newLine}   { yylineno++; }

.           {}

%%


