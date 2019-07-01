// Alunos: Alexandre Herrero Matias e Giuliana Martins Silva

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include "pingpong.h"
#include "queue.h"

#define STACKSIZE 32768

// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action ;

// estrutura de inicialização to timer
struct itimerval timer;

int cont = 1;
int ticks = 0;
task_t MainTask, *TaskCurrent, *TaskOld, *SuspendQueue, *ReadyQueue, Dispatcher;

void timer_handler(){ //tratador do timer
	ticks++; //incrementa o contador global de ticks
	printf("entrou no handler");
	if(ticks%20 == 0){ //se chegar a 20
		task_yield(); //retorna a fila de prontas e chama a próxima tarefa
	}
}

task_t *scheduler(){
	return((task_t*)queue_remove((queue_t**)&ReadyQueue, (queue_t*)ReadyQueue)); //retira a primeira tarefa da fila de prontas e a retorna
}

void dispatcher_body(){
	while(queue_size((queue_t*) ReadyQueue) > 0){ //verifica se a fila de prontas não esta vazia
		task_t *next = scheduler(); //cria uma task auxiliar que recebe a primeira tarefa da fila de prontas
		if(next){ //se next não for NULL
			task_switch(next); //da o processador para a primeira tarefa da fila
		}
	}
	task_exit(0);
}

void pingpong_init (){
	getcontext(&MainTask.context); //salva o contexto atual na task main
	MainTask.prev = NULL; //não iniciamos prev
	MainTask.id = 0; //id da main é 0
	MainTask.next = NULL; //não iniciamos next
	MainTask.status = Running; //status da main sempre é Running
	TaskCurrent = &MainTask; //primeiramente a task atual é a main

	//inicialização do timer análoga aos códigos-exemplo
	action.sa_handler = timer_handler;
    sigemptyset (&action.sa_mask);
    action.sa_flags = 0;
    if(sigaction (SIGALRM, &action, 0) < 0){
    	perror ("Erro em sigaction: ");
    	exit (1);
    }
    //ajusta valores do temporizador
    timer.it_value.tv_usec = 1000;      // primeiro disparo, em micro-segundos
    timer.it_interval.tv_usec = 1000;   // disparos subsequentes, em micro-segundos
    if(setitimer (ITIMER_REAL, &timer, 0) < 0){ //arma o temporizador ITIMER_REAL
        perror ("Erro em setitimer: ") ;
        exit (1) ;
    }
    task_create(&Dispatcher, dispatcher_body, ""); //cria o dispatcher

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

	if(task != &Dispatcher)
		queue_append((queue_t**)&ReadyQueue,(queue_t*)task); //insere task no final da fila de prontas

	makecontext(&(task->context), (void*)(*start_func), 1, arg); //modifica o contexto de task para start_func

	task->status = Ready; //status das tarefas na fila de prontas é Ready

	task->id = cont; //atualiza os ids das tasks
	cont++; //incrementa o contador global dos ids

	return(task->id); //retorna o id da tarefa que foi criada
}

int task_switch (task_t *task){
	task->status = Running; //o status de task vai para Running
	TaskOld = TaskCurrent; //usamos TaskOld como variavel auxiliar
	TaskCurrent = task; //task vira nossa task atual
	swapcontext(&(TaskOld->context), &(TaskCurrent->context)); //trocamos o contexto para task
	return(0);
}

void task_exit (int exitCode){
	if(TaskCurrent == &Dispatcher) //caso a tarefa atual seja o dispatcher devemos voltar para a main
		task_switch(&MainTask);
	else
		task_switch(&Dispatcher); //caso seja qualquer outra tarefa retornamos para o dispatcher
}

int task_id (){
	return(TaskCurrent->id); //retorna o id da task atual
}

void task_yield (){
	if(TaskCurrent != &MainTask){
		queue_append((queue_t**)&ReadyQueue,(queue_t*)TaskCurrent); //insere a tarefa no final da fila de prontas
		TaskCurrent->status = Ready; //status da task atual volta para Ready
	}
	task_switch(&Dispatcher); //volta para o dispatcher
}

void task_suspend (task_t *task, task_t **queue){
	if(queue == NULL) //caso a fila seja nula nada é feito
		return;
	else if(task == NULL){
		if(TaskCurrent == &Dispatcher || TaskCurrent == &MainTask) //não podemos suspender o dispatcher nem a main
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
		task->status = Suspended; //status de task vai para Suspended
	}
}

void task_resume (task_t *task){
	queue_remove((queue_t**)&SuspendQueue,(queue_t*)task); //remove a tarefa da fila de suspensas
	queue_append((queue_t**)&ReadyQueue,(queue_t*)task); //insere a tarefa no final da fila de prontas
	task->status = Ready; //status de task volta para Ready
}