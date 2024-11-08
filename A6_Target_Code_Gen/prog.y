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

SymbolTable* findID(char*);
SymbolTable* addIDtoST(char*);
SymbolTable* findOrAddID(char*);

void print_IntCode();

void emit(char*, char*, char*, char*);

void backpatch(int, int); 

%}

%union {int num; char* text;}

%token LP RP SET WHEN LOOP EQ NOT_EQ LT GT LE GE INVALID_TOKEN
%token <num> PLUS MINUS MULT DIV MOD
%token <text> IDEN NUMB

%start prog
%type stmt asgn cond loop bool 
%type <text> atom expr reln
%type <num> oper M N

%%

prog: 	
	  list		{ instruction_count++; }
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
	  LP SET IDEN atom RP			
	  { 
		setID($3, $4); 
		emit("=", $4, NULL, $3); 
	  }
	;

cond:
	  LP WHEN bool M list RP		{ backpatch($4+2, instruction_count+1); }
	;


loop:
	  LP LOOP bool M list N RP		{ backpatch($4+1, instruction_count+1); backpatch($6+2, $4); }
	;

M:	
	  { emit("goto", NULL, NULL, NULL); $$=(--instruction_count); }
	; 

N: 
	  { emit("goto", NULL, NULL, NULL); $$=instruction_count; }
	;

expr: 
	  LP oper atom atom RP		{ $$ = generateTemp(); char s[1] = {(char)$2}; emit(s, $3, $4, $$); }
	; 

bool:
	  LP reln atom atom RP		{ emit($2, $3, $4, "iffalse"); }
	;

atom:
	  IDEN  	{ findOrAddID($1); }
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
		printf("%s\n", iter->name);
		iter = iter->next;
	}
	return;
}

// searches for a particular identifier in the symbol table
SymbolTable* findID(char* id) {
    SymbolTable* temp = ST_Head;
    while(temp != NULL) {
        if( strcmp(temp->name, id) == 0) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

// adds an identifier to the symbol table
SymbolTable* addIDtoST(char* id) {
    SymbolTable* new_node = (SymbolTable*)malloc(sizeof(SymbolTable));
    new_node->name = strdup(id); 
    new_node->next = NULL;

    // If the head is NULL, this is the first node
    if (ST_Head == NULL) {
        ST_Head = new_node;
    } 
    else {
        SymbolTable* temp = ST_Head;
        while(temp->next != NULL) {
            temp = temp->next;
        }
        // Add new node to the end
        temp->next = new_node;
    }
    return new_node;
}

// Function to find an identifier in the symbol table, if not found, add it
SymbolTable* findOrAddID(char* id) {
    SymbolTable* temp = findID(id);
    if (!temp) 
        temp = addIDtoST(id);
    return temp;
}

void setID(char* id, char* rid) {
    SymbolTable* right_ID = findOrAddID(rid);
    SymbolTable* left_ID = findOrAddID(id);
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

void backpatch(int from, int target) {
	quadTable* iter = Q_Head;
	int inst = 1;

	while(iter != NULL) {
		if(inst == from) {
			char buff[10];
			sprintf(buff, "%d", target);
			/* printf("** TEST : %d, %s, %d, %s\n", inst, buff, target, iter->quad->op); */
			iter->quad->res = strdup(buff);
			return;
		}
		iter = iter->next;
		inst++;
	}	
}

void print_IntCode() {
	quadTable* iter = Q_Head;
	int inst = 0;

	while(iter != NULL) {
		inst++;
		printf("\t%d\t: ", inst);
		if( strcmp(iter->quad->res, "iffalse") == 0) {
			printf("%s (%s %s %s) ", iter->quad->res, iter->quad->arg1, iter->quad->op, iter->quad->arg2);
			iter = iter->next;
			printf("goto %s\n", iter->quad->res);
		}
		else if( strcmp(iter->quad->op, "goto") == 0) {
			printf("%s %s\n", iter->quad->op, iter->quad->res);
		}
		else {
			if(iter->quad->arg2 == NULL) {
				printf("%s = %s\n", iter->quad->res, iter->quad->arg1);
			}
			else {
				printf("%s = %s %s %s\n", iter->quad->res, iter->quad->arg1, iter->quad->op, iter->quad->arg2);
			}
		}
		iter = iter->next;
	}
}