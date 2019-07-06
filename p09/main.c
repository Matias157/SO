#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action ;

// estrutura de inicialização to timer
struct itimerval timer;

int ticks = 0;

void timer_handler(){ //tratador do timer
	ticks++; //incrementa o contador global de ticks
	if(ticks%20 == 0){ //se chegar a 20
		printf("%d\n", ticks); //retorna a fila de prontas e chama a próxima tarefa
	}
}


int main(){

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
    while(1);
}