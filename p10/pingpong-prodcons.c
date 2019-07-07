#include <stdio.h>
#include <stdlib.h>
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

void produtor()
{
  while(1){
    task_sleep (1);
    int num_item = random()%100;
    sem_down(&s_vaga);
    sem_down(&s_buffer);
    item_t* new_item;
    new_item->id = num_item;
    queue_append((queue_t**)&(itens),(queue_t*)(new_item));
    printf("p%d\n produziu %d", (char*)arg , num_item);
    sem_up(s_buffer);
    sem_up(s_item);
  }
}

void consumidor(){
  while (1){
    sem_down(s_item);
    sem_down(s_buffer);
    item_t* item = queue_remove((queue_t**)&(itens),(queue_t*)(itens));
    sem_up(s_buffer);
    sem_up(s_vaga);
    printf("p%d\n consumiu %d", (char*)arg , item->id);
    task_sleep (1);
  }
}

int main (int argc, char *argv[])
{

  printf ("Main INICIO\n");
  pingpong_init ();
  sem_create (&s_buffer, 5);
  sem_create (&s_vaga, 1);
  sem_create (&s_item, 0);
  
  task_create (&consumidores[0], consumidor, "c1");
  task_create (&consumidores[1], consumidor, "c2");
   
  task_create (&produtores[0], produtor, "p1");
  task_create (&produtores[1], produtor, "p2");
  task_create (&produtores[2], produtor, "p3");

  for(int i = 0; i < NUM_CONSUMIDORES; i++){
    task_join (&consumidores[i]);
  }

  for(int i = 0; i < NUM_PRODUTORES; i++){
    task_join (&produtores[i]);
  }

  sem_destroy(&s_buffer);
  sem_destroy(&s_vaga);
  sem_destroy(&s_item);

  printf ("Main FIM\n") ;
  task_exit (0) ;

  exit (0) ;
}
