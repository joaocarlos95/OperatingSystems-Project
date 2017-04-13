// ||#################################################||       
// ||#                                               #||        
// ||#         Instituto Superior Técnico            #||        
// ||#                                               #||        
// ||#################################################||       
// ||#                                               #||        
// ||#               |  João Freitas - 81950         #||        
// ||#   - Grupo 21 -|  Hugo Gaspar - 81977          #||        
// ||#               |  João Carlos - 82528          #||        
// ||#                                               #||        
// ||#################################################||       

                                                   
// |----------|                                          
// | Includes |                                          
// |----------|----------------------------------------------------|

#include <stdlib.h>
#include <stdio.h>
#include <time.h>


// |------------|                                          
// | Estruturas |                                          
// |------------|----------------------------------------------------|

/* lst_iitem - each element of the list points to the next element */
typedef struct lst_iitem {
   int pid;
   time_t starttime;
   time_t endtime;
   int valor;
   struct lst_iitem *next;
} lst_iitem_t;

/* list_t */
typedef struct {
   lst_iitem_t * first;
} list_t;

/* Estrutura para guardar o valor dos pids*/
typedef struct Pides {
	int valor;
	} *Item;

typedef struct Pool *Link;

/* Cada elemento da lista aponta para o próximo elemento */
struct Pool {
   Item item;
   Link next;
   };


Link Head;
Link Tail;


// |------------|                                     
// | Protótipos |                                     
// |------------|--------------------------------------------------|

/* lst_new - allocates memory for list_t and initializes it */
list_t* lst_new();

/* lst_destroy - free memory of list_t and all its items */
void lst_destroy(list_t *);

/* insert_new_process - insert a new item with process id and its start time in list 'list' */
void insert_new_process(list_t *list, int pid, time_t starttime);

/* update_terminated_process - updates the item 'item' with valor, end time and execution time in list 'list' */
list_t * update_terminated_process(list_t *list, int pid, time_t endtime, int valor);

/* lst_print - print the content of list 'list' to standard output */
void lst_print(list_t *list);

/* encontra_pid - searches in the list 'list' the item with the process id 'pid' and returns that item */ 
lst_iitem_t *encontra_pid(list_t *list, int pid);

/* Função que vai criar um pid */
Item criaPid(int valor);

/* Função que aloca memória e cria um apontador para o próximo elemento */
Link novoPid(Item item, Link next);

/* Função que adiciona um novo pid à lista */
void inserePid(Item item);

/* Função que remove um pid da lista de pids */
void removePid(int pid);

/* Função que procura um pid na lista de pids*/
Link procuraPid(Link Head, int pid);