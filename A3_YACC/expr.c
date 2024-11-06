#include "y.tab.c"
#include "lex.yy.c"

/*

Expression Tree Node Type
0 - operator
1 - number
2 - identifier

*/

// This function adds an identifier or a number to the symbol table
SymbolTablePtr addToSymbolTable(SymbolTablePtr head, char* id, const int num, int flag) {
    SymbolTablePtr newNode = (SymbolTablePtr)malloc(sizeof(SymbolTable));

    newNode->num = num;

    if(id!=NULL)
        newNode->id.text = strdup(id);
    else 
        newNode->id.text = NULL;

    newNode->id.val = -1e9;
    newNode->next = NULL;

    if(head == NULL) {
        head = newNode;
    } 
    else {
        SymbolTablePtr temp = head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode;
    }

    return head;
}

// This function initializes the head of the symbol table
SymbolTablePtr initHead(SymbolTablePtr head) {
	head = (SymbolTablePtr)malloc(sizeof(SymbolTable));
	head->num = -1e9;
	head->id.text = "";
    head->id.val = -1e9;
	return head;
}

// This function is to find a symbol (using a simple linear search) within the symbol table
SymbolTablePtr findSymbol(SymbolTablePtr head, const char* id, const int num, int flag) {
    SymbolTablePtr temp = head;
	if(flag) {
        while(temp!=NULL) {
            if(temp->num == -1000000000) {
                temp=temp->next;
                continue;
            }
            if(temp->num == num) {
                return temp;
            }
            temp = temp->next;
        }       
	}
    else {
        while(temp!=NULL) {
            if(temp->id.text == NULL) {
                temp=temp->next;
                continue;
            }
            if(strcmp(temp->id.text, id) == 0) {
                return temp;
            }
            temp = temp->next;
        }
    }
    return NULL;
}

// This function is to update the value of an identifier in the symbol table, wither wiht the value of a
// number or with the value of another identifier (using flag to differentiate)
SymbolTablePtr updateIdentifierValue(SymbolTablePtr head, char* lid, char* rid, const int val, int flag) {

    SymbolTablePtr LID = findSymbol(head, lid, -1, 0);

    // since this function is called only in the set statement, we need to add the identifier to the symbol table incase it is not present
    if(LID == NULL) {
        head = addToSymbolTable(head, lid, -1e9, 0);
        LID = findSymbol(head, lid, -1, 0);
    }

    if(flag) {
        LID->id.val = val;
    }
    else {
        SymbolTablePtr RID = findSymbol(head, rid, -1, 0);
        if(RID != NULL) {
            LID->id.val = RID->id.val;
        }
        else {
            printf("Error: Identifier %s not declared at line %d\n", rid, yylineno);
            free_allocated_memory();
            exit(1);
        }
    }

    printf("Variable %s is set to %d\n", LID->id.text, LID->id.val);

    return head;
}

// function to initialize the expression tree root
exprNodePtr initExprRoot(exprNodePtr r) {
    r = (exprNodePtr)malloc(sizeof(exprNode));
    r->left = NULL;
    r->right = NULL;
    r->type = -1;
    r->op = -1;
    r->id_num = NULL;
    return r;
}

// function to add an internal node to the expression tree (i.e. a node with an operator)
exprNodePtr addInternalNode(exprNodePtr p, int op) {
    p = (exprNodePtr)malloc(sizeof(exprNode));
    p->left = NULL;
    p->right = NULL;
    p->type = 0;
    p->op = op;
}

// function to add child nodes to an internal node
exprNodePtr addChildNodes(exprNodePtr par, exprNodePtr Lchild, exprNodePtr Rchild) {
    par->left = Lchild;
    par->right = Rchild;
    return par;
}

// function to add a leaf node to the expression tree (i.e. a node with an identifier or a number)
exprNodePtr addLeafNode(SymbolTablePtr entry, int flag) {
    exprNodePtr leaf;
    if(flag) {
        leaf = (exprNodePtr)malloc(sizeof(exprNode));
        leaf->type = 1;
        leaf->id_num = entry;
        leaf->left = NULL;
        leaf->right = NULL;
    }
    else {
        if(entry == NULL) {
            printf("Error: Identifier %s not declared at line %d\n", entry->id.text, yylineno);
            free_allocated_memory();
            exit(1);
        }
        leaf = (exprNodePtr)malloc(sizeof(exprNode));
        leaf->type = 2;
        leaf->id_num = entry;
        leaf->left = NULL;
        leaf->right = NULL;
    }
    return leaf;
}

int binary_exponentiation(int a, int b) {
    int ans = 1;
    while(b) {
        if(b%2==1) {
            ans = (1LL*ans*a);
        }
        a = (1LL*a*a);
        b/=2;
    }
    return ans;
}

// function to evaluate the expression tree
int evaluateExpression(exprNodePtr root) {
    if(root->type == 1) {
        return root->id_num->num;
    }
    else if(root->type == 2) {
        return root->id_num->id.val;
    }
    else {
        int lval = evaluateExpression(root->left);
        int rval = evaluateExpression(root->right);
        switch(root->op) {
            case PLUS: 
                return lval + rval;
            case MINUS: 
                return lval - rval;
            case MULT: 
                return lval * rval;
            case DIV: 
                if(rval == 0) {
                    yyerror("Division by zero");
                    free_allocated_memory();
                    exit(1);
                }
                return lval / rval;
            case MOD: 
                if(rval == 0) {
                    yyerror("Modulo by zero");
                    free_allocated_memory();
                    exit(1);
                }
                return lval % rval;
            case EXPO: 
                if(lval == 0 && rval == 0) {
                    yyerror("0^0 is Ambiguous");
                    free_allocated_memory();
                    exit(1);
                }
                else if(rval < 0) {
                    yyerror("Negative Exponent results in float");
                    free_allocated_memory();
                    exit(1);
                }
                return binary_exponentiation(lval, rval);
        }
    }
}

// function to print a syntax error message and exit
void yyerror(char *s) {
    fprintf(stderr, "Error: %s, at line number %d\n", s, yylineno);
    free_allocated_memory();
    exit(1);
}

// free Symbol Table
void free_SymbolTable(SymbolTablePtr head) {
    SymbolTablePtr temp = head;
    while(temp!=NULL) {
        SymbolTablePtr temp2 = temp;
        temp = temp->next;
        free(temp2);
    }
}

// free Expression Tree
void free_Expression_Tree(exprNodePtr root) {
    if(root == NULL) {
        return;
    }
    free_Expression_Tree(root->left);
    free_Expression_Tree(root->right);
    free(root);
}

// free all allocated memory
void free_allocated_memory() {
    free_SymbolTable(ST_Head);
    free_Expression_Tree(ExprRoot);
}

int main() {
    ST_Head = initHead(ST_Head);
    ExprRoot = initExprRoot(ExprRoot);
    yyparse();
    free_allocated_memory();
    return 0;
}