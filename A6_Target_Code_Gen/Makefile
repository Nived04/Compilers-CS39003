all: 
	yacc -d prog.y
	lex prog.l
	gcc -g codegen.c

run: all
	./a.out < sample.txt 5

clean:
	rm -f *.out lex.yy.c *.tab.h *.tab.c intermediate_code.txt target_code.txt