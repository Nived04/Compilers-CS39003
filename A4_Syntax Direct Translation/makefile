all:
	yacc -d poly.y
	lex poly.l
	gcc polyutils.c -lm

run: all sample.txt
	./a.out < sample.txt > output.txt

clean:
	-rm -f a.out y.tab.* lex.yy.c output.txt