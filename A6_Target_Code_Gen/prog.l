%{

#include "y.tab.h"
void yyerror();
int yylex();

%}

WS          [ \t]
newLine     [\n]

PLUS        "+"
MINUS       "-"
MULT        "*"
DIV         "/"
MOD         "%"

EQ          "="
NOT_EQ      "/="
LT          "<"
GT          ">"
LE          "<="
GE          ">="

DIGIT       [0-9]
NON_ZERO    [1-9]
NUMB        [-+]?({NON_ZERO}{DIGIT}*|0)
LETTER      [a-zA-Z_]
ALPHA_NUM   {LETTER}|{DIGIT}
IDEN        {LETTER}{ALPHA_NUM}*

SET         "set"
LP          "("
RP          ")"

LOOP        "loop while"
WHEN        "when"

%option noyywrap

%%

{WS}        {/* Ignore whitespaces */}

{PLUS}      {yylval.num = (int)yytext[0]; return PLUS;}
{MINUS}     {yylval.num = (int)yytext[0]; return MINUS;}
{MULT}      {yylval.num = (int)yytext[0]; return MULT;}
{DIV}       {yylval.num = (int)yytext[0]; return DIV;}
{MOD}       {yylval.num = (int)yytext[0]; return MOD;}

{EQ}        {return EQ;}
{NOT_EQ}    {return NOT_EQ;}
{LT}        {return LT;}
{GT}        {return GT;}
{LE}        {return LE;}
{GE}        {return GE;}

{NUMB}      {yylval.text = strdup(yytext); return NUMB;}
{SET}       {return SET;}

{LP}        {return LP;}
{RP}        {return RP;}

{LOOP}      {return LOOP;}
{WHEN}      {return WHEN;}

{IDEN}      {yylval.text = strdup(yytext); return IDEN;}

{newLine}   {yylineno++;}
.           {return INVALID_TOKEN;}

%%