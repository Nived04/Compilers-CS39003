#include "y.tab.c"
#include "lex.yy.c"

int getIDOffset(SymbolTablePtr st_head, char* id) {
    // printST(st_head);
    SymbolTablePtr temp = st_head;
    while(temp!=NULL) {
        if(strcmp(temp->name, id) == 0) {
            return temp->offset;
        }
        temp = temp->next;
    }
    return -1;
}

// void printST(SymbolTablePtr st_head) {
//     SymbolTablePtr temp = st_head;
//     while(temp!=NULL) {
//         printf("\tName: %s, Offset: %d;\n", temp->name, temp->offset);
//         temp = temp->next;
//     }
//     return;
// }

// finds the first available register
int getAvailableReg() {
    for(int i=2; i<=11; i++) {
        if(isRegInUse[i] == 0) {
            return i;
        }
    }
    return -1;
}

// searches for a particular identifier in the symbol table
SymbolTablePtr findID(SymbolTablePtr st_head, char* id) {
    SymbolTablePtr temp = st_head;
    while(temp != NULL) {
        if( strcmp(temp->name, id) == 0) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

// adds an identifier to the symbol table
SymbolTablePtr addIDtoST(SymbolTablePtr* st_head, char* id) {
    SymbolTablePtr new_node = (SymbolTablePtr)malloc(sizeof(SymbolTable));
    new_node->name = strdup(id); 
    new_node->offset = offset++;
    new_node->next = NULL;

    // If the head is NULL, this is the first node
    if (*st_head == NULL) {
        *st_head = new_node;
    } 
    else {
        SymbolTablePtr temp = *st_head;
        while(temp->next != NULL) {
            temp = temp->next;
        }
        // Add new node to the end
        temp->next = new_node;
    }
    return new_node;
}

// Function to find an identifier in the symbol table, if not found, add it
SymbolTablePtr findOrAddID(SymbolTablePtr* st_head, char* id) {
    SymbolTablePtr temp = findID(*st_head, id);
    if (!temp) 
        temp = addIDtoST(st_head, id);
    return temp;
}

// Function to fetch a value from a register
void fetchFromRegister(int fetch_to, int reg) {
    printf("\tMEM[%d] = R[%d];\n", fetch_to, reg);
    printf("\tmprn(MEM,%d);\n", fetch_to);
    isRegInUse[reg] = 0;  // Mark register as free after use
}

// Function to fetch a value from a memory location
void fetchToRegister(int fetch_from_addr, int fetch_to_mem, int fetch_to_reg, int isStore) {
    printf("\tR[%d] = MEM[%d];\n", fetch_to_reg, fetch_from_addr);
    if(isStore) {
        fetchFromRegister(fetch_to_mem, 0);
    }
}

// Function to set an identifier to a number
SymbolTablePtr setIDNUM(SymbolTablePtr st_head, char* id, int num) {
    SymbolTablePtr temp = findOrAddID(&st_head, id);
    printf("\tMEM[%d] = %d;\n", temp->offset, num);
    printf("\tmprn(MEM,%d);\n", temp->offset);
    return st_head;
}

// Function to set an identifier to another identifier
SymbolTablePtr setIDID(SymbolTablePtr st_head, char* id, char* rid) {
    SymbolTablePtr right_id = findOrAddID(&st_head, rid);
    SymbolTablePtr temp = findOrAddID(&st_head, id);
    fetchToRegister(right_id->offset, temp->offset, 0, 1);
    return st_head;
}

// Function to set an identifier to an expression
SymbolTablePtr setIDEXPR(SymbolTablePtr st_head, char* id, valueType* expr) {
    SymbolTablePtr temp = findOrAddID(&st_head, id);
    fetchFromRegister(temp->offset, expr->value);
    return st_head;
}

// sets the output string value depending on argument type
void selectArg(valueType* arg, char* s, int argnum) {
    switch (arg->type) {
        case 0:
            if(isRegInUse[0] == 0) {
                fetchToRegister(arg->value, -1, 0, 0);
                sprintf(s, "R[0]");
                isRegInUse[0] = 1;
            }
            else {
                fetchToRegister(arg->value, -1, argnum-1, 0);
                sprintf(s, "R[%d]", argnum - 1);
                isRegInUse[argnum - 1] = 1;
            }
            break;
        case 1:
            sprintf(s, "%d", arg->value);
            break;
        case 2:
            sprintf(s, "R[%d]", arg->value);
            isRegInUse[arg->value] = 0;
            break;
        default:
            break;
    }
}

// Function that does the required intermediate operations for the given expression
valueType* encodeEXPR(int op, valueType* arg1, valueType* arg2) {
    char s1[15], s2[15];

    selectArg(arg1, s1, 1);
    selectArg(arg2, s2, 2);

    int availReg = getAvailableReg();
    valueType* temp = (valueType*)malloc(sizeof(valueType));

    if(availReg == -1) {
        char s[15];
        sprintf(s, "$%d", ++temp_offset);
        SymbolTablePtr new_node = addIDtoST(&ST_Head, s);

        if(op != EXPO) {
            printf("\tR[0] = %s %c %s;\n", s1, (char)op, s2);
            printf("\tMEM[%d] = R[0];\n", new_node->offset);
        }
        else {
            printf("\tR[0] = pwr(%s,%s);\n", s1, s2);
            printf("\tMEM[%d] = R[0];\n", new_node->offset);
        }
        temp->type = 0;
        temp->value = offset-1;
        isRegInUse[0] = 0;
        return temp;    
    }
    else {
        if(op != EXPO) {
            printf("\tR[%d] = %s %c %s;\n", availReg, s1, (char)op, s2);
        }
        else {
            printf("\tR[%d] = pwr(%s,%s);\n", availReg, s1, s2);
        }
        isRegInUse[availReg] = 1;
        temp->type = 2;
        temp->value = availReg;
        isRegInUse[0] = 0;
        return temp;
    }
}

// Function to handle standalone expressions
void standalone(valueType* expr) {
    if(expr->type == 0) {
        printf("\teprn(MEM,%d);\n", expr->value);
    }
    else if(expr->type == 2) {
        printf("\teprn(R,%d);\n", expr->value);
        isRegInUse[expr->value] = 0;
    }
    return;
}

int main() {
    printf("#include <stdio.h>\n#include <stdlib.h>\n#include \"aux.c\"\n\nint main ( )\n{\n" );
    printf("\tint R[12];\n\tint MEM[65536];\n\n");
    yyparse();
    printf("\n\texit(0);\n}\n");
    return 0;
}
