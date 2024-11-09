%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex(void);
void yyerror(char *);
extern int yylineno;
extern char* yytext;

int temp_gen_count = 0, instruction_count = 0;
int block_leaders[1000]; // assuming at most 1000 instructions can be given
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
	int inst_no;
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

void emit(char*, char*, char*, char*, int);

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
		emit("=", $4, NULL, $3, 1); 
	  }
	;

cond:
	  LP WHEN bool M list RP		{ backpatch($4, instruction_count+1); }
	;

loop:
	  LP LOOP bool M list N RP		{ backpatch($4, instruction_count+1); backpatch($6, $4); }
	;

M:	
	  { $$=instruction_count; emit("goto", NULL, NULL, NULL, 0); block_leaders[instruction_count+1] = 1; }
	; 

N: 
	  { emit("goto", NULL, NULL, NULL, 1); $$=instruction_count; block_leaders[instruction_count+1] = 1; }
	;

expr: 
	  LP oper atom atom RP		{ $$ = generateTemp(); char s[1] = {(char)$2}; emit(s, $3, $4, $$, 1); }
	; 

bool:
	  LP reln atom atom RP		{ emit($2, $3, $4, "iffalse", 1); }
	;

atom:
	  IDEN  	{ findOrAddID($1); $$ = strdup($1); }
	| NUMB		{ $$=strdup($1); }
	| expr		{ $$=strdup($1); }
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

void emit(char* op, char* arg1, char* arg2, char* res, int flag) {
	if(flag) {
		instruction_count++;
	}
	quadTable* new_quad = (quadTable*)malloc(sizeof(quadTable));
	new_quad->quad = (quadruple*)malloc(sizeof(quadruple));
	new_quad->quad->inst_no = instruction_count;
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
	block_leaders[target] = 1;
	quadTable* iter = Q_Head;
	/* printf("from: %d, target: %d\n", from, target); */
	while(iter != NULL) {
		char buff[10];
		sprintf(buff, "%d", target);
		if((iter->quad->op != NULL) && (strcmp(iter->quad->op, "goto") == 0) && (iter->quad->inst_no == from)) {
			/* printf("HELLO\n"); */
			iter->quad->res = strdup(buff);
			return;
		}
		iter = iter->next;
	}	
}

void print_IntCode() {
	quadTable* iter = Q_Head;
	int inst = 0;
	int block_count = 1;
	printf("Block 1\n");
	while(iter != NULL) {
		inst++;
		if(block_leaders[inst] == 1) {
			printf("\nBlock %d\n", ++block_count);
		}
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
