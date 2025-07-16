#
# File Name		: Makefile
# Description	: Makefile for project #3.
#

CC = gcc
LEX = flex
YACC = bison

CFLAGS = -g
#CFLAGS = -fsanitize=address -static-libasan -g
LDFLAGS = -lfl 
YACCFLAGCS = -vd

OBJECTS = lex.yy.o subc.tab.o

all : subc

clean :
	rm -f *.o lex.yy.c subc.tab.c subc.tab.h subc.output subc

subc : ${OBJECTS}
	${CC} -o subc ${OBJECTS} ${CFLAGS} ${LDFLAGS}

subc.tab.o : subc.tab.c subc.h
	${CC} -c ${CFLAGS} subc.tab.c

lex.yy.o : lex.yy.c subc.tab.h subc.h
	${CC} -c ${CFLAGS} lex.yy.c

lex.yy.c : subc.l
	${LEX} subc.l

subc.tab.h subc.tab.c : subc.y
	${YACC} ${YACCFLAGCS} subc.y
