/*
QUANDO A FUNCAO SLEEP E CHAMADA, A TAREFA ATUAL ENTRA NA FILA DE TAREFAS
ADORMECIDAS PELO TEMPO INDICADO NA FUNCAO

A CADA VEZ QUE A FUNCAO task_yield_temp FOR CHAMADA, O CONTADOR DE CADA TASK
ADORMECIDA VAI SER DECREMENTADO. QUANDO ESTE CONTADOR CHEGAR EM ZERO, A TAREFA
SERA ACORDADA E SERA TRANSFERIDA PARA A FILA DE TAREFAS PRONTAS. A VERIFICACAO
DAS TASKS PARA SABER QUAL SERA ACORDADA SERA FEITA PELA FUNCAO acorda_adormecidas

NO dispatcher_body A TASK ATUAL SO SERA TROCADA CASO TENHA ALGUMA TAREFA NA FILA
DE PRONTAS, CASO CONTRARIO, SE AINDA HOUVER TAREFAS ADORMECIDAS, MAS NENHUMA NA
FILA DE PRONTAS, A TASK ATUAL CONTINUARA NO dispatcher 
*/

#include "pingpong.h"
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#define STACKSIZE 32768
#define N 100
//#define DEBUG 1
// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action ;

// estrutura de inicialização to timer
struct itimerval timer;

task_t t_main, dispatcher, *fila0= NULL,*antigo=NULL,*atual, *next, *fila_espera=NULL, *aux, *adormecidas=NULL;
int cont_id=1;
int userTasks=0;
int tick=19;
int ticks=0; //contador geral de ticks
int valor_task_exit=0;
int tasksAdormecidas=0;
void task_yield (){

    #ifdef DEBUG
        printf ("task_yield: vai sair da tarefa %d e ir para o dispatcher\n", atual->tid) ;
    #endif
    if (atual->tid !=dispatcher.tid){
        task_switch (&dispatcher);
    }

}

void acorda_adormecidas(){
  /*Percorre a fila de tarefas adormecidas, diminui o tempo de todas e verifica quem precisa ser acordada*/
    if (tasksAdormecidas){
        task_t* elem=adormecidas;
        int inicio, acorda=0;
        for (inicio=0; inicio<tasksAdormecidas; inicio++){
            elem->adormecida--;
            elem=elem->next;
            if (elem->adormecida<=0){
                //printf ("tem que acordar a task %d\n", elem->tid);
                acorda++;
            }
        }
        //printf ("tem que acordar %d tasks\n", acorda);
        elem=adormecidas;
        task_t* anterior;
        while(acorda){
          if (elem->adormecida<=0){
              //acorda
              anterior=elem->prev;
              aux=(task_t*)queue_remove((queue_t**)&adormecidas,(queue_t*)elem);
              tasksAdormecidas--;
              queue_append((queue_t**)&fila0,(queue_t*)aux);
              userTasks++;
              aux->status=0;
              acorda--;
              elem=anterior;
              #ifdef DEBUG
                  printf ("acorda_adormecidas: acordando a tarefa %d\n", aux->tid) ;
              #endif
          }
          elem=elem->next;
        }
    }
}

void task_yield_temp (){
    ticks++;
    atual->ntick+=1;
    if (!(ticks%1000))
      printf ("\t %d ms\n", ticks);

    acorda_adormecidas();
    if(tick>0){
        #ifdef DEBUG
            //printf ("task_yield_temp1: tarefa %d diminuiu o quantuum\n", atual->tid) ;
        #endif
        tick--;
    }
    else{
        tick=19;
        #ifdef DEBUG
            printf ("task_yield_temp: tarefa %d terminou o quantuum\n", atual->tid) ;
        #endif
        task_yield();
    }
}

task_t* scheduler(){
    //PRIORIDADE(ENVELHECIMENTO)!
    if (userTasks==0){
        return NULL;
    }
    int i;
    task_t *elem=fila0->next;
    aux=fila0;
    int pri_aux = aux->prio_static-aux->aging;
    int pri_elem;
    for (i=0; i<userTasks; i++){
        pri_elem=elem->prio_static-elem->aging;
        if ((pri_elem<=pri_aux) ){
            aux->aging=(aux->aging)+1;
            aux=elem;
            pri_aux = aux->prio_static-aux->aging;
        }
        else if ((pri_elem>pri_aux) ){
            elem->aging=(elem->aging)+1;
        }

        elem=elem->next;
    }
    #ifdef DEBUG
        printf ("scheduler retornou a task %d com a prioridade %d\n", aux->tid, pri_aux) ;
    #endif
    aux->aging=0;
    return aux;

}

