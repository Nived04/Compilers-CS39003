#include <iostream>
#include <stdlib.h>
#include <string>
#include <stack>
#include "lex.yy.c" //from list.l

/*

Integer returns from lex file:

LP 1
RP 2
OP 3
ID 4
INTEGER 5
NL 10
OPS 11

*/

#define EXPR 6
#define ARG 7

using namespace std;

// may be required later
typedef struct production_rules {
    string lhs;
    string rhs;
    production_rules *next;
} production_rules;

// symbol table for identifiers
struct symbolTableNode {
    string name;
    int value;
    symbolTableNode *next;
};

typedef symbolTableNode* symbolTable;

// add to symbol table
symbolTable addToSymbolTable(symbolTable& T, string id) {
    symbolTable p;

    p = T;

    while(p) {
        if(p->name == id) {
            return p;
        }
        p = p->next;
    }

    // cout << "Adding new terminal: " << id << endl;

    p = new symbolTableNode;
    p->name = id;
    p->next = T;
    T = p;

    return p;
}

void printSymbolTable(symbolTable t) {
    if(t == NULL) {
        return;
    }
    printSymbolTable(t->next);
    cout << t->name << " = " << t->value << endl;
    return;
}

// this will be a binary search tree, because it provides efficient operations for constants
struct const_table {
    int value;
    const_table *left;
    const_table* right;
};

typedef const_table* constTable;

constTable val = nullptr;

// add to BST
constTable addToConstTable(constTable C, int value) {
    if (C == NULL) {
        constTable newNode = new const_table;
        newNode->value = value;
        newNode->left = NULL;
        newNode->right = NULL;
        val = newNode;
        return newNode;
    }

    if (value < C->value) {
        C->left = addToConstTable(C->left, value);
    } else if (value > C->value) {
        C->right = addToConstTable(C->right, value);
    } else {
        cout << "Error: Duplicate value in constant table" << endl;
    }

    return C;
}

// parse tree node for the syntax tree
struct parseTreeNode {
    parseTreeNode *parent;
    parseTreeNode *left;
    parseTreeNode *right;
    int type; // could be ID, INTEGER, OP

    // union for the different types of nodes
    union {
        char op;                    // stores the operator at the node
        constTable const_node;      // points to the constant in the constant table
        symbolTable symbol_node;    // points to the identifier in the symbol table
    };

    parseTreeNode() { 
        parent = nullptr; 
        left = nullptr; 
        right = nullptr;
    }
};


typedef parseTreeNode* parseTree;

// a class for the parser
class List_Lang_Parser {
    int next_token;
    int non_terminal_count;
    int terminal_count;
    int** parse_table;
    parseTreeNode* root;
    symbolTable T;
    constTable C;
    
    // creates the parse table, not being currently used, but could be implemented later for parsing 
    void create_parse_table() {
        
        parse_table = (int**)malloc(non_terminal_count * sizeof(int*));
        for(int i = 0; i < non_terminal_count; i++) {
            parse_table[i] = (int*)malloc(terminal_count * sizeof(int));
        }

        // Prod 1: EXPR ->  ( OP ARG ARG )
        // Prod 2: OP   ->  +|-|*|/|%
        // Prod 3: ARG  ->  EXPR|num|id 

        parse_table[0][7] = 1;
        parse_table[1][0] = 2;
        parse_table[1][1] = 2;
        parse_table[1][2] = 2;
        parse_table[1][3] = 2;
        parse_table[1][4] = 2;
        parse_table[2][5] = 3;
        parse_table[2][7] = 3;
    }

    // inserts a node into the parse tree
    parseTree insert(parseTree curr, int type, constTable C, symbolTable T, char txt='-') {
        if(curr==NULL) {
            curr = new parseTreeNode;
            curr->parent = NULL;
            curr->left = NULL;
            curr->right = NULL;
            curr->type = type;
            if(type!=OP) {
                cout << "*** Error: Syntax Error" << endl;
            }
            else{
                curr->op = txt;
            }  
            root = curr;
            return curr;
        }

        parseTree new_node = new parseTreeNode;
        new_node->parent = curr;
        new_node->left = NULL;
        new_node->right = NULL;
        new_node->type = type;

        // for union, only one of its members can be active at a time
        if(type == OP) {
            new_node->op = txt; 
        } 
        else if(type == INTEGER) {
            new_node->const_node = C;
        } 
        else if(type == ID) {
            new_node->symbol_node = T;
        }

        if(curr->left == NULL) {
            curr->left = new_node;
        } 
        else if(curr->right == NULL) {
            curr->right = new_node;
        }
        else {
            cout << "Error: Invalid Parse Tree" << endl;
        }

        if(type == INTEGER || type == ID) {
            return curr;
        }

        curr = new_node;
        return curr;
    }

    void printParseTree(parseTree root, int depth) {
        if(root == NULL) {
            return;
        }

        for(int i = 0; i < depth-1; i++) {
            cout << "      ";
        }
        if(depth!=0)
            cout << "---> ";

        if(root->type == ID) {
            cout << "ID(" << root->symbol_node->name << ")" << endl;
        }
        else if(root->type == INTEGER) {
            cout << "NUM(" << root->const_node->value << ")" << endl;
        }  
        else if(root->type == OP) {
            cout << "OP(" <<  root->op << ")" <<  endl;
        }
        //cout << "Left" << endl; 
        printParseTree(root->left, depth+1);
        //cout << "Right" << endl;
        printParseTree(root->right, depth+1);

        return;
    }

public:
    List_Lang_Parser() {
        non_terminal_count = 3;
        terminal_count = 9;
        root = NULL;
        this->create_parse_table();
        T = NULL;
        C = NULL;
        val = (constTable)malloc(sizeof(constTable));
    }

