CC = gcc
CFLAGS = -Wall -O0
EXE = main
OBJS = utils.o database.o

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) main.c $(OBJS) -o $(EXE)

gen: utils.o
	$(CC) $(CFLAGS) -o gen cmd_generator.c utils.o

utils.o: utils.c
	$(CC) $(CFLAGS) -c utils.c

database.o: database.c
	$(CC) $(CFLAGS) -c database.c

.PHONY: clean
clean:
	rm -f *.o $(EXE) gen