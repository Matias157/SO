// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DAINF UTFPR
// Versão 1.0 -- Março de 2015
//
// Estruturas de dados internas do sistema operacional
// Alunos: Alexandre Herrero Matias e Giuliana Martins Silva 

#include <ucontext.h>

#ifndef __DATATYPES__
#define __DATATYPES__

enum status_t{Ready, Running, Suspended, Terminated};

// Estrutura que define uma tarefa
typedef struct task_t
{
	struct task_t *prev, *next;
	int id;
	int exit_code;
	ucontext_t context;
	void *stack;
	struct task_t *parent;
	enum status_t status;
	unsigned int execution_time;
	unsigned int processor_time;
	unsigned int activations;
	int parent_excd;
	int parent_id;
	int wakeup;
} task_t ;

// estrutura que define um semáforo
typedef struct semaphore_t
{
  	task_t* queue;
  	int value;
  	int counter;
  	int status;
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct barrier_t
{
	task_t* queue;
  	int max_tasks;
	int current_tasks;
  	int status;
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{

} mqueue_t ;

typedef struct item_t
{
   struct queue_t *prev ;  // aponta para o elemento anterior na fila
   struct queue_t *next ;  // aponta para o elemento seguinte na fila
   int id;
} item_t ;

#endif
