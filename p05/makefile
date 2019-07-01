all: p05

p05: pingpong-preempcao.o pingpong.o queue.o
	gcc -o p05 pingpong-preempcao.o pingpong.o queue.o

queue.o: queue.c
	gcc -o queue.o -c queue.c 

pingpong.o: pingpong.c queue.h datatypes.h
	gcc -o pingpong.o -c pingpong.c 

pingpong-preempcao.o: pingpong-preempcao.c pingpong.h
	gcc -o pingpong-preempcao.o -c pingpong-preempcao.c