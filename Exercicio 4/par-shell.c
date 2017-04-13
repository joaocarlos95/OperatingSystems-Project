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
#include <string.h>													
#include <sys/types.h>												
#include <sys/wait.h>												
#include <pthread.h>												
#include <unistd.h>													
#include "commandlinereader.h"										
#include "list.h"													
																	
																	
// |---------|														
// | Defines |														
// |---------|-----------------------------------------------------|
																	
#define NTAREFAS 1													// Número de tarefas criadas
#define NARGUMENTOS 7												// Número máximo de argumentos
#define MAXPARALELO 4												// Número máximo de processos a correr em paralelo
#define TAMSTRING 60												// Numero máximo de caracteres da string
#define COMANDO_EXIT "exit"											// Comando exit
#define FICHEIRO "log.txt"											// Ficheiro
#define KEY_WORD_1 "iteracao"										// Palavra-chave 1
#define KEY_WORD_2 "pid:"											// Palavra-chave 2
#define KEY_WORD_3 "total"											// Palavra-chave 3
																	
																	
// |-------------------|											
// | Variáveis Globais |											
// |-------------------|-------------------------------------------|
																	
int pid, status, terminou = 0, numProcessos = 0;					
int tempoTotal = 0, iteracao = -1;									
																	
FILE *file;															// Ponteiro para um ficheiro
char str[TAMSTRING], str1[TAMSTRING];								// Declaração das strings
list_t *list;														// Ponteiro para estrutura list_t
lst_iitem_t *item;													// Ponteiro para estrutura lst_iitem_t
time_t starttime, endtime;											// Variáveis dos tempos
																	
pthread_t tid[NTAREFAS];										 	// Identificador da tarefa
pthread_mutex_t mutexsum, mutex;									// Trinco
pthread_cond_t cond_maxpar, cond_num_proc;							// Incialização das Variáveis de condição
																	
																	
// |------------|													
// | Protótipos |													
// |------------|--------------------------------------------------|
																	
void *monitora();													// Protótipo da função monitora
void mutexLock(pthread_mutex_t *mutex); 							// Protótipo da função encarregue pelo Lock do trinco
void mutexUnlock(pthread_mutex_t *mutex); 							// Protótipo da função encarregue pelo Unlock do trinco
void condWait(pthread_cond_t *condition, pthread_mutex_t *mutex);  	// Protótipo da função encarregue por fazer Wait
void condSignal(pthread_cond_t *condition);							// Protótipo da função encarregue por fazer Signal	
void closeFile(FILE *file); 										// Protótipo que fecha o ficheiro
void mutexInit(pthread_mutex_t *mutex);								// Protótipo para inicializar os trincos
void condInit(pthread_cond_t *cond);								// Protótipo para inicializar as variáveis de condição
void mutexDestroy(pthread_mutex_t *mutex);							// Protótipo para destruir os trincos
void condDestroy(pthread_cond_t *cond); 							// Protótipo para destruir as variáveis de condição			
																	
// |------------------|												
// | Função Principal |												
// |------------------|--------------------------------------------|
																	
