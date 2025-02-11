CC = gcc
CFLAGS = -Wall -Werror

dungeonGeneration: dungeonGeneration.o perlin.o
	$(CC) dungeonGeneration.o perlin.o -o dungeonGeneration $(CFLAGS) -lm

dungeonGeneration.o: dungeonGeneration.c dungeon.h
	$(CC) -c dungeonGeneration.c -o dungeonGeneration.o $(CFLAGS)

perlin.o: perlin.c dungeon.h
	$(CC) -c perlin.c -o perlin.o $(CFLAGS)

clean:
	rm -f dungeonGeneration *.o *~