void dispatcher_body (){
    #ifdef DEBUG
        printf ("dispatcher_body chamado\n") ;
    #endif
    while (userTasks || tasksAdormecidas){
        //printf ("userTasks %d || tasksAdormecidas %d\n", userTasks, tasksAdormecidas) ;
        next = scheduler();
        if (next){
            if (setitimer (ITIMER_REAL, &timer, 0) < 0)
            {
              perror ("Erro em setitimer: ") ;
              exit (1) ;
            }
            task_switch(next);
        }

    }

    #ifdef DEBUG
        printf ("saiu do dispatcher_body\n") ;
    #endif
    task_exit(0);
}

void pingpong_init () {

    getcontext(&t_main.context);
    t_main.prev = NULL;
    t_main.next = NULL;
    t_main.tid= 0;
    t_main.status=0;
    t_main.prio_static=0;
    t_main.aging = 0;
    t_main.ntick=0;    //numero de ticks usados pela tarefa
    t_main.tick_total=ticks;   //numero de ticks total do processador
    t_main.ativacoes=1;
    t_main.parent=NULL;
    t_main.exit_parent=0;
    t_main.adormecida=0;
    atual=&t_main;    //queue_append ((queue_t **) &fila0, (queue_t*) atual);
    setvbuf (stdout, 0, _IONBF, 0) ;
    action.sa_handler = task_yield_temp ;
    sigemptyset (&action.sa_mask) ;
    action.sa_flags = 0 ;
    if (sigaction (SIGALRM, &action, 0) < 0)
    {
      perror ("Erro em sigaction: ") ;
      exit (1) ;
    }
   // ajusta valores do temporizador
    timer.it_value.tv_usec = 1000 ;      // primeiro disparo, em micro-segundos
    timer.it_interval.tv_usec = 1000 ;   // disparos subsequentes, em micro-segundos
    task_create(&dispatcher,dispatcher_body,NULL);
    #ifdef DEBUG
        printf ("pingpong_init iniciado: main iniciada com id %d\n", t_main.tid) ;
    #endif
    task_yield();


}

int task_create (task_t *task, void (*start_func)(void *), void *arg) {
    getcontext(&(task->context));
    char *stack ;
    stack = malloc (STACKSIZE) ;
    if (stack){
       task->context.uc_stack.ss_sp = stack ;
       task->context.uc_stack.ss_size = STACKSIZE;
       task->context.uc_stack.ss_flags = 0;
       task->context.uc_link = 0;
    }
    else{
       perror ("Erro na criao da pilha: ");
       exit (1);
    }
    task->tid=cont_id++;
    task->prev=NULL;
    task->next=NULL;
    task->status=0;
    task->prio_static=0;
    task->aging=0;
    task->ntick=0;    //numero de ticks usados pela tarefa
    task->tick_total=ticks;   //numero de ticks total do processador
    task->ativacoes=0;
    task->parent=NULL;
    task->exit_parent=0;
    task->adormecida=0;
    makecontext (&(task->context), (void*)(*start_func), 1, arg);
    queue_append((queue_t**)&fila0,(queue_t*)task);
    userTasks++;

    #ifdef DEBUG
        printf ("task_create: criou tarefa %d\n", task->tid);
    #endif
    //task_yield();
    return task->tid;
}

