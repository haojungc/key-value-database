CC = gcc
CFLAGS = -Wall
EXE = main
OBJS = utils.o

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) main.c $(OBJS) -o $(EXE)

utils.o: utils.c
	$(CC) $(CFLAGS) -c utils.c

clean:
	rm -f *.o $(EXE)