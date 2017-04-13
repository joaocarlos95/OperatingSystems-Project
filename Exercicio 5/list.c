// ||#################################################||			
// ||# 												 #||			
// ||#		  	Instituto Superior Técnico			 #||			
// ||# 												 #||			
// ||#################################################||			
// ||#												 #||			
// ||#				|	João Freitas - 81950		 #||			
// ||# 	- Grupo 21 -|	Hugo Gaspar - 81977			 #||			
// ||#  			| 	João Carlos - 82528 		 #||			
// ||#												 #||			
// ||#################################################||			
																	
																	
// |----------|														
// | Includes |														
// |----------|----------------------------------------------------|

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "list.h"


// |---------------------|												
// | Estruturas de dados |												
// |---------------------|-----------------------------------------|

list_t* lst_new() {
	
   list_t *list;
   list = (list_t*) malloc(sizeof(list_t));
   list->first = NULL;
   return list;
}

// |---------|												
// | Funções |												
// |---------|-----------------------------------------------------|

void lst_destroy(list_t *list) {

	struct lst_iitem *item, *nextitem;
	item = list->first;

	while (item != NULL) {
		nextitem = item->next;
		free(item);
		item = nextitem;
	}
	free(list);
}

void insert_new_process(list_t *list, int pid, time_t starttime) {
	
	lst_iitem_t *item;

	item = (lst_iitem_t *) malloc (sizeof(lst_iitem_t));
	item->pid = pid;
	item->valor = 0;
	item->starttime = starttime;
	item->endtime = 0;
	item->next = list->first;
	list->first = item;
}

list_t *update_terminated_process(list_t *list, int pid, time_t endtime, int valor) {

	lst_iitem_t *item = encontra_pid(list, pid);
	if (item != NULL) {
   		item->endtime = endtime;
   		item->valor = valor;
	}
	return list;
}

void lst_print(list_t *list) {
	
	lst_iitem_t *item;
	
	// Imprime todos os pids e inteiros devolvidos pelos filhos
	for (item = list->first ; item != NULL ; item = item->next) {
		
		printf("Pid: %d ", item->pid); 
		
		if (WIFEXITED(item->valor))
			printf("valor: %d ", item->valor);
		else {
			printf("terminou sem chamar o exit, valor %d ", item->valor);
		}
		printf("e o seu tempo de execucao: %d\n", (int)(item->endtime - item->starttime));
	}
}

lst_iitem_t *encontra_pid(list_t *list, int pid) {
	
	lst_iitem_t *item;
	
	for (item = list->first; item != NULL; item = item->next) {
		if (pid == item->pid) {
			return item;
		}
	}
	return NULL;
}

/* Funcao que vai criar um pid (adicionar o novo valor) */
Item criaPid(int valor) {

	Item newPid = (Item) malloc(sizeof(struct Pides));

	newPid -> valor = valor;
	return newPid;
}

/* Funcao que cria o novo pid a lista , colocando um apontador */
/* para a estrutura que contem o valor, e um apontador para o proximo */
/* pid na lista. */
Link novoPid(Item item, Link next) {

	Link newPid = (Link) malloc(sizeof(struct Pool));
	
	newPid -> item = item;
	newPid -> next = next;

	return newPid;
}


/* Funcao que adiciona um novo pid a lista , tendo em */
/* atencao se a lista esta vazia ou nao. Caso esteja, a Head vai ser igual a Tail, caso */
/* contrario muda a Tail para o novo pid. */
void inserePid(Item item) {

	if (Head == NULL)
		Tail = (Head = novoPid(item, Head));
	else {
		Tail -> next = novoPid(item,Tail -> next);
		Tail = Tail -> next;
	}
}


/* Funcao que remove um pid da lista de pids por processar, recebendo apenas */
/* o valor do mesmo. Depois actualiza a Head, caso o pid retirado tenha sido */
/* o primeiro, actualiza a Tail, caso o pid retirado tenha sido o ultimo, e coloca */
/* o apontador do pid anterior ao removido a apontar para o pid a seguir ao removido. */
void removePid(int pid) {
	
	Link foundPid, previousPid;

	for (foundPid = Head, previousPid = NULL; foundPid != NULL && foundPid -> item -> valor != pid;
			previousPid = foundPid, foundPid = foundPid -> next);
	
	if(foundPid == NULL) {
		printf("Pid %d does not exist\n", pid);
		return;
	}
	printf("O pid com o valor %d foi removido.\n", pid);

	if(Head == foundPid)
		Head = foundPid -> next;
	else 
		previousPid -> next = foundPid -> next;
	if(foundPid -> next == NULL)
		Tail = previousPid; 
	
	free(foundPid -> item);
	free(foundPid);
}

/* Funcao que procura um pid na lista de pids tendo */
/* como ponto de partida a referencia do mesmo. */
Link procuraPid(Link Head, int pid) {

	Link foundPid;
	
	for (foundPid = Head; foundPid != NULL; foundPid = foundPid -> next) {
		if (pid == foundPid -> item -> valor)
			return foundPid;
	}
	return NULL;
}