/*
 * list.c - implementation of the integer list functions 
 */


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "list.h"



list_t* lst_new()
{
   list_t *list;
   list = (list_t*) malloc(sizeof(list_t));
   list->first = NULL;
   return list;
}


void lst_destroy(list_t *list)
{
	struct lst_iitem *item, *nextitem;

	item = list->first;
	while (item != NULL){
		nextitem = item->next;
		free(item);
		item = nextitem;
	}
	free(list);
}


void insert_new_process(list_t *list, int pid, time_t starttime)
{
	lst_iitem_t *item;

	item = (lst_iitem_t *) malloc (sizeof(lst_iitem_t));
	item->pid = pid;
	item->valor = 0;
	item->starttime = starttime;
	item->endtime = 0;
	item->next = list->first;
	list->first = item;
}

list_t *update_terminated_process(list_t *list, int pid, time_t endtime, int valor)
{

	lst_iitem_t *item = encontra_pid(list, pid);
	if(item != NULL){
   		item->endtime = endtime;
   		item->valor = valor;
	}
	return list;
}


void lst_print(list_t *list)
{
	lst_iitem_t *item;
	for (item = list->first ; item != NULL ; item = item->next) {
		//imprime todos os pids e inteiros devolvidos pelos filhos
		printf("Pid: %d ", item->pid);
		if(WIFEXITED(item->valor))
			printf("valor: %d ", item->valor);
		else{
			printf("terminou sem chamar o exit, valor %d ", item->valor);
		}
		printf("e o seu tempo de execucao: %d\n", (int)(item->endtime - item->starttime));
	}
}

lst_iitem_t *encontra_pid(list_t *list, int pid) {
	lst_iitem_t *item;
	for(item = list->first; item != NULL; item = item->next) {
		if(pid == item->pid) {
			return item;
		}
	}
	return NULL;
}
