all: p11

p11: pingpong-barrier.o pingpong.o queue.o
	gcc -o p11 pingpong-barrier.o pingpong.o queue.o

queue.o: queue.c
	gcc -o queue.o -c queue.c 

pingpong.o: pingpong.c queue.h datatypes.h
	gcc -o pingpong.o -c pingpong.c 

pingpong-barrier.o: pingpong-barrier.c pingpong.h
	gcc -o pingpong-barrier.o -c pingpong-barrier.c