#include "y.tab.c"
#include "lex.yy.c"

void print_QuadTable(quadTable** head) {
    quadTable* iter = *head;
    while(iter != NULL) {
        printf("%d: %d %s %s %s\n", iter->quad->inst_no, iter->quad->op, iter->quad->arg1, iter->quad->arg2, iter->quad->res);
        iter = iter->next;
    }
}

void emitTC(int op, char* arg1, char* arg2, char* res) {
    target_inst_count++;

    quadTable* new_quad = (quadTable*)malloc(sizeof(quadTable));
    new_quad->quad = (quadruple*)malloc(sizeof(quadruple));

    new_quad->quad->inst_no = target_inst_count;
    new_quad->quad->op = op;
    new_quad->quad->arg1 = (arg1 == NULL) ? NULL : strdup(arg1);
    new_quad->quad->arg2 = (arg2 == NULL) ? NULL : strdup(arg2);
    new_quad->quad->res = (res == NULL) ? NULL : strdup(res);

    new_quad->next = NULL;

    if(T_Head == NULL) {
        T_Head = new_quad;
    }
    else {
        quadTable* iter = T_Head;
        while(iter->next != NULL) {
            iter = iter->next;
        }
        iter->next = new_quad;
    }
}

void print_IntCode() {
	quadTable* iter = Q_Head;
	int inst = 0, block_count = 1;

	block_leaders[block_count] = 1;

    printf("Block %d\n", block_count++);

	while(iter != NULL) {
		inst++;

		if(block_begin[inst] == 1) {
			block_leaders[block_count] = inst;
            printf("\nBlock %d\n", block_count++);
		}

		printf("\t%d\t: ", inst);

		if( strcmp(iter->quad->res, "iffalse") == 0) {
			printf("%s (%s %s %s) ", iter->quad->res, iter->quad->arg1, relation_op[iter->quad->op], iter->quad->arg2);
			iter = iter->next;
			printf("goto %s\n", iter->quad->res);
		}
		else if(iter->quad->op == GOTO) {
			printf("%s %s\n", "goto", iter->quad->res);
		}
		else {
			if(iter->quad->arg2 == NULL) {
				printf("%s = %s\n", iter->quad->res, iter->quad->arg1);
			}
			else {
				printf("%s = %s %c %s\n", iter->quad->res, iter->quad->arg1, (char)iter->quad->op, iter->quad->arg2);
			}
		}

		iter = iter->next;
	}
}

void freeRegDesc(int regno) {
    name_list* n = RegBank[regno].reg_descriptor;
    name_list* temp = n;

    while(n != NULL) {
        SymbolTable* sym = findID(n->var_name); 
        sym->reg_locs = -1;
        if(sym->isInSync == 0) {
            if(sym->name[0] != '$') {
                char temp[20];
                sprintf(temp, "R%d", regno+1);
                emitTC(ST, n->var_name, NULL, temp);
            }
            sym->isInSync = 1;
        }
        temp = n;
        n = n->next;
        free(temp);
    }
}

void freeRegisters() {
    for(int i=0; i<MAX_REG; i++) {
        RegBank[i].score = 0;
        freeRegDesc(i);
        RegBank[i].reg_descriptor = NULL;
    }
}

