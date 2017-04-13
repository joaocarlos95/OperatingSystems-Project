#include <stdio.h>
#include <stdlib.h>
#include "commandlinereader.h"
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "list.h"
#include <pthread.h>

#define N 1				//numero tarefas criadas
#define NARGS 7			//número máximo de argumentos

int pid, status, err, executtime, terminou = 0, numProcessos = 0;	//variáveis globais 
list_t *list;														//ponteiro para estrutura list_t 
lst_iitem_t *item;													//ponteiro para estrutura lst_iitem_t
time_t starttime, endtime;											//variáveis do tempos
pthread_t tid[N];													//identificador da tarefa
pthread_mutex_t mutexsum;											//trinco

void *monitora();													//protótipo da função

int main (int argc, char **argv) {
	
	list = lst_new();												//nova lista onde vão ser guardados os elementos
	char** argVector = (char**) malloc (NARGS*sizeof(char*));		//alocação de memória para o vetor que guarda
																	//os argumentos introduzidos
	pthread_mutex_init(&mutexsum, NULL);							//inicializa o trinco

	err = pthread_create(&tid[0], NULL, monitora, NULL);			//criação da tarefa monitora

//Porque é que o &tid[0] tem como argumento 0?

	if (err!=0){								 					//erro na criação da tarefa
		perror("");
	}
	
	while(1) {


		if (readLineArguments(argVector, NARGS) > 0) {   			//verifica se foram introduzidos argumentos

			if (strcmp(argVector[0], "exit") == 0) {				//instruções para quando for introduzido o exit
				
				terminou = 1;										//flag que indica à tarefa monitora que foi introduzido o comando exit
				err = pthread_join(tid[0], NULL);					//suspende a tarefa principal até a monitora terminar

				if (err!=0){										//erro na suspensão
					perror("");
				}

				lst_print(list);									//imprime o pid,estado e o tempo de execução de cada processo
				lst_destroy(list);									//libertar a memória alocada pela lista
				free(argVector[0]);									//libertar memória do primeiro argumento do vetor
				free(argVector);									//libertar a memória alocada pelo vetor dos argumentos
				pthread_mutex_destroy (&mutexsum);					//elimina o trinco
				exit(EXIT_SUCCESS);
			}
			
			else {

				pid = fork();										//cria processo filho
				starttime = time(NULL);								//tempo inicial do processo
				if (pid < 0) {										//processo mal criado
					perror("");		
					free(argVector);								//libertar a memória alocada pelo vetor dos argumentos
				}
				else if (pid == 0) {								//processo bem criado 
					execv(argVector[0], argVector);					//substitui o programa e espaço de endereçamento 
					perror("");										//do processo onde é invocado
					free(argVector); 								//não executou o exec mas o filho ficou com os dados
																	//do pai, sendo preciso fazer free dos mesmos
					lst_destroy(list);								//libertar a memória alocada pela lista
					exit(EXIT_FAILURE);								
				}													
				else {
					// inicio da seccao critica
					pthread_mutex_lock (&mutexsum);					
					insert_new_process(list, pid, starttime);		//insere o pid e o starttime na lista
					numProcessos++;									
					pthread_mutex_unlock (&mutexsum);
					//final da seccao critica
				}
			}
			free(argVector[0]);										//libertar memória do primeiro argumento do vetor
		}
		else {
			printf("Não inseriu argumentos.\n");
		}
	}

return 0;
}

void *monitora() {
	
	while(1) {
		
		if (terminou == 0) {										//verifica se a flag não foi ativada
			if (numProcessos > 0) {
				while (numProcessos > 0) {
					
					pid = wait(&status);							//espera pela terminação do processo filho
					endtime = time(NULL);							//tempo final de execução do processo
					item = encontra_pid(list, pid);					//procura o item na lista

					if (pid > 0) {
						if (WIFEXITED(status) == 1) {				//macro que diz se filho terminou normalmente
							// inicio da seccao critica
							pthread_mutex_lock (&mutexsum);
							list = update_terminated_process(list, pid, endtime, WEXITSTATUS(status), item);
							pthread_mutex_unlock (&mutexsum);
							//final da seccao critica
						}
					}
					else {
						perror("Erro no wait.\n");
					}
					//inicio da seccao critica
					pthread_mutex_lock (&mutexsum);
					numProcessos--;
					pthread_mutex_unlock (&mutexsum);
					//final da seccao critica
				}
			}
			else {
					sleep(1);										//nao ha processos, suspende a tarefa monitora 1 segundo
			}
		}
		else {
			pthread_exit((void*) 0);								//termina a tarefa monitora
		}
	}	
}