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

// estrutura para pausar o timer
struct itimerval pause;

int cont = 1, tasksSem = 0, tasksBar = 0;
unsigned int ticks = 0, processor_time_init; // inicializa o número de ticks total e o número de ticks inicial de cada tempo de processamento
task_t MainTask, *TaskCurrent, *TaskOld, *ReadyQueue, Dispatcher, *SuspendQueue, *SleepQueue;

void timer_handler(){ //tratador do timer
	ticks++; //incrementa o contador global de ticks
    if(ticks%20 == 0 && TaskCurrent != &Dispatcher){
        task_yield();
	}
}

void timer_on(){
	if(setitimer (ITIMER_REAL, &timer, 0) < 0){ //arma o temporizador ITIMER_REAL
	  perror ("Erro em setitimer: ") ;
	  exit (1) ;
	}
}

void pause_timer(){
	if (setitimer(ITIMER_VIRTUAL, &pause, 0) < 0){
      	perror ("Erro em setitimer: ");
        exit (1);
	}
}

task_t *scheduler(){
	task_t* task_aux = SleepQueue;

	task_aux = SleepQueue;
	while(queue_size((queue_t*)ReadyQueue) == 0 && queue_size((queue_t*)SleepQueue) > 0){
		if(task_aux->wakeup <= ticks){
			task_t *aux = (task_t*)queue_remove((queue_t**)&SleepQueue, (queue_t*)task_aux);
			queue_append((queue_t**)&ReadyQueue,(queue_t*)aux);
		}
		task_aux = task_aux->next;
	}

	return((task_t*)queue_remove((queue_t**)&ReadyQueue, (queue_t*)ReadyQueue)); //retira a primeira tarefa da fila de prontas e a retorna
}

void dispatcher_body(){
    while(queue_size((queue_t*)ReadyQueue) > 0 || queue_size((queue_t*)SleepQueue) > 0 || tasksSem > 0 || tasksBar > 0){
    	timer_on();
    	task_t* next = scheduler();
    	if(next){
    		task_switch(next);
    	}
    }
    
	task_exit(0);
}

void pingpong_init (){

	getcontext(&MainTask.context); //salva o contexto atual na task main
	MainTask.prev = NULL; //não iniciamos prev
	MainTask.id = 0; //id da main é 0
	MainTask.next = NULL; //não iniciamos next
	MainTask.status = Ready; //status da main sempre é Running
	MainTask.parent_id = -1;
	MainTask.parent_excd = -1;
	MainTask.wakeup = 0;
	TaskCurrent = &MainTask; //primeiramente a task atual é a main

	//inicialização do timer análoga aos códigos-exemplo
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
	timer_on();
	task_create(&Dispatcher, dispatcher_body, ""); //cria o dispatcher

	setvbuf(stdout , 0, _IONBF, 0); //desativa o buffer da saida padrao (stdout), usado pela função printf

	task_yield();
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

	task->execution_time = systime();   // salva em qual tempo se iniciou a tarefa
	task->processor_time = 0;        // inicia em 0 porque não foi usado tempo de processador para executá-la ainda
	task->activations = 0;           // mesmo motivo do de cima
	task->status = Ready; //status das tarefas na fila de prontas é Ready
	task->parent_id = -1;
	task->parent_excd = -1;
	task->wakeup = 0;

	task->id = cont; //atualiza os ids das tasks
	cont++; //incrementa o contador global dos ids

	return(task->id); //retorna o id da tarefa que foi criada
}

int task_switch (task_t *task){
	TaskCurrent->processor_time += (systime() - processor_time_init); // tarefa que já usou o processador recebe a soma do tempo de uso antigo com o tempo usado na última vez
	processor_time_init = systime(); // reinicializa o tempo de processamento para a próxima tarefa

	task->status = Running; //o status de task vai para Running
	TaskOld = TaskCurrent; //usamos TaskOld como variavel auxiliar
	TaskCurrent = task; //task vira nossa task atual
	TaskCurrent->activations++;   // contabiliza o número de activations já que está tarefa será executada logo em seguida
	swapcontext(&(TaskOld->context), &(TaskCurrent->context)); //trocamos o contexto para task
	return(0);
}

