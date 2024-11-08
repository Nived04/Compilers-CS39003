all: 
	yacc -d prog.y
	lex prog.l
	gcc -g codegen.c

run: all
	./a.out < sample.txt

clean:
	rm -f *.out lex.yy.c y.tab.h y.tab.c