int main(int argc, char **argv) {									

	int i = 0;														
																	
	if ((file = fopen(FICHEIRO, "a+")) == NULL) {					// Abre o ficheiro 'log.txt' para leitura e concatenação do mesmo
		perror("");													// Erro na abertura do ficheiro 'log.txt'
		exit(EXIT_FAILURE);											
	}																
																	
	while (fgets(str, sizeof str, file) != NULL) {					// Percorre o ficheiro e verifica se o mesmo está corrompido
																	
		sscanf(str, "%s", str1);									// Guarda em str1 a primeira palavra de str (linha)
																	
		if (i%3 == 0 && strcmp(str1, KEY_WORD_1) == 0) {			
			i++;													
			sscanf(str, "%s %d", str1, &iteracao);					// Guarda o valor da última iteração
		}															
		else if (i%3 == 1 && strcmp(str1, KEY_WORD_2) == 0) { i++; }
		else if (i%3 == 2 && strcmp(str1, KEY_WORD_3) == 0) { i++; }
	}																

	if (i%3 !=0) {													// Caso a última/duas últimas linhas não existam
		printf("Ficheiro corrompido por ausência de linhas.\n");	
		closeFile(file);											// Fecha o ficheiro
	}																
																	
	if (str != NULL) {												
		sscanf(str, "%s %s %s %d", str1, str1, str1, &tempoTotal);	// Guarda o valor do tempo total da última iteração
	}																
																	
	list = lst_new();												// Nova lista onde vão ser guardados os elementos
	char** argVector = (char**) malloc(NARGUMENTOS*sizeof(char*));	// Alocação de memória para o vetor que guarda os elementos introduzidos
																	
	mutexInit(&mutex);												// Inicialização dos trincos
	mutexInit(&mutexsum);											// Inicialização dos trincos
	condInit(&cond_maxpar);											// Inicialização das variáveis de condição
	condInit(&cond_num_proc);										// Inicialização das variáveis de condição
																	
	if (pthread_create(&tid[0], NULL, monitora, NULL) != 0) {		// Criação da Tarefa Monitora
		perror("");													// Erro na criação da Tarefa Monitora
		exit(EXIT_FAILURE);											
	}																
																	
	while(1) {														

		if (readLineArguments(argVector, NARGUMENTOS) > 0) {   		// Verifica se foram introduzidos argumentos

			if (strcmp(argVector[0], COMANDO_EXIT) == 0) {			// Comando exit introduzido
				terminou = 1;										// Flag que indica à tarefa monitora que foi introduzido o comando exit
				
				mutexLock(&mutex);									// Início da secção crítica da variável de condição
				condSignal(&cond_num_proc);							// Signal à variável de condição que o exit foi introduzido
				mutexUnlock(&mutex);								// Final da secção crítica da variável de condição												
				
				if (pthread_join(tid[0], NULL)) {					// Suspende a tarefa principal até a tarefa monitora terminar
					perror("");										
					exit(EXIT_FAILURE);								
				}													
				closeFile(file);									// Fecha o ficheiro												
				
				lst_print(list);									// Imprime o pid, o estado e o tempo de execução de cada processo
				lst_destroy(list);									// Liberta a memória alocada pela lista
				free(argVector[0]);									// Liberta a memória do primeiro argumento do vetor
				free(argVector);									// Liberta a memória alocada pelo vetor dos argumentos
				mutexDestroy(&mutexsum);							// Elimina os trincos
				mutexDestroy(&mutex);								// Elimina os trincos
				condDestroy(&cond_maxpar);							// Elimina as variáveis de condição
				condDestroy(&cond_num_proc);						// Elimina as variáveis de condição
				exit(EXIT_SUCCESS);									
			}														
																	
			else {													
																	
				mutexLock(&mutex);									// Início da secção crítica					
																	
				while(numProcessos >= MAXPARALELO) {				// Espera que possa ser criado um novo processo em paralelo
					condWait(&cond_maxpar, &mutex);				
				}													
																	
				pid = fork();										// Cria um processo filho
				starttime = time(NULL);								// Tempo inicial do processo
																	
				if (pid < 0) {										// Processo mal criado
					perror("");										
					free(argVector);								// Libertar a memória alocada pelo vetor dos argumentos
				}													
				else if (pid == 0) {								// Processo bem criado 
					execv(argVector[0], argVector);					// Substitui o programa e espaço de endereçamento do processo
																	
					perror("");										// Não executou o exec mas o filho ficou com os dados do pai, sendo
					free(argVector); 								// necessário fazer free dos mesmos
					closeFile(file);								// Caso o exec não tenha sido bem sucedido, fecha-se o ficheiro e
					lst_destroy(list);								// liberta-se a memória alocada pela lista
					exit(EXIT_FAILURE);								
				}													
				else {												
																	
					mutexLock(&mutexsum);							// Início da secção crítica
					insert_new_process(list, pid, starttime);		// Insere o pid e o starttime na lista
					numProcessos++;									// Incrementa o número de processos
					mutexUnlock(&mutexsum);							// Final da secção crítica
																	
					condSignal(&cond_num_proc);						// Signal para a variável de condição que um novo processo foi criado
					mutexUnlock(&mutex);							// Final da secção crítica
				}													
			}														
			free(argVector[0]);										// Liberta a memória do primeiro argumento do vetor
		}															
		else { printf("Não inseriu argumentos.\n"); }				
	}																
	return 0;															
}																	
																	