    int parse(){
        stack<int> parse_stack;


        parseTree curr = root;

        parse_stack.push(EXPR);

        int line_no = 1;

        // main logic for parsing
        while( !parse_stack.empty() ) {
            // scans the next token from the source
            next_token = yylex();
            switch(parse_stack.top()) {
                case EXPR:
                    //cout << "EXPR" << endl;
                    if(next_token == LP) {
                        parse_stack.pop();
                        parse_stack.push(RP);
                        parse_stack.push(ARG);
                        parse_stack.push(ARG);
                        parse_stack.push(OP);
                    } 
                    else if(next_token == NL) {
                        line_no++;
                    } 
                    else {
                        cout << "*** Error: Left parenthesis expected in place of " << yytext << ", at line " << line_no << endl;
                        return 0;
                    }
                    break;
                case OP:
                    //cout << "OP" << endl;
                    if(next_token == OP) {
                        parse_stack.pop();
                        char temp = yytext[0];
                        curr = insert(curr, OP, NULL, NULL, temp);
                    } 
                    else if(next_token == OPS) {
                        cout << "*** Error: Invalid operator " << yytext << " found at line " << line_no << endl;
                        return 0;
                    }
                    else if(next_token == NL) {
                        line_no++;
                    } 
                    else {
                        cout << "*** Error: Operator expected in place of " << yytext << ", at line " << line_no << endl;
                        return 0;
                    }
                    break;
                case ARG:
                    //cout << "ARG" << endl;
                    if(next_token == INTEGER) {
                        C = addToConstTable(C, atoi(yytext));
                        parse_stack.pop();
                        curr = insert(curr, INTEGER, val, NULL);
                        //cout << val->value << endl;
                    } 
                    else if(next_token == ID) {
                        symbolTable id_ref = addToSymbolTable(T, yytext);
                        parse_stack.pop();
                        curr = insert(curr, ID, NULL, id_ref);
                        //cout << id_ref->name << endl;
                    } 
                    else if(next_token == LP) {
                        parse_stack.pop();
                        parse_stack.push(RP);
                        parse_stack.push(ARG);
                        parse_stack.push(ARG);
                        parse_stack.push(OP);
                    }  
                    else if(next_token == NL) {
                        line_no++;
                    }
                    else {
                        cout << "*** Error: ID/NUM/LP expected in place of " << yytext << ", at line " << line_no << endl;
                        return 0;
                    }
                    break;
                case RP:
                    // cout << "RP" << endl;
                    if(next_token == RP) {
                        parse_stack.pop();
                        curr = curr->parent;
                    } 
                    else if(next_token == NL) {
                        line_no++;
                    }
                    else {
                        cout << "*** Error: Right parenthesis expected in place of " << yytext << ", at line " << line_no << endl;
                        return 0;
                    }
                    break;
                default:
                    break;
            }
        }
        cout << "Parsing is successful" << endl;
        printParseTree(root, 0);
        return 1;
    }

    // fills the values of the identifiers
    void fill_values(symbolTable p) {
        if(p == NULL) {
            return;
        }

        fill_values(p->next);
        next_token = yylex();


        while(next_token == NL) {
            next_token = yylex();
        }

        if(next_token == 0) {
            cout << "*** Error, less values given than expected for the identifiers" << endl;
            exit(1);
        }
        // cout << next_token << yytext <<  endl;

        if(next_token == INTEGER) {
            p->value = stoi(yytext);
        } 
        else {
            cout << "*** Error, Expected integer value for " << p->name << endl;
            exit(1);
        }

        return;
    }

    void read_variable_inputs() {
        symbolTable p = T;
        if(p!=NULL) {
            cout << "Reading variable values from the input" << endl;
        }
        fill_values(p);
        printSymbolTable(T);
    }

    // evaluates the parse tree
    int evaluate(parseTree root) {
        if(root == NULL) {
            return 0;
        }

        if(root->type == INTEGER) {
            return root->const_node->value;
        } 
        else if(root->type == ID) {
            return root->symbol_node->value;
        } 
        else if(root->type == OP) {
            int left = evaluate(root->left);
            int right = evaluate(root->right);

            if(left == INT32_MIN || right == INT32_MIN) {
                return INT32_MIN;
            }

            if(root->op == '/' && right == 0) {
                cout << "Error: Division by zero not allowed" << endl;
                return INT32_MIN;
            }
            if(root->op == '%' && right == 0) {
                cout << "Error: Modulo by zero invalid" << endl;
                return INT32_MIN;
            }

            switch(root->op) {
                case '+':
                    return left + right;
                case '-':
                    return left - right;
                case '*':
                    return left * right;
                case '/':
                    return left / right;
                case '%':
                    return left % right;
                default:
                    cout << "Error: Invalid operator" << endl;
                    return INT32_MIN;
            }
        }
        return 0;
    }

    void evaluate_tree() {
        int result = evaluate(root);
        if(result == INT32_MIN) {
            return;
        }
        cout << "The expression evaluates to " << result << endl;
    }

};

int main() {
    List_Lang_Parser parser;

    if(parser.parse()) {
        parser.read_variable_inputs();
        parser.evaluate_tree();
    }

    free(val);
    return 0;
}