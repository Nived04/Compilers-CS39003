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

int getAvailableReg() {
    for(int i=2; i<=11; i++) {
        if(isRegInUse[i] == 0) {
            return i;
        }
    }
    return -1;
}

void printST(SymbolTablePtr st_head) {
    SymbolTablePtr temp = st_head;
    while(temp != NULL) {
        printf("Name: %s, Offset: %d\n", temp->name, temp->offset);
        temp = temp->next;
    }
    return;
}

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

SymbolTablePtr addIDtoST(SymbolTablePtr st_head, char* id) {
    SymbolTablePtr temp = st_head;
    SymbolTablePtr prev = NULL;
    while(temp != NULL) {
        prev = temp;
        temp = temp->next;
    }

    temp = (SymbolTablePtr)malloc(sizeof(SymbolTable));
    temp->name = strdup(id);
    temp->offset = offset;
    temp->next = NULL;

    offset++;

    if(st_head == NULL) {
        return temp;
    }
    else {
        prev->next = temp;
    }
    return st_head;
}

void printFetchesforIDNUM(int fetch_to, int num) {
    printf("\tMEM[%d] = %d;\n", fetch_to, num);
    printf("\tmprn(MEM,%d);\n", fetch_to);
    return;
}

SymbolTablePtr setIDNUM(SymbolTablePtr st_head, char* id, int num) {
    SymbolTablePtr temp = findID(st_head, id);
    if(temp == NULL) {
        temp = addIDtoST(st_head, id);
        printFetchesforIDNUM(offset-1, num);
        return temp;
    }
    else {
        printFetchesforIDNUM(temp->offset, num);
        return st_head;
    }
}

void printFetchesforIDID(int fetched_addr, int fetch_to) {
    printf("\tR[0] = MEM[%d];\n", fetched_addr);
    printf("\tMEM[%d] = R[0];\n", fetch_to);
    printf("\tmprn(MEM,%d);\n", fetch_to);
    return;
}

SymbolTablePtr setIDID(SymbolTablePtr st_head, char* id, char* rid) {
    int roff = getIDOffset(st_head, rid);
    if(roff == -1) {
        printFetchesforIDID(offset, offset-1);
        offset++;
        return st_head;
    }
    SymbolTablePtr temp = findID(st_head, id);
    if(temp == NULL) {
        temp = addIDtoST(st_head, id);
        printFetchesforIDID(roff, offset-1);
        return temp;
    }
    else {
        printFetchesforIDID(roff, temp->offset);
        return st_head;
    }
}

void printFetchesforIDEXPR(int fetch_to, int fetch_from, int expr_type) {
    if(expr_type == 0) {
        printf("\tMEM[%d] = MEM[%d];\n", fetch_to, fetch_from);
    }
    else if(expr_type == 2) {
        printf("\tMEM[%d] = R[%d];\n", fetch_to, fetch_from);
        isRegInUse[fetch_from] = 0;
    }
    printf("\tmprn(MEM,%d);\n", fetch_to);
    return;
}

SymbolTablePtr setIDEXPR(SymbolTablePtr st_head, char* id, valueType* expr) {
    SymbolTablePtr temp = findID(st_head, id);
    if(temp == NULL) {
        temp = addIDtoST(st_head, id);
        printFetchesforIDEXPR(offset-1, expr->value, expr->type);
        return st_head;
    }
    else {
        printFetchesforIDEXPR(temp->offset, expr->value, expr->type);
        return st_head;
    }
}

void selectArg(valueType* arg, char* s, int argnum) {
    switch (arg->type) {
        case 0:
            if(isRegInUse[0] == 0) {
                printf("\tR[0] = MEM[%d];\n", arg->value);
                sprintf(s, "R[0]");
                isRegInUse[0] = 1;
            }
            else {
                printf("\tR[%d] = MEM[%d];\n", argnum - 1, arg->value);
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

valueType* encodeEXPR(int op, valueType* arg1, valueType* arg2) {

    char s1[15], s2[15];

    selectArg(arg1, s1, 1);
    selectArg(arg2, s2, 2);

    int availReg = getAvailableReg();
    valueType* temp = (valueType*)malloc(sizeof(valueType));

    if(availReg == -1) {
        if(op != EXPO) {
            printf("\tR[0] = %s %c %s;\n", s1, (char)op, s2);
            printf("\tMEM[%d] = R[0];\n", offset++);
        }
        else {
            printf("\tR[0] = pwr(%s,%s);\n", s1, s2);
            printf("\tMEM[%d] = R[0];\n", offset++);
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
