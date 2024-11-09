all: 
	yacc -d prog.y
	lex prog.l
	gcc -g codegen.c

run: all
	./a.out < sample.txt > output.txt

clean:
	rm -f *.out lex.yy.c *.tab.h *.tab.c output.txt