#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "pingpong.h"

#define STACKSIZE 32768

int cont = 1;
task_t MainTask, *TaskCurrent, *TaskOld;

void pingpong_init (){
	getcontext(&MainTask.context); //salva o contexto atual na task main
	MainTask.prev = NULL; //não iniciamos prev
	MainTask.id = 0; //id da main é 0
	MainTask.next = NULL; //não iniciamos next
	TaskCurrent = &MainTask; //primeiramente a task atual é a main

	setvbuf(stdout , 0, _IONBF, 0); //desativa o buffer da saida padrao (stdout), usado pela função printf
}

int task_create (task_t *task, void (*start_func)(void *), void *arg){
	getcontext(&(task->context)); //salva o contexto atual na task

	task->next = NULL; //não iniciamos next
	task->prev = NULL; //não iniciamos prev

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

	makecontext(&(task->context), (void*)(*start_func), 1, arg); //modifica o contexto de task para start_func

	task->id = cont; //atualiza os ids das tasks
	cont++; //incrementa o contador global dos ids

	return(task->id); //retorna o id da tarefa que foi criada
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
	return(TaskCurrent->id); //retorna o id da task atual
}