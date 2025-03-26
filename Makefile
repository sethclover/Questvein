CC = gcc -g # -g for debugging
CFLAGS = -Wall -Werror
OBJECTS = dungeonGeneration.o errorHandle.o fibonacciHeap.o game.o main.o pathFinding.o perlin.o saveLoad.o

dungeon: $(OBJECTS)
	$(CC) $(OBJECTS) -o dungeon $(CFLAGS) -lm -lncurses

dungeonGeneration.o: dungeonGeneration.c dungeon.h errorHandle.h pathFinding.h perlin.h
	$(CC) -c dungeonGeneration.c -o dungeonGeneration.o $(CFLAGS)

errorHandle.o: errorHandle.c errorHandle.h
	$(CC) -c errorHandle.c -o errorHandle.o $(CFLAGS)

fibonacciHeap.o: fibonacciHeap.c dungeon.h errorHandle.h fibonacciHeap.h
	$(CC) -c fibonacciHeap.c -o fibonacciHeap.o $(CFLAGS)

game.o: game.c dungeon.h errorHandle.h fibonacciHeap.h
	$(CC) -c game.c -o game.o $(CFLAGS)

main.o: main.c dungeon.h errorHandle.h game.h pathFinding.h saveLoad.h
	$(CC) -c main.c -o main.o $(CFLAGS)

pathFinding.o: pathFinding.c dungeon.h fibonacciHeap.h
	$(CC) -c pathFinding.c -o pathFinding.o $(CFLAGS)

perlin.o: perlin.c dungeon.h
	$(CC) -c perlin.c -o perlin.o $(CFLAGS)

saveLoad.o: saveLoad.c dungeon.h errorHandle.h
	$(CC) -c saveLoad.c -o saveLoad.o $(CFLAGS)

clean:
	rm -f dungeon *.o *~
