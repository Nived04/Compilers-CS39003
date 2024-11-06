all: 
	@yacc -d expr.y
	@lex expr.l
	@gcc -g expr.c

run: all
	@./a.out < sample.txt

clean:
	rm -f *.out y.tab.h y.tab.c lex.yy.c