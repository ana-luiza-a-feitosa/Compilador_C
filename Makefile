CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

TARGET = cminus
OBJS = main.o util.o scan.o parse.o symtab.o analyze.o codegen.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c globals.h
	$(CC) $(CFLAGS) -c main.c

util.o: util.c globals.h
	$(CC) $(CFLAGS) -c util.c

scan.o: scan.c globals.h
	$(CC) $(CFLAGS) -c scan.c

parse.o: parse.c globals.h
	$(CC) $(CFLAGS) -c parse.c

symtab.o: symtab.c globals.h
	$(CC) $(CFLAGS) -c symtab.c

analyze.o: analyze.c globals.h
	$(CC) $(CFLAGS) -c analyze.c

codegen.o: codegen.c globals.h
	$(CC) $(CFLAGS) -c codegen.c

clean:
	rm -f $(TARGET) $(OBJS) *.tm

run: $(TARGET)
	./$(TARGET) test.cm

.PHONY: all clean run