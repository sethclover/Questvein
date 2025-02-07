CC = gcc
CFLAGS = -Wall -Werror

dungeonGeneration: dungeonGeneration.o
	$(CC) dungeonGeneration.o -o dungeonGeneration $(CFLAGS)

dungeonGeneration.o: dungeonGeneration.c dungeon.h
	$(CC) -c dungeonGeneration.c -o dungeonGeneration.o $(CFLAGS)

clean:
	rm -f dungeonGeneration *.o *~