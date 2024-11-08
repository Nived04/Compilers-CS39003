%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex(void);
void yyerror(char *);
extern int yylineno;
extern char* yytext;

int temp_gen_count = 0, instruction_count = 0;
char t[5];

typedef struct _SymbolTable {
	char* name;
    int offset;
	int reg_locs; 
	int mem_locs;
	int isInSync;
	struct _SymbolTable* next;
}SymbolTable;

typedef struct quadruple {
	char *op, *arg1, *arg2, *res;
}quadruple;

typedef struct quadTable {
	quadruple* quad;
	struct quadTable* next;
}quadTable;

typedef struct name_list { 
    char* var_name;
    struct name_list* next;
}name_list;

typedef struct _register {
	int score;
    name_list reg_descriptor;
}reg;

SymbolTable* ST_Head = NULL;
quadTable* Q_Head = NULL;

char* generateTemp();
void setID(char*, char*);

void printST();

SymbolTable findID(char*);
SymbolTable addIDtoST(char*);
SymbolTable findOrAddID(char*);

%}

%union {int num; char* text; Symbol* sym;}

%token LP RP SET WHEN LOOP EQ NOT_EQ LT GT LE GE INVALID_TOKEN
%token <num> PLUS MINUS MULT DIV MOD
%token <text> IDEN NUMB

%start prog
%type stmt asgn cond loop bool 
%type <text> atom expr reln
%type <num> oper M

%%

prog: 	
	  list		{ instruction_count = 1; }
	;

list:
      stmt		{}
    | stmt list	{}
	;

stmt:
      asgn 		{}
    | cond		{}
    | loop		{}
	;

asgn:
	  LP SET IDEN atom RP			{ ST_Head = setID(ST_Head, $3, $4); emit("=", $4, NULL, $3); }
	;

cond:
	  LP WHEN bool M list RP		{ backpatch($4, instruction_count+1); }
	;


loop:
	  LP LOOP bool M list M RP		{ backpatch($4, instruction_count+1); backpatch($6, $4); }
	;

M:	
	  { emit("goto", NULL, NULL, NULL); }
	; 

expr: 
	  LP oper atom atom RP		{ emit("=", $3, $4, generateTemp()); }
	; 

bool:
	  LP reln atom atom RP		{ emit($2, $3, $4, "iffalse"); }
	;

atom:
	  IDEN  	{ findorAddID(ST_Head, $1); }
	| NUMB		{ $$=$1; }
	| expr		{ $$=$1; }
	;

oper:
	  PLUS		{ $$=$1; } 
	| MINUS		{ $$=$1; }
	| MULT		{ $$=$1; }
	| DIV		{ $$=$1; }
	| MOD		{ $$=$1; }
	;

reln:
	  EQ		{ $$ = strdup("=="); }
	| NOT_EQ	{ $$ = strdup("!="); }
	| LT		{ $$ = strdup("<"); }
	| GT		{ $$ = strdup(">"); }
	| LE		{ $$ = strdup("<="); }
	| GE		{ $$ = strdup(">="); }
	;


%%

void yyerror(char* message) {
    printf("*** Error on line: %d", yylineno);
}

char* generateTemp() {
	sprintf(t, "$%d", ++temp_gen_count);
	return t;
}

void printST() {
	SymbolTable* iter = ST_Head;
	while(iter != NULL) {
		printf("%s\n", iter->symbol->name);
		iter = iter->next;
	}
	return;
}

// searches for a particular identifier in the symbol table
SymbolTable* findID(SymbolTable* st_head, char* id) {
    SymbolTable* temp = st_head;
    while(temp != NULL) {
        if( strcmp(temp->name, id) == 0) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

// adds an identifier to the symbol table
SymbolTable* addIDtoST(SymbolTable** st_head, char* id) {
    SymbolTable* new_node = (SymbolTable*)malloc(sizeof(SymbolTable));
    new_node->name = strdup(id); 
    new_node->offset = offset++;
    new_node->next = NULL;

    // If the head is NULL, this is the first node
    if (*st_head == NULL) {
        *st_head = new_node;
    } 
    else {
        SymbolTable* temp = *st_head;
        while(temp->next != NULL) {
            temp = temp->next;
        }
        // Add new node to the end
        temp->next = new_node;
    }
    return new_node;
}

// Function to find an identifier in the symbol table, if not found, add it
SymbolTable* findOrAddID(SymbolTable** st_head, char* id) {
    SymbolTable* temp = findID(*st_head, id);
    if (!temp) 
        temp = addIDtoST(st_head, id);
    return temp;
}

SymbolTable* setID(SymbolTable** st_head, char* id, char* rid) {
    SymbolTable* right_ID = findOrAddID(st_head, rid);
    SymbolTable* left_ID = findOrAddID(st_head, id);
    return *st_head;
}

void emit(char* op, char* arg1, char* arg2, char* res) {
	instruction_count++;
	quadTable* new_quad = (quadTable*)malloc(sizeof(quadTable));
	new_quad->quad = (quadruple*)malloc(sizeof(quadruple));

	new_quad->quad->op = (op == NULL) ? NULL : strdup(op);
	new_quad->quad->arg1 = (arg1 == NULL) ? NULL : strdup(arg1);
	new_quad->quad->arg2 = (arg2 == NULL) ? NULL : strdup(arg2);
	new_quad->quad->res = (res == NULL) ? NULL : strdup(res);

	new_quad->next = NULL;

	if(Q_Head == NULL) {
		Q_Head = new_quad;
	}
	else {
		quadTable* iter = Q_Head;
		while(iter->next != NULL) {
			iter = iter->next;
		}
		iter->next = new_quad;
	}
}

void backpatch(int addr, int val) {
	
}