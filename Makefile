CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -g -Wall
LIBS = -lfl # Linka com a biblioteca do Flex

SRCS = main.c ast.c symbol_table.c semantic_analyzer.c code_generator.c
OBJS = $(SRCS:.c=.o)
TARGET = cminus_compiler

all: $(TARGET)

$(TARGET): cminus.tab.o lex.yy.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Regra para gerar o Parser (cminus.tab.c e cminus.tab.h)
cminus.tab.c cminus.tab.h: cminus.y
	$(BISON) -d -y cminus.y

# Regra para gerar o Scanner (lex.yy.c)
lex.yy.c: cminus.l
	$(FLEX) cminus.l

# Regra para compilar arquivos .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(TARGET) *.o *.tab.c *.tab.h lex.yy.c

.PHONY: all clean