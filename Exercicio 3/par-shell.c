#include <stdio.h>
#include <stdlib.h>
#include "commandlinereader.h"
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "list.h"
#include <pthread.h>
#include <semaphore.h>

#define N 1				//numero tarefas criadas
#define NARGS 7			//número máximo de argumentos
#define MAXPAR 4		//numero maximo de processos a correr em paralelo

//variáveis globais 
int pid, status, err, terminou = 0, numProcessos = 0;				//'err' -> verifica criacao de tarefa, 0 -> tarefa criada com sucesso
																	//'terminou' -> flag do comando exit, 0 -> exit nao introduzido, 1 -> exit introduzido 
list_t *list;														//ponteiro para estrutura list_t 'list'
lst_iitem_t *item;													//ponteiro para estrutura lst_iitem_t 'item'
time_t starttime, endtime;											//variáveis do tempos
pthread_t tid[N];												 	//identificador da tarefa
pthread_mutex_t mutexsum;											//trinco
sem_t sem_maxpar, sem_num_proc;										//semaforos 'sem_maxpar' -> limite maximo de processos a correr em paralelo
																	//'sem_num_proc' -> tarefa monitora espera caso nao existam processos a serem executados 
																	//e o comando exit nao foi introduzido

void *monitora();													//protótipo da função

int main (int argc, char **argv) {
	
	list = lst_new();												//nova lista onde vão ser guardados os elementos
	char** argVector = (char**) malloc (NARGS*sizeof(char*));		//alocação de memória para o vetor que guarda
																	//os argumentos introduzidos
	sem_init(&sem_maxpar, 0, MAXPAR);								//inicializa semaforo com o valor MAXPAR
	pthread_mutex_init(&mutexsum, NULL);							//inicializa o trinco

	err = pthread_create(&tid[0], NULL, monitora, NULL);			//criação da tarefa monitora
	if (err!=0){													//erro na criação da tarefa
		perror("");
	}
	
	while(1) {


		if (readLineArguments(argVector, NARGS) > 0) {   			//verifica se foram introduzidos argumentos

			if (strcmp(argVector[0], "exit") == 0) {				//instruções para quando for introduzido o exit
				
				terminou = 1;										//flag que indica à tarefa monitora que foi introduzido o comando exit
				sem_post(&sem_num_proc);							//assinala o semaforo, comando exit foi introduzido
				err = pthread_join(tid[0], NULL);					//suspende a tarefa principal até a monitora terminar

				if (err!=0){										//erro na suspensão
					perror("");
				}

				lst_print(list);									//imprime o pid,estado e o tempo de execução de cada processo
				lst_destroy(list);									//libertar a memória alocada pela lista
				free(argVector[0]);									//libertar memória do primeiro argumento do vetor
				free(argVector);									//libertar a memória alocada pelo vetor dos argumentos
				pthread_mutex_destroy (&mutexsum);					//elimina o trinco
				sem_destroy(&sem_maxpar);							//elimina o semaforo 'sem_maxpar'
				sem_destroy(&sem_num_proc);							//elimina o semaforo 'sem_num_proc'

				exit(EXIT_SUCCESS);
			}
			
			else {

				sem_wait(&sem_maxpar);								//espera que possa ser criado um novo processo em paralelo 
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
					sem_post(&sem_num_proc);						//assinala o semaforo que um novo processo foi criado
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
		
		if (terminou == 0) {											//verifica se a flag não foi ativada
			if (numProcessos > 0) {
				while(numProcessos > 0) {
					
					pid = wait(&status);							//espera pela terminação do processo filho
					sem_post(&sem_maxpar);							//assinala semaforo que um processo foi terminado
					endtime = time(NULL);							//tempo final de execução do processo
					
					if (pid > 0) {
						if (WIFEXITED(status) == 1) {				//macro que diz se filho terminou normalmente
							// inicio da seccao critica
							pthread_mutex_lock (&mutexsum);
							list = update_terminated_process(list, pid, endtime, WEXITSTATUS(status)); 
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
					sem_wait(&sem_num_proc);						//nao ha processos, espera que hajam processos ou do comando exit 
				}													//atraves do semaforo partilhado com a tarefa principal
			}
			else {
				sem_wait(&sem_num_proc);							//nao ha processos, espera que hajam processos ou do comando exit 
			}														//atraves do semaforo partilhado com a tarefa principal
		}
		else {
			pthread_exit((void*) 0);								//termina a tarefa monitora
		}
	}	
}