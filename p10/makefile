all: p10

p10: pingpong-semaphore.o pingpong.o queue.o
	gcc -o p10 pingpong-semaphore.o pingpong.o queue.o

queue.o: queue.c
	gcc -o queue.o -c queue.c 

pingpong.o: pingpong.c queue.h datatypes.h
	gcc -o pingpong.o -c pingpong.c 

pingpong-semaphore.o: pingpong-semaphore.c pingpong.h
	gcc -o pingpong-semaphore.o -c pingpong-semaphore.c

all: p10b

p10b: pingpong-racecond.o pingpong.o queue.o
	gcc -o p10b pingpong-racecond.o pingpong.o queue.o

queue.o: queue.c
	gcc -o queue.o -c queue.c 

pingpong.o: pingpong.c queue.h datatypes.h
	gcc -o pingpong.o -c pingpong.c 

pingpong-racecond.o: pingpong-racecond.c pingpong.h
	gcc -o pingpong-racecond.o -c pingpong-racecond.c
