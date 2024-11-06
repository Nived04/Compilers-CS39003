#include "lex.yy.c"
#include "y.tab.c"
#include <math.h>

// a variadic function that creates an internal node of the parse tree
parseTreeNode* createNode(int type, char* nodeName, int num_children, ...) {
	parseTreeNode* newNode = (parseTreeNode*)malloc(sizeof(parseTreeNode));

    newNode->num_children = num_children;
	newNode->nodeType = type;
    newNode->name = strdup(nodeName);
	newNode->par = NULL;
	newNode->childNode = NULL;

	if(!num_children) return newNode;

	va_list args; // for handling variable number of children
	va_start(args, num_children); // initialize the va_list

	newNode->childNode = (parseTreeNode**)malloc(num_children*sizeof(parseTreeNode*));

	newNode->childNode[0] = va_arg(args, parseTreeNode*);
	newNode->childNode[0]->par = newNode;
	int ct = 1;

    // va_arg updates the va_list to the next argument

	while(ct < num_children){
		newNode->childNode[ct] = va_arg(args,parseTreeNode*);
		newNode->childNode[ct]->par = newNode;
		ct++;
	}
	va_end(args);

	return newNode;
}

// creates a leaf node of the parse tree
parseTreeNode* createLeaf(int type, int val) {
	parseTreeNode* newNode = (parseTreeNode*)malloc(sizeof(parseTreeNode));
	newNode->nodeType = type;
    newNode->name = (char*)malloc(sizeof(char));
    if(type == DIGIT || type == ZERO || type == ONE) {
        newNode->name[0] = val + '0';
        newNode->val = val;
    }
    else {
        newNode->name[0] = (char)val;
    }
	newNode->par = NULL;
	newNode->childNode = NULL;
}

// sets the inherited and synthesized attributes of the parse tree
// has a parameter passon which is relevant for inherited attributes
void setatt(parseTreeNode* node, int lvl, int passon) {
    if(node == NULL)
        return;
    switch (node->nodeType) {
        case X_Node: // has no inherited value to pass so passon is irrelevant and essentially has the same functionality as the S_Node case
        case S_Node:
            for(int i = 0; i < node->num_children; i++) {
                setatt(node->childNode[i], lvl+1, node->childNode[0]->name[0]);
            }
            break;

        case P_Node:
            node->inh = passon;    
            for(int i = 0; i < node->num_children; i++) {
                if(i >= 1) {
                    setatt(node->childNode[i], lvl+1, node->childNode[1]->name[0]);
                }
                else {
                    setatt(node->childNode[i], lvl+1, node->inh);
                }
            }
            break;

        case T_Node:
            node->inh = passon;
            for(int i = 0; i < node->num_children; i++) {
                setatt(node->childNode[i], lvl+1, 0);
            }
            break;

        case N_Node:
            node->val = node->childNode[0]->val;
            if(node->num_children == 2) {
                setatt(node->childNode[1], lvl+1, node->childNode[0]->val);
                node->val = node->childNode[1]->val;
            }
            break;

        case M_Node:
            node->inh = passon;
            if(node->num_children == 2) {
                setatt(node->childNode[1], lvl+1, node->inh*10+node->childNode[0]->val);
                node->val = node->childNode[1]->val;
            }
            else {
                node->val = node->inh * 10 + node->childNode[0]->val;
            }   
            break; 

        default:
            return;
    }
}

// prints the annotated parse tree
void printTree(parseTreeNode* root, int lvl) {
    if(root == NULL) {
        return;
    }
    if(root->nodeType == S_Node) {
        printf("    S []\n");
        for(int i = 0; i < root->num_children; i++) {
            printTree(root->childNode[i], lvl);
        }
    }
    else {
        for(int i = 0; i < lvl+1; i++) {
            printf("    ");
        }
        printf("==> %s ", root->name);
        switch (root->nodeType) {
            case P_Node:
            case T_Node:
                printf("[inh = %c]\n", root->inh);
                break;
            case M_Node:    
                printf("[inh = %d, val = %d]\n", root->inh, root->val);
                break;
            case N_Node:
            case ZERO:
            case ONE:
            case DIGIT:
                printf("[val = %d]\n", root->val);
                break;
            default:
                printf("[]\n");
                break;
        }
        for(int i = 0; i < root->num_children; i++) {
            printTree(root->childNode[i], lvl+1);
        }
    }
}

// evaluates the expression for a given value of x
long evalpoly(parseTreeNode* root, int x) {
    if(root == NULL) {
        return 0;
    }
    long res = 1;
    switch (root->nodeType) {
        case S_Node:
            return evalpoly(root->childNode[root->num_children - 1], x);
            break;
        case P_Node:
            if(root->num_children == 1) {
                return evalpoly(root->childNode[0], x);
            }
            else {
                return evalpoly(root->childNode[0], x) + evalpoly(root->childNode[2], x);
            }
            break;
        case T_Node:
            for(int i = 0; i < root->num_children; i++) {
                res *= evalpoly(root->childNode[i], x);
            }
            return (root->inh == '-') ? -res : res;
        case X_Node:
            return pow(x, (root->num_children == 1) ? 1 : root->childNode[2]->val);
            break;
        case N_Node:
        case DIGIT:
            return root->val;
        case ZERO:
            return 0;
        case ONE:
            return 1;
        default:
            return 0;
    }
}

// prints the derivate of the expression. 
// uses a flag first_flag for not printing the first '+' sign in the derivative of the first variable term.
int printderivative(parseTreeNode* node) {
    if(node == NULL) {
        return 0;
    }
    switch (node->nodeType) {
        case S_Node:
            if(printderivative(node->childNode[node->num_children - 1]) == 2) {
                printf("0");
            }   
            return 0;
        case P_Node:
            if(node->num_children == 1) {
                return printderivative(node->childNode[0]);
            }
            else {
                printderivative(node->childNode[0]);
                printderivative(node->childNode[2]);
            }
            break;
        case T_Node:
            if(node->num_children == 1 && (node->childNode[0]->nodeType == N_Node || node->childNode[0]->nodeType == ONE)) {
                return 2;
            }
            else {
                if(!first_flag) {
                    first_flag = 1;
                    if(node->inh == '-') {
                        printf("- ");
                    }
                }
                else {
                    printf("%c ", node->inh);
                }              
                int temp = 1, exp;
                if(node->childNode[0]->nodeType == N_Node) {
                    temp = node->childNode[0]->val;
                }
                exp = printderivative(node->childNode[node->num_children - 1]);
                if(exp == 1) {
                    printf("%ld ", (1L)*temp);
                }
                else if(exp == 2) {
                    printf("%ldx ", (1L)*temp*2);
                }
                else {
                    printf("%ldx^%d ", (1L)*temp*exp, exp-1);
                }
            }
            break;
        case X_Node:
            return (node->num_children == 1) ? 1 : node->childNode[2]->val;
        default:
            return 0;
    }
    return 0;        
}

int main() {
	if(!yyparse()) {
        setatt(root, 0, 0);
        printf("+++ The annotated parse tree is\n");
        printTree(root, 0);
        printf("\n");
        for(int i=-5; i<=5; i++) {
            printf("+++ f(%2d) = %15ld\n", i, evalpoly(root, i));
        }
        printf("\n");
        printf("+++ f'(x) = ");
        if(x_occurence == 0) {
            printf("0\n");
            return 0;
        }
        printderivative(root);
        printf("\n");
        return 0;
    }
}