void addDescriptor(int regno, char* arg) {
    name_list* new_name = (name_list*)malloc(sizeof(name_list));
    new_name->var_name = strdup(arg);

    name_list* temp = RegBank[regno].reg_descriptor;

    if(temp == NULL) {
        RegBank[regno].reg_descriptor = new_name;
        return;
    }
    while(temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = new_name;
}

void removeDescriptor(int regno, char* arg) {
    name_list* temp = RegBank[regno].reg_descriptor;
    name_list* prev = NULL;

    while(temp != NULL) {
        if(strcmp(temp->var_name, arg) == 0) {
            if(prev == NULL) {
                RegBank[regno].reg_descriptor = temp->next;
            }
            else {
                prev->next = temp->next;
            }
            free(temp);
            return;
        }
        prev = temp;
        temp = temp->next;
    }
}

void issueLoad(SymbolTable* s, int reg, int isLHS) {
    // freeing the register before loading
    RegBank[reg].score = 0;
    freeRegDesc(reg);
    RegBank[reg].reg_descriptor = NULL;

    addDescriptor(reg, s->name);
    RegBank[reg].score = 1;
    s->reg_locs = reg;
    if((isLHS == 0) && (s->name[0] != '$')) {
        char temp[20];
        sprintf(temp, "R%d", reg+1);  
        emitTC(LD, s->name, NULL, temp);
    }
}

int isDigit(char c) {
    return (c >= '0' && c <= '9') ? 1 : 0;
}

int getReg(char* arg, int isLHS) {
    if(arg[0] == '-' || arg[0] == '+' || isDigit(arg[0])) {
        return -1;
    }

    SymbolTable* sym_arg = findID(arg);

    // printf("%s - %p - %d\n", sym_arg->name, sym_arg, sym_arg->reg_locs);

    int reg = (sym_arg->reg_locs != -1) ? sym_arg->reg_locs : -1;
    if(reg != -1) {
        return reg;
    }

    // check for an empty register
    for(int i=0; i<MAX_REG; i++) {
        if(RegBank[i].reg_descriptor == NULL) {
            issueLoad(sym_arg, i, isLHS);
            return i;
        }
    }

    // check for a register having variable and temporaries with latest value in memory
    for(int i=0; i<MAX_REG; i++) {
        int flag = 1;

        name_list* iter = RegBank[i].reg_descriptor;

        while(iter != NULL) {
            SymbolTable* sym = findID(iter->var_name);

            if((sym->name[0]=='$') && sym->isLive==1) {
                flag = 0;
                break;
            }
            else if(sym->isInSync == 0) {
                flag = 0;
                break;
            }
            iter = iter->next;
        }

        if(flag) {
            issueLoad(sym_arg, i, isLHS);
            return i;
        }
    }

    // check for a register with lowest score and non-live temporaries
    int min_score = 10000;
    int min_score_reg = -1;

    for(int i=0; i<MAX_REG; i++) {
        if(RegBank[i].score < min_score) {
            int flag = 1;
            name_list* iter = RegBank[i].reg_descriptor;

            while(iter != NULL) {
                SymbolTable* sym = findID(iter->var_name);
                if((sym->name[0]=='$') && sym->isLive==1) {
                    flag = 0;
                    break;
                }
                iter = iter->next;
            }
            if(flag) {
                min_score = RegBank[i].score;
                min_score_reg = i;
            }
        }
    }
    if(min_score_reg != -1) {
        issueLoad(sym_arg, min_score_reg, isLHS);
        return min_score_reg;
    }

    return 0;
}

void convertToTargetCode() {
    quadTable* iter = Q_Head;
    int inst  = 1;

    while(iter != NULL) {
        if(block_begin[inst] == 1) {
            freeRegisters();
            inst_to_target[inst] = target_inst_count + 1;
        }

        if(iter->quad->op == GOTO) {
            freeRegisters();
            emitTC(JUMP, NULL, NULL, iter->quad->res);
        }
        else if(strcmp(iter->quad->res, "iffalse") == 0) {
            char tc_arg1[20], tc_arg2[20], tc_op[20];

            int arg1_reg = getReg(iter->quad->arg1, 0);
            int arg2_reg = getReg(iter->quad->arg2, 0);

            if(iter->quad->arg1[0] == '$') {
                SymbolTable* sym = findID(iter->quad->arg1);
                sym->isLive = 0;
            }   
            if(iter->quad->arg2[0] == '$') {
                SymbolTable* sym = findID(iter->quad->arg2);
                sym->isLive = 0;
            }

            if(arg1_reg == -1) {
                strcpy(tc_arg1, iter->quad->arg1);
            }
            else {
                sprintf(tc_arg1, "R%d", arg1_reg+1);
            }

            if(arg2_reg == -1) {
                strcpy(tc_arg2, iter->quad->arg2);
            }
            else {
                sprintf(tc_arg2, "R%d", arg2_reg+1);
            }

            freeRegisters();

            emitTC(iter->quad->op, tc_arg1, tc_arg2, iter->next->quad->res);

            iter = iter->next;
        }
        else if(iter->quad->op == EQUATE){
            SymbolTable* sym = findID(iter->quad->res);

            if(iter->quad->arg1[0] == '-' || iter->quad->arg1[0] == '+' || isDigit(iter->quad->arg1[0])) {
                char temp[20];
                int res_reg = getReg(iter->quad->res, 1);

                sprintf(temp, "R%d", res_reg+1);
                
                RegBank[res_reg].score = 0;
                freeRegDesc(res_reg);
                RegBank[res_reg].reg_descriptor = NULL;
                
                addDescriptor(res_reg, sym->name);

                RegBank[res_reg].score = 1;
                sym->reg_locs = res_reg ;

                // printf("%p, %d\n", sym, sym->reg_locs);

                emitTC(LDI, iter->quad->arg1, NULL, temp);
            }
            else {
                int arg1_reg = getReg(iter->quad->arg1, 0);
                
                if(sym->reg_locs != -1) {
                    removeDescriptor(sym->reg_locs, sym->name);
                }
                sym->reg_locs = arg1_reg;

                addDescriptor(arg1_reg, sym->name);

                if(iter->quad->arg1[0] == '$')
                    findID(iter->quad->arg1)->isLive = 0;
            }
            sym->isInSync = 0;
        }
        // OP T A B
        else {
            char tc_arg1[20], tc_arg2[20], tc_res[20];

            int arg1_reg = getReg(iter->quad->arg1, 0);
            int arg2_reg = getReg(iter->quad->arg2, 0);

            if(iter->quad->arg1[0] == '$') {
                SymbolTable* sym = findID(iter->quad->arg1);
                sym->isLive = 0;
            }   
            if(iter->quad->arg2[0] == '$') {
                SymbolTable* sym = findID(iter->quad->arg2);
                sym->isLive = 0;
            }

            int res_reg = getReg(iter->quad->res, 1);
            SymbolTable* sym = findID(iter->quad->res);
            sym->isInSync = 0;

            if(arg1_reg == -1) {
                strcpy(tc_arg1, iter->quad->arg1);
            }
            else {
                sprintf(tc_arg1, "R%d", arg1_reg+1);
            }

            if(arg2_reg == -1) {
                strcpy(tc_arg2, iter->quad->arg2);
            }
            else {
                sprintf(tc_arg2, "R%d", arg2_reg+1);
            }

            sprintf(tc_res, "R%d", res_reg+1);

            emitTC(iter->quad->op, tc_arg1, tc_arg2, tc_res);
        }
        iter = iter->next;
        inst++;
    }
    freeRegisters();
}

void print_TargetCode() {
    quadTable* iter = T_Head;
    int target_inst = 1, block_count = 1;

    FILE *f = fopen("target_code.txt", "w");

    fprintf(f, "Block %d\n", block_count++);

    while(iter) {
        if(inst_to_target[block_leaders[block_count]] == target_inst) {
            fprintf(f, "\nBlock %d\n", block_count++);
        }

        char* tc_op;
        if(iter->quad->op < 10) {
            tc_op = strdup(relation_target[iter->quad->op]);
        }
        else {
            if(iter->quad->op == '+') 
                tc_op = strdup("ADD");
            else if(iter->quad->op == '-')
                tc_op = strdup("SUB");
            else if(iter->quad->op == '*')
                tc_op = strdup("MUL");
            else if(iter->quad->op == '/')
                tc_op = strdup("DIV");
            else if(iter->quad->op == '%')
                tc_op = strdup("REM");
        }

        fprintf(f, "\t%d\t: %s ", target_inst, tc_op);

        if(iter->quad->op == JUMP) { // jump type code
            fprintf(f, "%d\n", inst_to_target[atoi(iter->quad->res)]);
        }
        else if(iter->quad->op < 6) {
            fprintf(f, "%s %s %d\n", iter->quad->arg1, iter->quad->arg2, inst_to_target[atoi(iter->quad->res)]);
        }
        else if(iter->quad->op == LDI || iter->quad->op == LD) { // load type code
            fprintf(f, "%s %s\n", iter->quad->res, iter->quad->arg1);
        }
        else if(iter->quad->op == ST) { // store type code
            fprintf(f, "%s %s\n", iter->quad->arg1, iter->quad->res);
        }
        else { // arithmetic type code
            fprintf(f, "%s %s %s\n", iter->quad->res, iter->quad->arg1, iter->quad->arg2);
        } 
        iter = iter->next;
        target_inst++;
    }
    fprintf(f, "\t%d\t: ", target_inst);
    fclose(f);  
}

int main() {
    memset(block_begin, 0, 1000*sizeof(int));
    memset(block_leaders, 0, 1000*sizeof(int));

    if(yyparse()) {
        printf("Error\n");
    } 

    print_IntCode();
    printf("\n\t%d\t:", instruction_count);

    convertToTargetCode();

    print_TargetCode();
}