void *monitora() {													
																	
	while(1) {														

		mutexLock(&mutexsum);										// Início da secção crítica
		
		if (terminou == 0) {										// Verifica se o comando exit foi inserido
			if(numProcessos > 0) {									// Verifica se há processos pendentes
				mutexUnlock(&mutexsum);			 					// Final da secção crítica

				while(numProcessos > 0) {							
																	
					pid = wait(&status);							// Epera pela terminação do processo filho
					mutexLock(&mutex);								// Início da secção crítica
					condSignal(&cond_maxpar);						// Signal à tarefa principal que um processo foi terminado
					endtime = time(NULL);							// Eempo final de execução do processo
																	
					if (pid > 0) {									
						if (WIFEXITED(status) == 1) {				// Macro que diz se filho terminou normalmente
																												
							list = update_terminated_process(list, pid, endtime, WEXITSTATUS(status)); 			// Actualiza os dados da lista
							item = encontra_pid(list, pid);														// Item que contem o pid na lista
							tempoTotal = tempoTotal + (int)(endtime - item->starttime);							// Actualiza o tempo total

							fprintf(file, "iteracao %d\n pid: %d execution time: %d s\n total execution time: %d s\n",
							++iteracao, pid, (int)(endtime - item->starttime), tempoTotal);						
							if (fflush(file)) {																	// Limpa o buffer
								perror("");																		
								exit(EXIT_FAILURE);					
							}										
						}														
					}												
					else {											
						perror("");									
						exit(EXIT_FAILURE);							
					}												
																	
					mutexLock(&mutexsum);							// Início da secção crítica
					numProcessos--;									// Decrementa o número de processos
					mutexUnlock(&mutexsum);							// Final da secção crítica
																						
					while (numProcessos == 0 && terminou == 0) {	// Espera enquanto não forem criados processos ou que
						condWait(&cond_num_proc, &mutex);			// o comando exit não seja inserido
					}												
					mutexUnlock(&mutex);							// Final da secção crítica
				}													
			}														
			else {
				mutexUnlock(&mutexsum);			 					// Final da secção crítica											
				mutexLock(&mutex);									// Início da secção crítica													
				while (numProcessos == 0 && terminou == 0) { 		// Espera enquanto não forem criados processos ou que
					condWait(&cond_num_proc, &mutex);				// o comando exit não seja inserido
				}													
				mutexUnlock(&mutex);								// Final da secção crítica
			}														
		}															
		else {
			mutexUnlock(&mutexsum);			 						// Final da secção crítica											
			pthread_exit((void*) 0);								// Termina a tarefa monitora
		}							
	}																
}																	
																	
void mutexLock(pthread_mutex_t *mutex) {							
  if (pthread_mutex_lock(mutex)) {									
    perror("Error locking mutex");									// Erro ao fazer lock do trinco
    exit(EXIT_FAILURE);												
  }																	
}																	
																	
void mutexUnlock(pthread_mutex_t *mutex) {							
  if (pthread_mutex_unlock(mutex)) {								
    perror("Error unlocking mutex");								// Erro ao fazer unlock do trinco
    exit(EXIT_FAILURE);												
  }																	
}																	
																	
void condWait(pthread_cond_t *condition, pthread_mutex_t *mutex) {	
  	if (pthread_cond_wait(condition, mutex)) {						
    	perror("Error waiting on condition");						// Erro ao fazer wait da variável de condição
    	exit(EXIT_FAILURE);												
 	}																	
}																	
																	
void condSignal(pthread_cond_t *condition) {						
  if (pthread_cond_signal(condition)) {								
    perror("Error signaling on condition");							// Erro ao fazer signal da variável de condição
    exit(EXIT_FAILURE);												
  }																	
}																	
																	
void closeFile(FILE *file) {										
	if (fclose(file)) {												// Fecha o ficheiro
		perror("");													// Erro ao fechar o ficheiro
		exit(EXIT_FAILURE);											
	}																
}																	
																	
void mutexInit(pthread_mutex_t *mutex) {								
	if (pthread_mutex_init(mutex, NULL)) {							// Inicialização dos trincos
    	perror("Error initializing mutex");							// Erro na inicialização dos trincos
    	exit(EXIT_FAILURE);
 	}
}

void condInit(pthread_cond_t *cond) {								
	if (pthread_cond_init(cond, NULL) && numProcessos > 0) {		// Inicialização das variáveis de condição
		perror("Error initializing condition");						// Erro na inicialização das variáveis de condição
		exit(EXIT_FAILURE);
	}
}

void mutexDestroy(pthread_mutex_t *mutex) {							
	if (pthread_mutex_destroy(mutex) && numProcessos > 0) {			// Destrói os trincos
		perror("Error destroying mutex");							// Erro na destruição dos trincos
		exit(EXIT_FAILURE);
	}
}

void condDestroy(pthread_cond_t *cond) {	
	if(pthread_cond_destroy(cond)) {								// Destrói as variáveis de condição
    	perror("Error destroying condition");						// Erro na destruição das variáveis de condição
    	exit(EXIT_FAILURE);
  	}
}