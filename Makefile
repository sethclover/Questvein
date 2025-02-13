CC = gcc
CFLAGS = -Wall -Werror

dungeon: main.o dungeonGeneration.o perlin.o saveLoad.o
	$(CC) main.o dungeonGeneration.o perlin.o saveLoad.o -o dungeon $(CFLAGS) -lm

main.o: main.c dungeon.h
	$(CC) -c main.c -o main.o $(CFLAGS)

dungeonGeneration.o: dungeonGeneration.c dungeon.h
	$(CC) -c dungeonGeneration.c -o dungeonGeneration.o $(CFLAGS)

perlin.o: perlin.c dungeon.h
	$(CC) -c perlin.c -o perlin.o $(CFLAGS)

saveLoad.o: saveLoad.c dungeon.h
	$(CC) -c saveLoad.c -o saveLoad.o $(CFLAGS)

clean:
	rm -f dungeon *.o *~
