#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pingpong.h"

// operating system check
#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif

#define NUM_CONSUMIDORES 2
#define NUM_PRODUTORES 3

semaphore_t s_vaga, s_buffer, s_item;
item_t* itens;
task_t consumidores[NUM_CONSUMIDORES], produtores[NUM_PRODUTORES];

void produtor(void* arg)
{
  while(1){
    task_sleep (1);
    int num_item = rand()%100+1;
    sem_down(&s_vaga);      // pede uma vaga
    sem_down(&s_buffer);    // bloqueia o buffers

    item_t* new_item = (item_t*)malloc(sizeof(item_t));
    new_item->id = num_item;
    queue_append((queue_t**)&itens,(queue_t*)new_item);   // insere item no fim da fila
    printf("%s produziu %d\n", (char*)arg , num_item);

    sem_up(&s_buffer);     // libera o buffer
    sem_up(&s_item);       // libera um item
  }
}

void consumidor(void* arg){
  while (1){
    sem_down(&s_item);   // pede para consumir um item
    sem_down(&s_buffer); // bloqueia o buffer

    item_t* item = (item_t*)queue_remove((queue_t**)&(itens),(queue_t*)(itens));  // remove item do começo da fila
    
    sem_up(&s_buffer);   // libera o buffer
    sem_up(&s_vaga);     // libera uma vaga no buffer
    printf("%s consumiu %d\n", (char*)arg , item->id);
    task_sleep (1);
  }
}

int main (int argc, char *argv[])
{

  printf ("Main INICIO\n");

  pingpong_init ();

  sem_create (&s_buffer, 1);  // valor 1 no buffer
  sem_create (&s_vaga, 5);    // inicialmente há 5 vagas
  sem_create (&s_item, 0);    // inicialmente não há item nenhum

  task_create (&produtores[0], produtor, "p1");
  task_create (&produtores[1], produtor, "p2");
  task_create (&produtores[2], produtor, "p3");
  
  task_create (&consumidores[0], consumidor, "                         c1");
  task_create (&consumidores[1], consumidor, "                         c2");

  // um produtor deve esperar pelo outro
  task_join (&produtores[0]);

  sem_destroy(&s_buffer);
  sem_destroy(&s_vaga);
  sem_destroy(&s_item);

  printf ("Main FIM\n") ;
  task_exit (0) ;

  exit (0) ;
}
