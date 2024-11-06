%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex(void);
void yyerror(char *);
extern int yylineno;
extern char* yytext;

// -----*******-----

int offset = 0;
int temp_offset = 0;
int isRegInUse[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

typedef struct _SymbolTable {
    char* name;
    int offset;
    struct _SymbolTable* next;
}SymbolTable;

typedef SymbolTable* SymbolTablePtr;

SymbolTablePtr ST_Head = NULL;

typedef struct valueType {
	enum valtype {
		memory_offset,
		num_value,
		register_value
	}type;
	int value;
}valueType;

SymbolTablePtr findID(SymbolTablePtr, char*);
SymbolTablePtr addIDtoST(SymbolTablePtr*, char*);
SymbolTablePtr findOrAddID(SymbolTablePtr*, char*);
void printST(SymbolTablePtr);

void fetchFromRegister(int, int);
void fetchToRegister(int, int, int, int);

SymbolTablePtr setIDNUM(SymbolTablePtr, char*, int);
SymbolTablePtr setIDID(SymbolTablePtr, char*, char*);
SymbolTablePtr setIDEXPR(SymbolTablePtr, char*, valueType*);

int getIDOffset(SymbolTablePtr, char*);
int getAvailableReg();

void selectArg(valueType*, char*, int);
valueType* encodeEXPR(int, valueType*, valueType*);
void standalone(valueType*);

%}

%union {int num; char p; char* txt; struct valueType* idx;}
%token <num> NUM PLUS MINUS MULT DIV MOD EXPO SET
%token RP LP
%token <txt> ID
%start program
%type <num> op 
%type <idx> arg expr exprstmt
%type setstmt stmt 

%%

program: 
         stmt program       { }
       | stmt               { }
       ;

stmt:
         setstmt            { }
       | exprstmt           { }
       ;

setstmt:
         LP SET ID NUM RP   { ST_Head = setIDNUM(ST_Head, $3, $4); }
       | LP SET ID ID RP    { ST_Head = setIDID(ST_Head, $3, $4); }
       | LP SET ID expr RP  { ST_Head = setIDEXPR(ST_Head, $3, $4); }
       ;

exprstmt:
         expr               { standalone($1); }
       ;

expr:
         LP op arg arg RP   { $$ = encodeEXPR($2, $3, $4); }
       ;

op:
         PLUS               { $$ = $1; }
       | MINUS              { $$ = $1; }
       | MULT               { $$ = $1; }
       | DIV                { $$ = $1; }
       | MOD                { $$ = $1; }
       | EXPO               { $$ = $1; }
       ;

arg:
         ID                 { SymbolTablePtr temp = findOrAddID(&ST_Head, $1); $$ = (valueType*)malloc(sizeof(valueType)); $$->type = memory_offset; $$->value = temp->offset; }
       | NUM                { $$ = (valueType*)malloc(sizeof(valueType)); $$->type = num_value; $$->value = $1; }
       | expr               { $$ = $1; }
       ;

%%

void yyerror(char * s) {
	printf("\tparsingError(%d);\n", yylineno);
	return;
}