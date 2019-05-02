#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "pingpong.h"
#include "queue.h"

#define STACKSIZE 32768

int cont = 1;
task_t MainTask, *TaskCurrent, *TaskOld, *SuspendQueue, *ReadyQueue, Dispatcher;

task_t *scheduler(){
	return((task_t*)queue_remove((queue_t**)&ReadyQueue, (queue_t*)ReadyQueue));
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
	MainTask.status = Running;
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

	if(task != &Dispatcher)
		queue_append((queue_t**)&ReadyQueue,(queue_t*)task);

	makecontext(&(task->context), (void*)(*start_func), 1, arg);

	task->status = Ready;

	task->id = cont;
	cont++;

	return(task->id);
}

int task_switch (task_t *task){
	task->status = Running;
	TaskOld = TaskCurrent;
	TaskCurrent = task;
	swapcontext(&(TaskOld->context), &(TaskCurrent->context));
	return(0);
}

void task_exit (int exitCode){
	if(TaskCurrent == &Dispatcher)
		task_switch(&MainTask);
	else
		task_switch(&Dispatcher);
}

int task_id (){
	return(TaskCurrent->id);
}

void task_yield (){
	if(TaskCurrent != &MainTask)
		queue_append((queue_t**)&ReadyQueue,(queue_t*)TaskCurrent);
		TaskCurrent->status = Ready;
	task_switch(&Dispatcher);
}

void task_suspend (task_t *task, task_t **queue){
	if(queue == NULL)
		return;
	else if(task == NULL){
		if(TaskCurrent == &Dispatcher || TaskCurrent == &MainTask)
			return;
		else{
			queue_remove((queue_t**)&ReadyQueue,(queue_t*)TaskCurrent);
			queue_append((queue_t**)queue,(queue_t*)TaskCurrent);
			TaskCurrent->status = Suspended;
			return;
		}	
	}
	else{
		queue_remove((queue_t**)&ReadyQueue,(queue_t*)task);
		queue_append((queue_t**)queue,(queue_t*)task);
		task->status = Suspended;
	}
}

void task_resume (task_t *task){
	queue_remove((queue_t**)&SuspendQueue,(queue_t*)task);
	queue_append((queue_t**)&ReadyQueue,(queue_t*)task);
	task->status = Ready;
}