all: p06

p06: pingpong-contab-prio.o pingpong.o queue.o
	gcc -o p06 pingpong-contab-prio.o pingpong.o queue.o

queue.o: queue.c
	gcc -o queue.o -c queue.c 

pingpong.o: pingpong.c queue.h datatypes.h
	gcc -o pingpong.o -c pingpong.c 

pingpong-contab-prio.o: pingpong-contab-prio.c pingpong.h
	gcc -o pingpong-contab-prio.o -c pingpong-contab-prio.c
