all: 
	yacc -d expr.y
	lex expr.l
	gcc -o icgen intcodegen.c

run: all
	./icgen < sample.txt > intcode.c
	gcc -o ic intcode.c
	./ic

clean:
	rm -f *.out ic icgen y.tab.h y.tab.c lex.yy.c output.txt intcode.c