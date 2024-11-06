%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define S_Node 1
#define P_Node 2
#define T_Node 3
#define X_Node 4
#define N_Node 5
#define M_Node 6

extern int yylex();
void yyerror(char *);
extern int yylineno;
extern char* yytext;

typedef struct parseTreeNode {
	int num_children;
	int nodeType;
	int inh, val;
	char* name;
	struct parseTreeNode* par;
	struct parseTreeNode** childNode;
}parseTreeNode;

parseTreeNode* createNode(int, char*, int, ...);
parseTreeNode* createLeaf(int, int);

void setatt(parseTreeNode*, int, int);
void printTree(parseTreeNode*, int);

long evalpoly(parseTreeNode*, int);
int printderivative(parseTreeNode*);

parseTreeNode* root;

int x_occurence = 0, first_flag = 0;

%}

%union {
	int num;
	struct parseTreeNode* node;
}

%token <num> PLUS MINUS EXPO DIGIT ZERO ONE VAR
%start S
%type  <node> S P T X N M

%%

S: 
         P				{ $$ = createNode(S_Node, "S", 1, $1); root = $$; }
       | PLUS P			{ $$ = createNode(S_Node, "S", 2, createLeaf(PLUS, $1), $2); root = $$; }
       | MINUS P		{ $$ = createNode(S_Node, "S", 2, createLeaf(MINUS, $1), $2); root = $$; }
       ;

P:
         T				{ $$ = createNode(P_Node, "P", 1, $1); }
       | T PLUS P		{ $$ = createNode(P_Node, "P", 3, $1, createLeaf(PLUS, $2), $3); }
       | T MINUS P		{ $$ = createNode(P_Node, "P", 3, $1, createLeaf(MINUS, $2), $3); }
       ;

T:
         ONE			{ $$ = createNode(T_Node, "T", 1, createLeaf(ONE, $1)); }
       | N				{ $$ = createNode(T_Node, "T", 1, $1); }
       | X				{ $$ = createNode(T_Node, "T", 1, $1); }
	   | N X			{ $$ = createNode(T_Node, "T", 2, $1, $2); }
       ;

X:
		 VAR			{ $$ = createNode(X_Node, "X", 1, createLeaf(VAR, $1)); x_occurence = 1;  }
	   | VAR EXPO N		{ $$ = createNode(X_Node, "X", 3, createLeaf(VAR, $1), createLeaf(EXPO, $2), $3); x_occurence = 1; }
		
N:

		 DIGIT			{ $$ = createNode(N_Node, "N", 1, createLeaf(DIGIT, $1)); }
	   | ONE M			{ $$ = createNode(N_Node, "N", 2, createLeaf(ONE, $1), $2); }
	   | DIGIT M		{ $$ = createNode(N_Node, "N", 2, createLeaf(DIGIT, $1), $2); }	

M:
		 ZERO			{ $$ = createNode(M_Node, "M", 1, createLeaf(ZERO, $1)); }
	   | ONE			{ $$ = createNode(M_Node, "M", 1, createLeaf(ONE, $1)); }
	   | DIGIT			{ $$ = createNode(M_Node, "M", 1, createLeaf(DIGIT, $1)); }
	   | ZERO M			{ $$ = createNode(M_Node, "M", 2, createLeaf(ZERO, $1), $2); }
	   | ONE M			{ $$ = createNode(M_Node, "M", 2, createLeaf(ONE, $1), $2); }
	   | DIGIT M		{ $$ = createNode(M_Node, "M", 2, createLeaf(DIGIT, $1), $2); }

%%

void yyerror(char * s) {
	printf("*** Syntax Error");
	return;
}