#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "pingpong.h"
#include "queue.h"

#define STACKSIZE 32768

int cont = 1;
task_t MainTask, *TaskCurrent, *TaskOld, *SuspendQueue, *ReadyQueue, Dispatcher;

task_t *scheduler(){
	return((task_t*)queue_remove((queue_t**)&ReadyQueue, (queue_t*)ReadyQueue)); //retira a primeira tarefa da fila de prontas e a retorna
}

void dispatcher_body(){
	while(queue_size((queue_t*) ReadyQueue) > 0){ //verifica se a fila de prontas não esta vazia
		task_t *next = scheduler();
		if(next){
			task_switch(next); //da o processador para a primeira tarefa da fila
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
	task_create(&Dispatcher, dispatcher_body, ""); //cria o dispatcher

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
		queue_append((queue_t**)&ReadyQueue,(queue_t*)task); //insere a nova tarefa no final da fila de prontas

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
	if(TaskCurrent == &Dispatcher) //caso a tarefa atual seja o dispatcher devemos voltar para a main
		task_switch(&MainTask);
	else
		task_switch(&Dispatcher); //caso seja qualquer outra tarefa retornamos para o dispatcher
}

int task_id (){
	return(TaskCurrent->id);
}

void task_yield (){
	if(TaskCurrent != &MainTask)
		queue_append((queue_t**)&ReadyQueue,(queue_t*)TaskCurrent); //insere a tarefa no final da fila de prontas
		TaskCurrent->status = Ready;
	task_switch(&Dispatcher); //volta para o dispatcher
}

void task_suspend (task_t *task, task_t **queue){
	if(queue == NULL)
		return;
	else if(task == NULL){
		if(TaskCurrent == &Dispatcher || TaskCurrent == &MainTask)
			return;
		else{
			queue_remove((queue_t**)&ReadyQueue,(queue_t*)TaskCurrent); //remove a tarefa da fila de prontas
			queue_append((queue_t**)queue,(queue_t*)TaskCurrent); //insere a tarefa no final da fila queue
			TaskCurrent->status = Suspended;
			return;
		}	
	}
	else{
		queue_remove((queue_t**)&ReadyQueue,(queue_t*)task); //remove a tarefa da fila de prontas
		queue_append((queue_t**)queue,(queue_t*)task); //insere a tarefa no final da fila queue
		task->status = Suspended;
	}
}

void task_resume (task_t *task){
	queue_remove((queue_t**)&SuspendQueue,(queue_t*)task); //remove a tarefa da fila de suspensas
	queue_append((queue_t**)&ReadyQueue,(queue_t*)task); //insere a tarefa no final da fila de prontas
	task->status = Ready;
}