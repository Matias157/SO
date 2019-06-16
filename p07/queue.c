#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

void print_elem (void *ptr) ;

void queue_append (queue_t **queue, queue_t *elem){
	queue_t *aux;
	if(queue != NULL){ // verifica se fila existe
		if(elem != NULL){ // verifica se o elemento existe
			if(elem->next == NULL && elem->prev == NULL) // verifica se o elemento não está em outra fila
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
				printf("O elemento já existe em outra fila");
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

queue_t *queue_remove (queue_t **queue, queue_t *elem){
	queue_t *cont;
	queue_t *aux;
	queue_t *aux2;
	if(queue != NULL){ // verifica se fila existe
		if(elem != NULL){ // verifica se o elemento existe
			if(elem->next == NULL && elem->prev == NULL){ // verifica se o elemento está em alguma fila
				printf("Elemento não pertence a nenhuma fila\n");
				return(NULL);
			}
			if(*queue != elem){ // verfifoca se o elemento não é o inicio da lista
				cont = elem->next;
				while(cont != *queue){ // procura o inicio da lista
					cont = cont->next;
					if(cont == elem){ // se voltar para o próprio elemento é porque não foi possivel achar o inicio da lista, logo o elemento não pertence a essa lista
						printf("Elemento não pertence a fila\n");
						return(NULL);
					} 
				}
			}
			if(*queue == NULL){	// verifica se a fila está vazia
				printf("A fila está vazia\n");
				return(NULL);
			}
			else{
				if(*queue == elem){ // verifica se o elemento a ser removido é o primeiro da lista
					if((*queue)->next == *queue && (*queue)->prev == *queue){ // verifica se lista tem apenas 1 elemento
						aux = *queue;
						(*queue)->next = NULL;
						(*queue)->prev = NULL;
						*queue = NULL;
						return(aux); // precisamos retornar aux para não perdermos referencia do ponteiro de *queue
					}
					else{
						aux = *queue;
						*queue = aux->next;
						(*queue)->prev = aux->prev;
						aux->prev->next = *queue;
						aux->prev = NULL;
						aux->next = NULL;
						return(aux);
					}

				}
				else if((*queue)->prev == elem){ // verifica se o elemento a ser removio é o último da lista
					aux = *queue;
					while(aux->next->next != *queue){
						aux = aux->next;
					}
					aux2 = aux->next;
					aux->next = *queue;
					(*queue)->prev = aux;
					aux2->prev = NULL;
					aux2->next = NULL;
					return(aux2);
				}
				else{ // entra aqui se o elemento a ser removido estiver no meio da lista
					aux = elem->prev;
					aux2 = aux->next;
					aux->next = aux2->next;
					aux2->next->prev = aux;
					aux2->prev = NULL;
					aux2->next = NULL;
					return(aux2);
				}
			}
		}
		else{
			printf("Elemento não existe\n");
			return(NULL);
		}
	}
	else{
		printf("Está fila não existe!\n");
		return(NULL);
	}
}

int queue_size (queue_t *queue){
	int cont = 1;
	if(queue == NULL){
		return 0;
	}
	else{
		queue_t *aux = queue->next;
		while(aux != queue){
			cont++;
			aux = aux->next;
		}
		return cont;
	}
}

void queue_print (char *name, queue_t *queue, void print_elem (void*)){
	printf("%s: [", name);
	if(queue == NULL){ // verifica se a fila é vazia
		printf("]\n");
	}
	else{
		queue_t *aux;
		aux = queue;
		print_elem(aux);
		printf(" ");
		aux = queue->next;
		while(aux != queue){
			print_elem(aux);
			if(aux->next != queue){ // verifica se é o último elemento a printar
				printf(" ");
			}
			aux = aux->next;
		}
		printf("]\n");
	}
}