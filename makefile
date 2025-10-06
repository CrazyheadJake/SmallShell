CC=gcc --std=gnu99 -g

smallsh: command.o dynarray.o main.c
	$(CC) main.c command.o dynarray.o -o smallsh

command.o: command.c command.h
	$(CC) -c command.c

dynarray.o: dynarray.c dynarray.h
	$(CC) -c dynarray.c

run: smallsh
	./smallsh

clean:
	rm -f *.o smallsh