void task_exit (int exitCode){
	TaskCurrent->execution_time = (systime() - TaskCurrent->execution_time);   // contabiliza o tempo total de execução calculando o intervalo de tempo entre o que foi salvo inicialmente e o atual
	TaskCurrent->exit_code = exitCode;
	TaskCurrent->status = Terminated;

	if(SuspendQueue != NULL){
		task_t *aux = SuspendQueue;
		if(aux->parent_id == TaskCurrent->id){
			task_resume(aux);
		}
		aux = aux->next;

		while(aux != SuspendQueue && aux != NULL && SuspendQueue != NULL){
			if(aux->parent_id == TaskCurrent->id){
				task_resume(aux);
			}
			aux = aux->next;
		}
	}
	printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", TaskCurrent->id, TaskCurrent->execution_time, TaskCurrent->processor_time, TaskCurrent->activations);
	if(TaskCurrent == &Dispatcher)
		printf("Dispatcher FIM\n");
	else
		task_switch(&Dispatcher); //caso seja qualquer outra tarefa retornamos para o dispatcher
}

int task_id (){
	return(TaskCurrent->id); //retorna o id da task atual
}

void task_yield (){
	queue_append((queue_t**)&ReadyQueue,(queue_t*)TaskCurrent); //insere a tarefa no final da fila de prontas
	TaskCurrent->status = Ready; //status da task atual volta para Ready
	task_switch(&Dispatcher); //volta para o dispatcher
}

void task_suspend (task_t *task, task_t **queue){
	if(queue == NULL) //caso a fila seja nula nada é feito
		return;
	else if(task == NULL){
		if(TaskCurrent == &Dispatcher) //não podemos suspender o dispatcher
			return;
		else{
			queue_append((queue_t**)queue,(queue_t*)TaskCurrent); //insere a tarefa no final da fila queue
			TaskCurrent->status = Suspended; ///status de task vai para Suspended
			task_switch(&Dispatcher); //volta para o dispatcher
		}  
	}
	else{
		queue_remove((queue_t**)&ReadyQueue,(queue_t*)task); //remove a tarefa da fila de prontas
		queue_append((queue_t**)queue,(queue_t*)task); //insere a tarefa no final da fila queue
		task->status = Suspended; //status de task vai para Suspended
		task_switch(&Dispatcher); //volta para o dispatcher
	}
}

void task_resume (task_t *task){
	task_t *auxtask = (task_t*)queue_remove((queue_t**)&SuspendQueue,(queue_t*)task); //remove a tarefa da fila de suspensas
	queue_append((queue_t**)&ReadyQueue,(queue_t*)auxtask); //insere a tarefa no final da fila de prontas
	task->status = Ready; //status de task volta para Ready
	auxtask->parent_excd = task->exit_code; //atualiza o exit code da fila de suspensas com a task da tarefa que se esperava a conclusao	
}

unsigned int systime (){ //retorna o número de ticks desde o início do programa
	return ticks;
}

int task_join (task_t *task){
	if(task != NULL && task->status != Terminated){ //garante que não ira suspender uma task que já foi finalizada
		TaskCurrent->parent_id = task->id; //define a task_parent
		task_suspend(NULL, &SuspendQueue); //suspende a task
		return(TaskCurrent->parent_excd); //retorna o exit code da task que se esperava a conclusao
	}
	else{
		return(-1); //caso a tarefa seja nula retorna -1
	}
}

void task_sleep (int t){
	TaskCurrent->wakeup = t*1000 + ticks; //define o tempo que a task deve acordar
	TaskCurrent->status = Suspended; //muda o status da task
	queue_append((queue_t**)&SleepQueue,(queue_t*)TaskCurrent); //coloca a task na fila de adormecidas

	task_switch(&Dispatcher); //volta para o dDispatcher
}

// cria um semáforo com valor inicial "value"
int sem_create (semaphore_t *s, int value){
	if(s->status == 1){
		return(-1);		//semáforo já existente
	}
	s->value = value;
	s->status = 1;
	s->queue = NULL;
	return(0);
}

// requisita o semáforo
int sem_down (semaphore_t *s){
	if(s->status != 1){
		return(-1);		//semáforo inexistente ou destruído
	}
	//pause_timer();
	s->value--;
	if(s->value < 0){
		tasksSem++;
		//printf("task_suspend(NULL, &s->queue);\n");
		queue_append((queue_t**)&(s->queue),(queue_t*)TaskCurrent);; // suspende task atual e a coloca na fila do semáforo
		//printf("task_switch(&Dispatcher);\n");
		task_switch(&Dispatcher); //troca para o dispatcher
		//printf("saiu sem_down\n");
	}
	return(0);
}

