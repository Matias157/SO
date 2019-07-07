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
	int parent_id;
	int parent_excd;
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif
