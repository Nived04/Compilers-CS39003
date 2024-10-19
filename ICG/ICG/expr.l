%{

#include <stdlib.h>
#include <string.h>
#include "y.tab.h"
void yyerror();
int yylex();

%}

WS              [ \t]
newLine         [\n]

PLUS            "+"
MINUS           "-"
MULT            \*
DIV             \/
MOD             \%
EXPO            \*\*

DIGIT           [0-9]
NON_ZERO        [1-9]
NUM             [-+]?({NON_ZERO}{DIGIT}*|0)
LETTER          [a-zA-Z_]
ALPHA_NUM       {LETTER}|{DIGIT}
ID              {LETTER}{ALPHA_NUM}*

SET             "set"
LP              "("
RP              ")"

%option noyywrap

%%

{PLUS}          { yylval.num = (int)yytext[0]; return PLUS; }
{MINUS}         { yylval.num = (int)yytext[0]; return MINUS; }
{MULT}          { yylval.num = (int)yytext[0]; return MULT; }
{DIV}           { yylval.num = (int)yytext[0]; return DIV; }
{MOD}           { yylval.num = (int)yytext[0]; return MOD; }
{EXPO}          { yylval.num = EXPO; return EXPO; } 
{SET}           { yylval.num = SET; return SET; }
{NUM}           { yylval.num = atoi(yytext); return NUM; }
{ID}            { yylval.txt = strdup(yytext); return ID; }
{LP}            { return LP; }
{RP}            { return RP; }
{WS}            {}
{newLine}       { yylineno++; }
.               {}

%%