// libera o semáforo
int sem_up (semaphore_t *s){
	task_t* acorda_task;
	if(s->status != 1){
		return(-1);		//semáforo inexistente ou destruído
	}
	s->value++;
	//timer_on();
	if(s->value <= 0 && s->queue != NULL){
		tasksSem--;
		acorda_task = s->queue;	// primeira na fila do semáforo
		acorda_task = (task_t*)queue_remove((queue_t**)&s->queue,(queue_t*)acorda_task); //remove a tarefa da fila de suspensas
		queue_append((queue_t**)&ReadyQueue,(queue_t*)acorda_task); //insere a tarefa no final da fila de prontas
		acorda_task->status = Ready; //status de task volta para Ready
	}
	return(0);
}

// destroi o semáforo, liberando as tarefas bloqueadas
int sem_destroy (semaphore_t *s){
	task_t* acorda_task;
	if(s->status != 1){
		return(-1);		//semáforo inexistente ou destruído
	}
	s->status = 0;
	while(s->queue != NULL){
		tasksSem--;
		acorda_task = s->queue;	// primeira na fila do semáforo
		acorda_task = (task_t*)queue_remove((queue_t**)&s->queue,(queue_t*)acorda_task); //remove a tarefa da fila de suspensas
		queue_append((queue_t**)&ReadyQueue,(queue_t*)acorda_task); //insere a tarefa no final da fila de prontas
		acorda_task->status = Ready; //status de task volta para Ready
	}
	s = NULL;
	return(0);
}

// Inicializa uma barreira
int barrier_create (barrier_t *b, int N){
	if(b->status == 1){ //barreira não existe
		return(-1);
	}

	b->status = 1; //mostra que a barreira foi criada 
	b->max_tasks = N; //numero máximo de tasks da barreira
	b->current_tasks = 0; //contador de tasks na fila da barreira
	b->queue = NULL; //fila da barreira

	return(0);
}

// Chega a uma barreira
int barrier_join (barrier_t *b){
	if (b->status != 1){ //barreira não existe
        return(-1);
    }

    if(b->current_tasks < b->max_tasks - 1){ //se ainda tem espaço na barreira
    	TaskCurrent->status = Suspended; //muda o status da task
        queue_append((queue_t**)&(b->queue),(queue_t*)TaskCurrent); //coloca a task na fila da barreira
        b->current_tasks++; //incrementa o contador de tasks da barreira
        tasksBar++; //incrementa o contador global de tasks na fila da barreira

        task_switch(&Dispatcher); //volta para o dispatcher
        return(0);
    }

    task_t *elem = b->queue; //pega a fila da barreira
    while(b->current_tasks > 0){ //percorre a fila da barreira
        if(elem == &MainTask){ //para garantir que a Main esta no fim da fila (não faz diferença, mas fiz isso pra ficar igual o txt)
        	elem = elem->next; //se for a Main pegamos a proxima tarefa da fila
        	while(b->current_tasks > 1){ //vamos colocar todas as tarefas na fila até só sobrar a Main
	        	task_t *aux = (task_t*)queue_remove((queue_t**)&elem,(queue_t*)elem); //remove a task da fila da barreira
		        aux->status = Ready; //muda o status da task
		        b->current_tasks--; //decrementa o contador de tasks da barreira
		        tasksBar--; //decrementa o contador global de tasks na fila da barreira

		        queue_append((queue_t**)&ReadyQueue,(queue_t*)aux); //coloca a task de volta na fila de prontas
		    }
		    task_t *aux = (task_t*)queue_remove((queue_t**)&elem,(queue_t*)&MainTask); //remove a Main da fila, já que garantimos que era a unica task na fila
	        aux->status = Ready; //muda o status da task
	        b->current_tasks--; //decrementa o contador de tasks da barreira
	        tasksBar--; //decrementa o contador global de tasks na fila da barreira

	        queue_append((queue_t**)&ReadyQueue,(queue_t*)aux); //coloca a task de volta na fila de prontas
        }
    }
    return(0);
}

// Destrói uma barreira
int barrier_destroy (barrier_t *b){
	if (b->status != 1){ //barreira não existe
    	return -1;
  	}

 	b->status = 0; //mostra que a barreira foi destruida
  	task_t *elem = b->queue; //pega a fila da berreira
    while(b->current_tasks > 0){ //percorre a fila da barreira
        task_t *aux = (task_t*)queue_remove((queue_t**)&elem,(queue_t*)elem); //remove a task da fila da barreira
        aux->status = Ready; //muda o status da task
        b->current_tasks--; //decrementa o contador de tasks da barreira
        tasksBar--; //decrementa o contador global de tasks na fila da barreira

        queue_append((queue_t**)&ReadyQueue,(queue_t*)aux); //coloca a task de volta na fila de prontas
    }

	b = NULL; //destroi a barreira
	return(0);
}