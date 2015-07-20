OBJS=main.o
EXECNAME=test
CC=gcc
CFLAGS=-I./include

main:$(OBJS)
	$(CC) $(OBJS) -o $(EXECNAME)

main.o:main.c ./include/tsparser.h
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm $(EXECNAME) *.o


