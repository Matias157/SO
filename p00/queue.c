#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

void print_elem (void *ptr){
	
}

void queue_append (queue_t **queue, queue_t *elem){
	queue_t *aux;
	if(queue != NULL){ // verifica se fila existe
		if(elem != NULL){	// verifica se o elemento existe
			if(elem->next != NULL && elem->prev != NULL)	// verifica se o elemento não está em outra fila
			{
				if(*queue == NULL){	// verifica se a fila está vazia
					*queue = elem;
					elem->next = elem;
					elem->prev = elem;
				}
				else{
					aux = (*queue)->prev;
					aux->next = elem;
					elem->next = *queue;
					elem->prev = aux;
					(*queue)->prev = elem;
				}
			}
			else{
				printf("O elemento já existe em outra fila\n");
			}
		}
		else{
			printf("Elemento não existe\n");
		}
	}
	else{
		printf("Está fila não existe!\n");
	}
}

int queue_size (queue_t *queue){
	int cont = 0;
	queue_t *aux = queue->next;
	while(aux != queue){
		cont++;
		aux = aux->next;
	}
	return cont;
}