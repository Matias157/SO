#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "pingpong.h"
#include "queue.h"
#include <signal.h>
#include <sys/time.h>

#define STACKSIZE 32768

// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action ;

// estrutura de inicialização to timer
struct itimerval timer;

int cont = 1;
unsigned int ticks = 0, processor_time_init;	// inicializa o número de ticks total e o número de ticks inicial de cada tempo de processamento
task_t MainTask, *TaskCurrent, *TaskOld, *SuspendQueue, *ReadyQueue, Dispatcher;

void timer_handler(){ //tratador do timer
	ticks++;
	if(ticks%20 == 0)
		task_yield();
}

task_t *scheduler(){
	task_t *task_mais_prioritaria = ReadyQueue, *task_comparacao = ReadyQueue->next; // a task mais prioritaria é a com menor prioridade
	while(task_comparacao != ReadyQueue){											// avalia até voltar a cabeça da fila
		//printf("%d %d\n", task_mais_prioritaria->prio_dim, task_comparacao-> prio_dim);
		if(task_mais_prioritaria->prio_dim >= task_comparacao->prio_dim){			// se a task mais prioritaria até então tiver prioridade menor
			if(task_mais_prioritaria->prio_dim > -20){								// e se a task mais prioritaria não tiver prioridade menor que -20
				task_mais_prioritaria->prio_dim--;									// envenhece a prioridade da task mais prioritaria
			}
			task_mais_prioritaria = task_comparacao;								// a task mais prioritária pasa a ser a avaliada
		}
		else{																		// se a task mais prioritária tiver ainda a maior prioridade
			if(task_comparacao->prio_dim > -20){										// e se não tiver prioridade menor que -20	
				task_comparacao->prio_dim--;										// a task compara envelhece		
			}
		}
		task_comparacao = task_comparacao->next;									// compara com a task na sequencia
	}
	return((task_t*)queue_remove((queue_t**)&ReadyQueue, (queue_t*)task_mais_prioritaria));	// retorna a task mais prioritária	
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

void task_setprio (task_t *task, int prio){
	if(prio > 20 || prio < -20){
		printf("Prioridade fora dos limites permitidos\n");
	}
	else{
		if(task == NULL){	// se a tarefa for nula, seta a prioridade da tarefa atual
			TaskCurrent->prio_est = prio;
		}
		else{
			task->prio_est = prio;
			task->prio_dim = prio;
		}
	}
}

int task_getprio (task_t *task){
	task_t *aux = task;
	if(task == NULL){
		aux = TaskCurrent;
	}
	return(aux->prio_est);
}

void pingpong_init (){
	getcontext(&MainTask.context);
	MainTask.prev = NULL;
	MainTask.id = 0;
	MainTask.next = NULL;
	MainTask.status = Running;
	TaskCurrent = &MainTask;
	action.sa_handler = timer_handler;
    sigemptyset (&action.sa_mask);
    action.sa_flags = 0;
    if(sigaction (SIGALRM, &action, 0) < 0){
    	perror ("Erro em sigaction: ");
    	exit (1);
    }
    // ajusta valores do temporizador
    timer.it_value.tv_usec = 1000;      // primeiro disparo, em micro-segundos
    timer.it_interval.tv_usec = 1000;   // disparos subsequentes, em micro-segundos
    if(setitimer (ITIMER_REAL, &timer, 0) < 0){ //arma o temporizador ITIMER_REAL
        perror ("Erro em setitimer: ") ;	
        exit (1) ;
    }
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

	if(task != &Dispatcher){
		queue_append((queue_t**)&ReadyQueue,(queue_t*)task);
	}

	makecontext(&(task->context), (void*)(*start_func), 1, arg);

	task->execution_time = systime();	// salva em qual tempo se iniciou a tarefa
	task->processor_time = 0;			// inicia em 0 porque não foi usado tempo de processador para executá-la ainda
	task->activations = 0;				// mesmo motivo do de cima
	task->status = Ready;
	task->prio_est = 0;
	task->prio_dim = 0;
	task->id = cont;
	cont++;

	return(task->id);
}

int task_switch (task_t *task){
	TaskCurrent->processor_time += (systime() - processor_time_init); // tarefa que já usou o processador recebe a soma do tempo de uso antigo com o tempo usado na última vez
	processor_time_init = systime();	// reinicializa o tempo de processamento para a próxima tarefa

	task->status = Running;
	TaskOld = TaskCurrent;
	TaskCurrent = task;
	TaskCurrent->activations++;	// contabiliza o número de activations já que está tarefa será executada logo em seguida
	swapcontext(&(TaskOld->context), &(TaskCurrent->context));
	return(0);
}

void task_exit (int exitCode){
	TaskCurrent->execution_time = (systime() - TaskCurrent->execution_time); // contabiliza o tempo total de execução calculando o intervalo de tempo entre o que foi salvo inicialmente e o atual
	if(TaskCurrent == &Dispatcher)
		printf("Dispatcher FIM\n");
	printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", TaskCurrent->id, TaskCurrent->execution_time, TaskCurrent->processor_time, TaskCurrent->activations);
	if(TaskCurrent == &Dispatcher){
		task_switch(&MainTask);
	}
	else{
		task_switch(&Dispatcher);
	}
}

int task_id (){
	return(TaskCurrent->id);
}

void task_yield (){
	if(TaskCurrent != &MainTask){
		//ver se coloca para voltar à prioridade original aqui ou não
		TaskCurrent->prio_dim = TaskCurrent->prio_est; // acho que sim
		queue_append((queue_t**)&ReadyQueue,(queue_t*)TaskCurrent);
		TaskCurrent->status = Ready;
	}
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

unsigned int systime (){ //retorna o número de ticks desde o início do programa
	return ticks;
}