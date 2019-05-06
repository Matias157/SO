#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "pingpong.h"

#define STACKSIZE 32768

int cont = 1;
task_t MainTask, *TaskCurrent, *TaskOld;

void pingpong_init (){
	getcontext(&MainTask.context); //salva o contexto atual na task main
	MainTask.prev = NULL;
	MainTask.id = 0;
	MainTask.next = NULL;
	TaskCurrent = &MainTask;

	setvbuf(stdout , 0, _IONBF, 0); //desativa o buffer da saida padrao (stdout), usado pela função printf
}

int task_create (task_t *task, void (*start_func)(void *), void *arg){
	getcontext(&(task->context)); //salva o contexto atual na task

	task->next = NULL;
	task->prev = NULL;

	//faz a criação da pilha
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

	//modifica o contexto de task para start_func
	makecontext(&(task->context), (void*)(*start_func), 1, arg);

	//atualiza os ids das tasks
	task->id = cont;
	cont++;

	return(task->id);
}

int task_switch (task_t *task){
		TaskOld = TaskCurrent; //usamos TaskOld como variavel auxiliar
		TaskCurrent = task; //task vira nossa task atual
		swapcontext(&(TaskOld->context), &(TaskCurrent->context)); //trocamos o contexto para task
		return(0);
}

void task_exit (int exitCode){
	task_switch(&MainTask); //voltamos para a main sempre que uma tarefa termina
}

int task_id (){
	return(TaskCurrent->id);
}