minic : parser.tab.o lex.yy.o ast.builder.o
	gcc -lm -o $@ parser.tab.c lex.yy.c parser.tab.h Node.h -lfl -L C:\MingW\msys\1.0\lib

parser.tab.c : parser.y 
	bison $<

lex.yy.c : scanner.l 
	flex $<
