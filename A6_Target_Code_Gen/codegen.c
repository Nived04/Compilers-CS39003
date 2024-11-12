#include "y.tab.c"
#include "lex.yy.c"

// prints the intermediate code to a file
void print_IntCode() {
	quadTable* iter = Q_Head;
	int inst = 0, block_count = 1;

    // block_leaders keeps track of which instruction number is the leader of a block
    // so when we print the target code, we can print the correct block number

	block_leaders[block_count] = 1; // the first instruction is always a block leader

    FILE *f = fopen("intermediate_code.txt", "w");
    fprintf(f, "Block %d\n", block_count++);

	while(iter != NULL) {
		inst++;

		if(block_begin[inst] == 1) {
			block_leaders[block_count] = inst;
            fprintf(f, "\nBlock %d\n", block_count++);
		}

		fprintf(f, "\t%d\t: ", inst);

		if( strcmp(iter->quad->res, "iffalse") == 0) {
			fprintf(f, "%s (%s %s %s) ", iter->quad->res, iter->quad->arg1, relation_op[iter->quad->op], iter->quad->arg2);
			iter = iter->next;
            // since the next quadruple is the goto statement, we print it here along wiht iffalse
            // and during parsing, the instruction number was taken care of so that the goto statement
            // after an iffalse does not increment the instruction number
			fprintf(f, "goto %s\n", iter->quad->res);
		}
		else if(iter->quad->op == GOTO) {   // standalone goto statement
			fprintf(f, "%s %s\n", "goto", iter->quad->res);
		}
		else { 
            // set statement
			if(iter->quad->arg2 == NULL) {
				fprintf(f, "%s = %s\n", iter->quad->res, iter->quad->arg1);
			}
            // arithmetic statement
			else {
				fprintf(f, "%s = %s %c %s\n", iter->quad->res, iter->quad->arg1, (char)iter->quad->op, iter->quad->arg2);
			}
		}
		iter = iter->next;
	}
    fprintf(f, "\n\t%d\t:", instruction_count);
    fclose(f);
}

// emits a quadruple for target code
void emitTC(int op, char* arg1, char* arg2, char* res) {
    target_inst_count++; 

    quadTable* new_quad_node = (quadTable*)malloc(sizeof(quadTable));
    new_quad_node->quad = createQuad(target_inst_count, op, arg1, arg2, res);
    new_quad_node->next = NULL;

    if(T_Head == NULL) {
        T_Head = new_quad_node;
    }
    else {
        quadTable* iter = T_Head;
        while(iter->next != NULL) {
            iter = iter->next;
        }
        iter->next = new_quad_node;
    }
}

