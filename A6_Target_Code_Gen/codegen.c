#include "y.tab.c"
#include "lex.yy.c"

void print_QuadTable() {
    quadTable* iter = Q_Head;
    while(iter != NULL) {
        printf("%d: %s %s %s %s\n", iter->quad->inst_no, iter->quad->op, iter->quad->arg1, iter->quad->arg2, iter->quad->res);
        iter = iter->next;
    }
}

int main() {
    memset(block_leaders, 0, 1000*sizeof(int));

    if(yyparse()) {
        printf("Error\n");
    } 

    print_IntCode();
    printf("\n\t%d\t:", instruction_count);
    // print_QuadTable();
}