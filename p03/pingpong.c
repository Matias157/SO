#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "pingpong.h"
#include "queue.h"

#define STACKSIZE 32768

int cont = 1;
task_t MainTask, *TaskCurrent, *TaskOld, *SuspendQueue, *ReadyQueue, Dispatcher;

task_t *scheduler(){
	task_t *aux;
	aux = ReadyQueue;
	queue_remove((queue_t**)&ReadyQueue, (queue_t*)ReadyQueue);
	return(aux); //retorna o primeiro elemento da fila de tarefas
}

void dispatcher_body(){
	while(queue_size((queue_t*) ReadyQueue) > 0){
		task_t *next = scheduler();
		if(next){
			task_switch(next);
		}
	}
	task_exit(0);
}

void pingpong_init (){
	getcontext(&MainTask.context);
	MainTask.prev = NULL;
	MainTask.id = 0;
	MainTask.next = NULL;
	TaskCurrent = &MainTask;
	task_create(&Dispatcher, dispatcher_body, "");

	setvbuf(stdout , 0, _IONBF, 0);
}

int task_create (task_t *task, void (*start_func)(void *), void *arg){
	getcontext(&(task->context));

	task->next = NULL;
	task->prev = NULL;

	char *stack ;

	stack = malloc (STACKSIZE) ;
	if (stack){
		task->context.uc_stack.ss_sp = stack ;
		task->context.uc_stack.ss_size = STACKSIZE;
		task->context.uc_stack.ss_flags = 0;
		task->context.uc_link = 0;
	}
	else{
		perror ("Erro na criação da pilha: ");
		exit (1);
	}

	queue_append((queue_t**)&ReadyQueue,(queue_t*)task);

	makecontext(&(task->context), (void*)(*start_func), 1, arg);

	task->id = cont;
	cont++;

	return(task->id);
}

int task_switch (task_t *task){
	TaskOld = TaskCurrent;
	TaskCurrent = task;
	swapcontext(&(TaskOld->context), &(TaskCurrent->context));
	return(0);
}

void task_exit (int exitCode){
	task_switch(&MainTask);
}

int task_id (){
	return(TaskCurrent->id);
}

void task_yield (){
	task_switch(&Dispatcher);
}