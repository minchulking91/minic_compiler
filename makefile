ast_gen : parser.tab.o lex.yy.o
	gcc -lm -o $@ parser.tab.o lex.yy.o -lfl -L C:\MingW\msys\1.0\lib

parser.tab.o : parser.tab.c Node.h
	gcc -c $<

parser.tab.c : parser.y
	bison $< --defines

lex.yy.o : lex.yy.c parser.tab.h Node.h
	gcc -c $<

lex.yy.c : scanner.l
	flex $<