// frees the register descriptor of a register
void freeRegDesc(int regno) {
    name_list* n = RegBank[regno].reg_descriptor;
    name_list* temp = n;

    while(n != NULL) {
        SymbolTable* sym = findID(n->var_name); 
        sym->reg_locs = -1; // remove this register from the variable's register location
        if(sym->isInSync == 0) {
            // if the variable is not in sync with the memory and it is also not a temporary,
            // then we first need to store it before freeing the register
            if(sym->name[0] != '$' || sym->isLive == 1) {
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

// frees all the registers
void freeRegisters() {
    for(int i=0; i<MAX_REG; i++) {
        RegBank[i].score = 0;
        freeRegDesc(i);
        RegBank[i].reg_descriptor = NULL;
    }
}

// adds a variable to the register descriptor of a register
void addDescriptor(int regno, char* arg) {
    name_list* new_name = (name_list*)malloc(sizeof(name_list));
    new_name->var_name = strdup(arg);
    new_name->next = NULL;

    name_list* temp = RegBank[regno].reg_descriptor;

    if(temp == NULL) {
        RegBank[regno].reg_descriptor = new_name;
        return;
    }
    while(temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = new_name;

    RegBank[regno].score++;
}

// removes a variable from the register descriptor of a register
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

// allocates a register to a variable
void allocateReg(int regno, char* arg) {
    RegBank[regno].score = 0;
    freeRegDesc(regno);
    RegBank[regno].reg_descriptor = NULL;
    addDescriptor(regno, arg);
    RegBank[regno].score = 1;
}

// issues a load instruction for a variable
void issueLoad(SymbolTable* s, int reg, int isLHS) {
    // freeing the register before loading
    allocateReg(reg, s->name);
    s->reg_locs = reg;
    // the variable must not be LHS of an assignment, 
    if((isLHS == 0)) {
        char temp[20];
        sprintf(temp, "R%d", reg+1);  
        emitTC(LD, s->name, NULL, temp);
    }
}

// returns the available register for a variable or finds a register with least score (number of variables not in sync
// with memory) and non-live temporaries, and returns that register after saving the unsynced variables to memory
int getReg(char* arg, int isLHS, int used_reg, int next_leader) {
    // no need of a register for constants
    if(arg[0] == '-' || arg[0] == '+' || isDigit(arg[0])) 
        return -1;

    SymbolTable* sym_arg = findID(arg);

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
        if(used_reg == i) {
            continue;
        }

        int hasLatestValue = 1;

        name_list* iter = RegBank[i].reg_descriptor;

        while(iter != NULL) {
            SymbolTable* sym = findID(iter->var_name);

            if((sym->name[0]=='$') && (sym->isLive==1)) {
                hasLatestValue = 0;
                break;
            }
            else if(sym->isInSync == 0) {
                hasLatestValue = 0;
                break;
            }
            iter = iter->next;
        }

        if(hasLatestValue) {
            issueLoad(sym_arg, i, isLHS);
            return i;
        }
    }

    // check for a register containing only temporaries, neither of which will be used in the rest of the block
    for(int i=0; i<MAX_REG; i++) {
        if(used_reg == i) {
            continue;
        }
        
        int all_temp = 1;
        name_list* iter = RegBank[i].reg_descriptor;
        while(iter != NULL) {
            SymbolTable* sym = findID(iter->var_name);
            if(iter->var_name[0] != '$' || sym->isLive == 1 ) {
                all_temp = 0;
                break;
            }
            iter = iter->next;
        }
        if(all_temp) {
            issueLoad(sym_arg, i, isLHS);
            return i;
        }
    }

    // check for a register with lowest score
    int min_score = 10000;
    int min_score_reg = -1;
    for(int i=0; i<MAX_REG; i++) {
        if(used_reg == i) {
            continue;
        }
        if(RegBank[i].score < min_score) {
            min_score = RegBank[i].score;
            min_score_reg = i;
        }
    }
    if(min_score_reg != -1) {
        issueLoad(sym_arg, min_score_reg, isLHS);
        return min_score_reg;
    }

    // never happens
    return 0;
}

// kills a temporary variable
void kill_temp(char* name, int regno) {
    if(name[0] == '$') {
        SymbolTable* sym = findID(name);
        sym->isLive = 0; sym->reg_locs = -1;
        RegBank[regno].score--;
    }
}

// returns the target code value of an argument. 
char* tc_arg_value(char* store_in, char* arg, int isLHS, int reg) {
    store_in = (char*)malloc(20*sizeof(char));

    // if the arguments are temporaries, turn off their live status since this is a conditional statement
    // hence they are not live after this statement
    kill_temp(arg, reg);

    if(reg == -1) {
        strcpy(store_in, arg);
    }
    else {
        sprintf(store_in, "R%d", reg+1);
    }
    return store_in;
}

// converts the intermediate code to target code by sequentially going through the quadruples of
// the intermediate code and emitting the corresponding target code to the quadTable of target code
void convertToTargetCode() {
    quadTable* iter = Q_Head;
    int inst  = 1;
    int block_count = 1;
    int next_leader;
    while(iter != NULL) {
        if(block_begin[inst] == 1) {
            // free all the registers before starting a new block
            freeRegisters();
            next_leader = block_leaders[block_count+1];
            block_count++;

            // a map to keep track of which target code instruction corresponds to the leader instruction of a block
            inst_to_target[inst] = target_inst_count + 1;
        }

        // unconditional jump: goto L
        if(iter->quad->op == GOTO) {
            freeRegisters();
            emitTC(JUMP, NULL, NULL, iter->quad->res);
        }
        // conditional jump: iffalse A relop B goto L
        else if(strcmp(iter->quad->res, "iffalse") == 0) {
            char *tc_arg1, *tc_arg2, tc_op[20];

            int arg1_reg = getReg(iter->quad->arg1, 0, -1, next_leader);
            int arg2_reg = getReg(iter->quad->arg2, 0, arg1_reg, next_leader);

            // get the target code values of the arguments
            tc_arg1 = tc_arg_value(tc_arg1, iter->quad->arg1, 0, arg1_reg);
            tc_arg2 = tc_arg_value(tc_arg2, iter->quad->arg2, 0, arg2_reg);

            freeRegisters();
            emitTC(iter->quad->op, tc_arg1, tc_arg2, iter->next->quad->res);

            iter = iter->next;
        }
        // set statement: T = A
        else if(iter->quad->op == EQUATE){
            SymbolTable* sym = findID(iter->quad->res);

            if(iter->quad->arg1[0] == '-' || iter->quad->arg1[0] == '+' || isDigit(iter->quad->arg1[0])) {
                char temp[20];
                int res_reg = getReg(iter->quad->res, 1, -1, next_leader);

                sprintf(temp, "R%d", res_reg+1);

                allocateReg(res_reg, sym->name);
                sym->reg_locs = res_reg ;

                // printf("%p, %d\n", sym, sym->reg_locs);

                emitTC(LDI, iter->quad->arg1, NULL, temp);
            }
            else {
                int arg1_reg = getReg(iter->quad->arg1, 0, -1, next_leader);
                
                // update the register location of the result to that of argument and free it from its previous location
                if(sym->reg_locs != -1) {
                    removeDescriptor(sym->reg_locs, sym->name);
                }
                sym->reg_locs = arg1_reg;

                addDescriptor(arg1_reg, sym->name);

                kill_temp(iter->quad->arg1, arg1_reg);
            }
            // since the value is recently updated in the set statment, it is out of sync with the memory
            sym->isInSync = 0;
        }
        // arithmetic statement: OP T A B
        else {
            char *tc_arg1, *tc_arg2, tc_res[20];

            int arg1_reg = getReg(iter->quad->arg1, 0, -1, next_leader);
            int arg2_reg = getReg(iter->quad->arg2, 0, arg1_reg, next_leader);

            tc_arg1 = tc_arg_value(tc_arg1, iter->quad->arg1, 0, arg1_reg);
            tc_arg2 = tc_arg_value(tc_arg2, iter->quad->arg2, 0, arg2_reg);

            int res_reg = getReg(iter->quad->res, 1, -1, next_leader);
            SymbolTable* sym = findID(iter->quad->res);
            sym->isInSync = 0;

            sprintf(tc_res, "R%d", res_reg+1);

            emitTC(iter->quad->op, tc_arg1, tc_arg2, tc_res);
        }
        iter = iter->next;
        inst++;
    }
    freeRegisters();
}

// prints the target code to a file
void print_TargetCode() {
    quadTable* iter = T_Head;
    int target_inst = 1, block_count = 1;

    FILE *f = fopen("target_code.txt", "w");

    fprintf(f, "Block %d\n", block_count++);
    while(iter) {
        // if the instruction is the leader of a block, print the block number
        if(inst_to_target[block_leaders[block_count]] == target_inst) {
            fprintf(f, "\nBlock %d\n", block_count++);
        }

        char* tc_op;
        if(iter->quad->op < 10) {
            tc_op = strdup(relation_target[iter->quad->op]);
        }
        else {
            if(iter->quad->op == '+') tc_op = strdup("ADD");
            else if(iter->quad->op == '-') tc_op = strdup("SUB");
            else if(iter->quad->op == '*') tc_op = strdup("MUL");
            else if(iter->quad->op == '/') tc_op = strdup("DIV");
            else if(iter->quad->op == '%') tc_op = strdup("REM");
        }

        fprintf(f, "\t%d\t: %s ", target_inst, tc_op);
        
        // jump instruction (unconditional)
        if(iter->quad->op == JUMP) { 
            fprintf(f, "%d\n", inst_to_target[atoi(iter->quad->res)]);
        }
        // conditional jump
        else if(iter->quad->op < 6) {
            fprintf(f, "%s %s %d\n", iter->quad->arg1, iter->quad->arg2, inst_to_target[atoi(iter->quad->res)]);
        }
        // load immediate or load type
        else if(iter->quad->op == LDI || iter->quad->op == LD) { 
            fprintf(f, "%s %s\n", iter->quad->res, iter->quad->arg1);
        }
        // store type
        else if(iter->quad->op == ST) {
            fprintf(f, "%s %s\n", iter->quad->arg1, iter->quad->res);
        }
        // arithmetic type
        else {
            fprintf(f, "%s %s %s\n", iter->quad->res, iter->quad->arg1, iter->quad->arg2);
        } 
        iter = iter->next;
        target_inst++;
    }
    fprintf(f, "\n\t%d\t: ", target_inst);
    fclose(f);  
}

int main(int argc, char* argv[]) {
    MAX_REG = atoi(argv[1]);
    RegBank = (reg*)malloc(MAX_REG*sizeof(reg));
    memset(block_begin, 0, 1000*sizeof(int));
    memset(block_leaders, 0, 1000*sizeof(int));

    if(yyparse()) {
        return 0;
    }

    instruction_count++; 
    print_IntCode();
    convertToTargetCode();
    inst_to_target[instruction_count] = target_inst_count + 1;
    print_TargetCode();
}