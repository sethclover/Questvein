dungeonGeneration: dungeonGeneration.c
	gcc dungeonGeneration.c -o dungeonGeneration -Wall -Werror

clean:
	rm -f dungeonGeneration *~