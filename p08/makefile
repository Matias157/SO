all: p08

p08: pingpong-join.o pingpong.o queue.o
	gcc -o p08 pingpong-join.o pingpong.o queue.o

queue.o: queue.c
	gcc -o queue.o -c queue.c 

pingpong.o: pingpong.c queue.h datatypes.h
	gcc -o pingpong.o -c pingpong.c 

pingpong-sleep.o: pingpong-join.c pingpong.h
	gcc -o pingpong-join.o -c pingpong-join.c