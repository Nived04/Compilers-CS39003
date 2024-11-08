#include "y.tab.c"
#include "lex.yy.c"

int main() {
    if(yyparse()) {
        printf("Error\n");
    } 
}