int task_switch (task_t *task) {
    antigo=atual;
    atual=(task_t*)queue_remove((queue_t**)&fila0,(queue_t*)task);
    userTasks--;
    if (antigo->status==0){   //entra quando a tarefa for voltar para a fila
        queue_append((queue_t**)&fila0,(queue_t*)antigo);
        userTasks++;
        #ifdef DEBUG
            printf ("task_switch: colocou a tarefa %d na fila\n", antigo->tid) ;
        #endif
    }
    else if (antigo->status==1){    //quando a tarefa chegar ao fim
        antigo->tick_total=ticks-antigo->tick_total;
        printf ("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", antigo->tid, antigo->tick_total, antigo->ntick, antigo->ativacoes);

    }
    #ifdef DEBUG
        printf ("task_switch: passou para a tarefa %d\n", atual->tid) ;
    #endif
    atual->ativacoes=atual->ativacoes+1;
    return swapcontext (&(antigo->context), &(atual->context));
}

void task_exit (int exitCode) {
    atual->status=1;
    valor_task_exit=exitCode;
    #ifdef DEBUG
        printf ("task_exit: vai retirar a tarefa %d\n", atual->tid) ;
    #endif
    fila_espera=(atual->parent);
    while(fila_espera){
        //fila_espera=(atual->parent);
        task_resume (atual->parent);
    }

    if(atual->tid != dispatcher.tid){
        task_switch (&dispatcher);
    }
    else {    //quando o dispatcher chegar ao fim
        userTasks--;
        atual->tick_total=ticks-atual->tick_total;
        printf ("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", atual->tid, atual->tick_total, atual->ntick, atual->ativacoes);

    }

}

int task_id () {
    #ifdef DEBUG
        printf ("task_id: retornou valor da task %d\n", atual->tid) ;
    #endif
   return atual->tid;
}

void task_suspend (task_t *task, task_t **queue) {
    if (task == NULL){
        aux = atual;
        aux->status=2;
        queue_append((queue_t**)queue,(queue_t*)aux);
        #ifdef DEBUG
            printf ("task_suspend: suspendendo tarefa %d \n", aux->tid) ;
        #endif
        task_switch(&dispatcher);
    }
    else {
        aux= (task_t*)queue_remove((queue_t**)&fila0,(queue_t*)task);
        aux->status=2;
        queue_append((queue_t**)queue,(queue_t*)aux);
        userTasks--;
        task_switch(&dispatcher);
        #ifdef DEBUG
            printf ("task_suspend: suspendendo tarefa %d \n", aux->tid) ;
        #endif


    }
}

void task_resume (task_t *task){
    aux= (task_t*)queue_remove((queue_t**)&fila_espera,(queue_t*)task);
    if (aux){
        queue_append((queue_t**)&fila0,(queue_t*)aux);
        userTasks++;
        aux->exit_parent=valor_task_exit;
    }

    #ifdef DEBUG
        printf ("task_resume: devolvendo tarefa %d a fila de prontas\nusertask: %d\n", aux->tid, userTasks) ;
    #endif


}

unsigned int systime (){
  #ifdef DEBUG
      printf ("systime: retornou valor da task %d\n", ticks) ;
  #endif

  return (ticks);

}

// a tarefa corrente aguarda o encerramento de outra task
int task_join (task_t *task) {
  /*A tarefa atual e colocada suspensa na fila da tarefa task->parent ate que a mesma finalize*/
  #ifdef DEBUG
      printf ("task_join: entrou com a tarefa %d\n", atual->tid) ;
  #endif
    if(task!=NULL && task->status!=1 ) {

        task_suspend (NULL, &(task->parent));
        #ifdef DEBUG
            printf ("task_join: suspendendo tarefa %d a fila de prontas\n", atual->tid) ;
        #endif
        return valor_task_exit;
    }
    else{
        #ifdef DEBUG
            printf ("task_join: nao achou tarefa\n") ;
        #endif
        return -1;
    }
}

// suspende a tarefa corrente por t segundos
void task_sleep (int t){
    /*passa a tarefa atual para uma fila de tarefas adormecidas e muda o parametro "adormecida" para o tempo em que a tera tera de ficar dormindo*/
    atual->adormecida=1000*t;
    queue_append((queue_t**)&adormecidas,(queue_t*)atual);
    tasksAdormecidas++;
    atual->status=3;
    //printf ("task_sleep: adormecendo a tarefa %d por %d ms \n", atual->tid, atual->adormecida) ;

    #ifdef DEBUG
        printf ("task_sleep: adormecendo a tarefa %d por %d ms \n", atual->tid, atual->adormecida) ;
    #endif
    task_switch (&dispatcher);
}
