all: p09

p09: pingpong-sleep.o pingpong.o queue.o
	gcc -o p09 pingpong-sleep.o pingpong.o queue.o

queue.o: queue.c
	gcc -o queue.o -c queue.c 

pingpong.o: pingpong.c queue.h datatypes.h
	gcc -o pingpong.o -c pingpong.c 

pingpong-sleep.o: pingpong-sleep.c pingpong.h
	gcc -o pingpong-sleep.o -c pingpong-sleep.c