all: p07

p07: pingpong-maintask.o pingpong.o queue.o
	gcc -o p07 pingpong-maintask.o pingpong.o queue.o

queue.o: queue.c
	gcc -o queue.o -c queue.c 

pingpong.o: pingpong.c queue.h datatypes.h
	gcc -o pingpong.o -c pingpong.c 

pingpong-maintask.o: pingpong-maintask.c pingpong.h
	gcc -o pingpong-maintask.o -c pingpong-maintask.c
