%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_REG 5
#define EQUATE 11
#define GOTO 10
#define JUMP 6
#define LDI 7
#define LD 8
#define ST 9

int yylex(void);
void yyerror(char *);
extern int yylineno;
extern char* yytext;

int temp_gen_count = 0, instruction_count = 0, target_inst_count = 0;
int block_begin[1000]; // assuming at most 1000 instructions can be given
int block_leaders[1000];
int inst_to_target[1000];
char* relation_op[6] = {"==", "!=", "<", ">", "<=", ">="};
char* relation_target[10] = {"JNE", "JEQ", "JGE", "JLE", "JGT", "JLT", "JMP", "LDI", "LD", "ST"};

char t[5];

typedef struct _SymbolTable {
	char* name;
    int offset;
	int reg_locs; 
	int mem_locs;
	int isInSync;
	int isLive;
	struct _SymbolTable* next;
}SymbolTable;

typedef struct quadruple {
	int inst_no;
	int op;
	char *arg1, *arg2, *res;
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
    name_list* reg_descriptor;
}reg;

reg RegBank[MAX_REG];

SymbolTable* ST_Head = NULL;
quadTable* Q_Head = NULL;
quadTable* T_Head = NULL;

char* generateTemp();
void setID(char*, char*);

void printST();

SymbolTable* findID(char*);
SymbolTable* addIDtoST(char*);
SymbolTable* findOrAddID(char*);

void print_IntCode();

void emit(int, char*, char*, char*, int);

void backpatch(int, int); 

%}

%union {int num; char* text;}

%token LP RP SET WHEN LOOP INVALID_TOKEN
%token <num> PLUS MINUS MULT DIV MOD EQ NOT_EQ LT GT LE GE 
%token <text> IDEN NUMB

%start prog
%type stmt asgn cond loop bool 
%type <text> atom expr 
%type <num> oper M N reln

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
		emit(EQUATE, $4, NULL, $3, 1); 
	  }
	;

cond:
	  LP WHEN bool M list RP		{ backpatch($4, instruction_count+1); }
	;

loop:
	  LP LOOP bool M list N RP		{ backpatch($4, instruction_count+1); backpatch($6, $4); }
	;

M:	
	  { $$=instruction_count; emit(GOTO, NULL, NULL, NULL, 0); block_begin[instruction_count+1] = 1; }
	; 

N: 
	  { emit(GOTO, NULL, NULL, NULL, 1); $$=instruction_count; block_begin[instruction_count+1] = 1; }
	;

expr: 
	  LP oper atom atom RP		{ $$ = generateTemp(); addIDtoST($$); emit($2, $3, $4, $$, 1); }
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
	  EQ		{ $$=0; }
	| NOT_EQ	{ $$=1; }
	| LT		{ $$=2; } 
	| GT		{ $$=3; }
	| LE		{ $$=4; }
	| GE		{ $$=5; }
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
	new_node->reg_locs = -1;
	new_node->mem_locs = -1;
	new_node->isInSync = 1;
	new_node->isLive = 1;
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

quadruple* createQuad(int inst_no, int op, char* arg1, char* arg2, char* res) {
    quadruple* new_quad = (quadruple*)malloc(sizeof(quadruple));
    new_quad->inst_no = inst_no;
    new_quad->op = op;
    new_quad->arg1 = (arg1 == NULL) ? NULL : strdup(arg1);
    new_quad->arg2 = (arg2 == NULL) ? NULL : strdup(arg2);
    new_quad->res = (res == NULL) ? NULL : strdup(res);

    return new_quad;
}

void emit(int op, char* arg1, char* arg2, char* res, int flag) {
	if(flag) {
		instruction_count++;
	}
	quadTable* new_quad = (quadTable*)malloc(sizeof(quadTable));
	new_quad->quad = createQuad(instruction_count, op, arg1, arg2, res);

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
	block_begin[target] = 1;
	quadTable* iter = Q_Head;
	/* printf("from: %d, target: %d\n", from, target); */
	while(iter != NULL) {
		char buff[10];
		sprintf(buff, "%d", target);
		if((iter->quad->op == GOTO) && (iter->quad->inst_no == from)) {
			iter->quad->res = strdup(buff);
			return;
		}
		iter = iter->next;
	}	
}
