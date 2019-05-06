all: p04

p04: pingpong-scheduler.o pingpong.o queue.o
	gcc -o p04 pingpong-scheduler.o pingpong.o queue.o

queue.o: queue.c
	gcc -o queue.o -c queue.c 

pingpong.o: pingpong.c queue.h datatypes.h
	gcc -o pingpong.o -c pingpong.c 

pingpong-preempcao.o: pingpong-scheduler.c pingpong.h
	gcc -o pingpong-scheduler.o -c pingpong-scheduler.c
