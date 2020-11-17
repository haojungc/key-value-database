CC = gcc
CFLAGS = -Wall
EXE = main
OBJS = utils.o database.o

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) main.c $(OBJS) -o $(EXE)

utils.o: utils.c
	$(CC) $(CFLAGS) -c utils.c

database.o: database.c
	$(CC) $(CFLAGS) -c database.c

clean:
	rm -f *.o $(EXE)