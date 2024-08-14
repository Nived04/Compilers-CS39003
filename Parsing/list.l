%{

#define LP 1
#define RP 2
#define OP 3
#define ID 4
#define INTEGER 5
#define NL 10
#define OPS 11

%}

WS          [ \t] 
NL          [\n] 
DIGIT       [0-9]
NUM         {DIGIT}+
NEG_NUM     \-{NUM}
INTEGER     {NUM}|{NEG_NUM}
OP          \+|\-|\*|\/|\%
ID          [a-zA-Z_][a-zA-Z0-9_]*
OPS         "&"|"|"|"~"|"!"|"^"|"<<"|">>"|"="|"&&"|"||"|"<"|">"|"<="|">="|"=="|"!="|"++"|"--"|"+="|"-="|"*="|"/="
LP          \(
RP          \)

%%

{LP}        return LP;
{RP}        return RP;
{OP}        return OP;
{OPS}       return OPS;
{ID}        return ID;
{INTEGER}   return INTEGER;
{NL}        return NL;
{WS}        {}
.           {}

%%

int yywrap(void) {
    return 1;
}