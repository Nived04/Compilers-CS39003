#include "y.tab.c"
#include "lex.yy.c"

void print_QuadTable() {
    quadTable* iter = Q_Head;
    int cnt = 1;
    while(iter != NULL) {
        printf("%d: %s %s %s %s\n", cnt, iter->quad->op, iter->quad->arg1, iter->quad->arg2, iter->quad->res);
        iter = iter->next;
        cnt++;
    }
}

int main() {
    if(yyparse()) {
        printf("Error\n");
    } 
    print_IntCode();
    // print_QuadTable();
}