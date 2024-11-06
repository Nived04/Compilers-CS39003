%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex(void);
void yyerror(char *);
extern int yylineno;
extern char* yytext;

// -----*******-----

typedef struct _identifier {
	char* text;
	int val;
}identifier;

typedef identifier* identifierPtr;

typedef struct _SymbolTable {
    int num;
    identifier id;
    struct _SymbolTable* next;
}SymbolTable;

typedef SymbolTable* SymbolTablePtr;

int cnt = 0;


SymbolTablePtr ST_Head = NULL;

SymbolTablePtr initHead(SymbolTablePtr head);
SymbolTablePtr updateIdentifierValue(SymbolTablePtr, char* lid, char* rid, const int val, int flag);
SymbolTablePtr findSymbol(SymbolTablePtr, const char* id, const int num, int flag);
SymbolTablePtr addToSymbolTable(SymbolTablePtr head, char* id, const int num, int flag);

// -----*******-----

typedef struct _exprNode {
    struct _exprNode* left;
    struct _exprNode* right;
    int type;
    int op;
    SymbolTablePtr id_num;
}exprNode;

typedef exprNode* exprNodePtr;

exprNodePtr ExprRoot = NULL;

exprNodePtr initExprRoot(exprNodePtr r);
exprNodePtr addInternalNode(exprNodePtr, int op);
exprNodePtr addChildNodes(exprNodePtr par, exprNodePtr lchild, exprNodePtr rchild);
exprNodePtr addLeafNode(SymbolTablePtr entry, int flag);

int evaluateExpression(exprNodePtr root);

int binary_exponentiation(int a, int b);

void free_SymbolTable(SymbolTablePtr head);
void free_Expression_Tree(exprNodePtr root);
void free_allocated_memory();

%}

%union {int num; char p; char* txt; struct _exprNode* enp;}
%token <num> NUM PLUS MINUS MULT DIV MOD EXPO SET
%token RP LP
%token <txt> ID
%start program
%type <enp> op expr arg exprstmt setstmt stmt

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
         LP SET ID NUM RP   { ST_Head = updateIdentifierValue(ST_Head, $3, NULL, $4, 1); }       
       | LP SET ID ID RP    { ST_Head = updateIdentifierValue(ST_Head, $3, $4, -1, 0); }
       | LP SET ID expr RP  { ST_Head = updateIdentifierValue(ST_Head, $3, NULL, evaluateExpression($4), 1); }
       ;        

exprstmt:
         expr               { ExprRoot = $$; printf("Standalone expression evaluates to %d\n", evaluateExpression(ExprRoot)); }
       ;

expr:
         LP op arg arg RP   { $$ = addChildNodes($2, $3, $4); }
       ;

op:
         PLUS               { $$ = addInternalNode($$, $1); }
       | MINUS              { $$ = addInternalNode($$, $1); }
       | MULT               { $$ = addInternalNode($$, $1); }
       | DIV                { $$ = addInternalNode($$, $1); }
       | MOD                { $$ = addInternalNode($$, $1); }
       | EXPO               { $$ = addInternalNode($$, $1); }
       ;

arg:
         ID                 { SymbolTablePtr temp = findSymbol(ST_Head, $1, -1, 0); 
                              if(temp == NULL) {
                                printf("Error: Identifier %s not declared at line %d\n", $1, yylineno);  
                                exit(1);
                              }                              
                              $$ = addLeafNode(temp, 0); 
                            }
       | NUM                { SymbolTablePtr temp = findSymbol(ST_Head, NULL, $1, 1); 
                              if(temp == NULL) {
                                ST_Head = addToSymbolTable(ST_Head, NULL, $1, 1);
                                temp = findSymbol(ST_Head, NULL, $1, 1);
                              }  
                              $$ = addLeafNode(temp, 1); 
                            }
       | expr               { $$ = $1; }
       ;

%%

