typedef struct _symbol_locs {
	int location;
	struct _symbol_reg_locs* next;
}locs;

typedef struct _Symbol {
	char* name;
    int offset;
	locs reg_locs; 
	locs mem_locs;
}Symbol;

typedef struct _SymbolTable {
	Symbol* symbol;
	struct _SymbolTable* next;
}SymbolTable;

typedef struct _instruction_quadTable {
	int op;

}quad_inst;

typedef struct name_list { 
    char* var_name;
    struct name_list* next;
}name_list;


typedef struct _register {
    name_list reg_descriptor;
}reg;