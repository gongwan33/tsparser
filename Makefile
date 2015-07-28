OBJS=main.o utils.o tsparser.o
EXECNAME=test
CC=gcc
CFLAGS=-I./include -g 

main:$(OBJS)
	$(CC) $(OBJS) -o $(EXECNAME)

main.o:main.c ./include/errnum.h
	$(CC) $(CFLAGS) -c $< -o $@

utils.o:utils.c ./include/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

tsparser.o:tsparser.c ./include/tsparser.h
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm $(EXECNAME) *.o tmp.dat outPutProgram.ts


