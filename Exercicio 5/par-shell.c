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
#include <fcntl.h>
#include <sys/stat.h>
			

// |---------|														
// | Defines |														
// |---------|-----------------------------------------------------|
																	
#define NTAREFAS 1													// Número de tarefas criadas
#define MAXPARALELO 4												// Número máximo de processos a correr em paralelo
#define NARGUMENTOS 7												// Número máximo de argumentos
#define TAMSTRING 60												// Numero máximo de caracteres da string
#define FD_PERMISSIONS 0666											// Permissões para o ficheiro onde é redireccionado o stdout
#define COMANDO_EXIT "exit"											// Comando exit
#define COMANDO_EXIT_GLOBAL "exit-global"							// Comando exit-global
#define COMANDO_STATS "stats"										// Comando stats
#define KEY_WORD_1 "iteracao"										// Palavra-chave 1
#define KEY_WORD_2 "pid:"											// Palavra-chave 2
#define KEY_WORD_3 "total"											// Palavra-chave 3
#define PARSHELLIN "/tmp/par-shell-in"								// Nome do pipe principal (presente da directoria /temp)
#define FICHEIRO_OUTPUT "log.txt"									// Nome do ficheiro para o qual é enviado o output				

// |-------------------|											
// | Variáveis Globais |											
// |-------------------|-------------------------------------------|
																	
int pid, status, terminou = 0, numProcessos = 0;					// Declaração de vários inteiros		
int tempoTotal = 0, iteracao = -1;
																							
FILE *file;															// Ponteiro para um ficheiro
char str[TAMSTRING], str1[TAMSTRING], string[TAMSTRING];			// Declaração das strings
char string1[TAMSTRING], string4[TAMSTRING], string5[TAMSTRING], string6[TAMSTRING];
list_t *list;														// Ponteiro para estrutura list_t
lst_iitem_t *item;													// Ponteiro para estrutura lst_iitem_t
Link Head, Tail;													// Link para a Head e Tail da lista de pids			
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
void closeFileF(FILE *file); 										// Protótipo que fecha um ficheiro
void mutexInit(pthread_mutex_t *mutex);								// Protótipo para inicializar os trincos
void condInit(pthread_cond_t *cond, int numProcessos);				// Protótipo para inicializar as variáveis de condição
void mutexDestroy(pthread_mutex_t *mutex, int numProcessos);		// Protótipo para destruir os trincos
void condDestroy(pthread_cond_t *cond); 							// Protótipo para destruir as variáveis de condição
void readFileF(char *nome);											// Protótipo para ler o conteúdo do ficheiro "log.txt"							
void atualizaFileF(FILE *file);										// Protótipo para actualizar o conteúdo do ficheiro "log.txt"
void closeFile(int j);												// Protótipo que fecha um ficheiro
void openedFile(int j);												// Protótipo que verifica se o ficheiro foi aberto correctamente
void dupFile(int j);												// Protótipo que verifica se o dup foi feito correctamente
void apanhaCTRLC();													// Protótipo que apanha o signal referente ao Crtl-C
void killPids(Link Head);											// Protótipo que mata todos os processos terminais
void inicializa();													// Protótipo que inicializa a Head e a Tail da lista de pids
		

// |------------------|												
// | Função Principal |												
// |------------------|--------------------------------------------|
																	
int main(int argc, char **argv) {																
	
	signal(SIGINT, apanhaCTRLC);									// Redefinição do signal SIGINT
	
	int ficheiro, ficheiro1, ficheiro2;								// Nomes de ficheiros a abrir
	char** argVector = (char**) malloc(NARGUMENTOS*sizeof(char*));	// Alocação de memória para o vetor que guarda os elementos introduzidos
	char *myfifo = PARSHELLIN;

	mutexInit(&mutex);												// Inicialização dos trincos
	mutexInit(&mutexsum);											// Inicialização dos trincos
	condInit(&cond_maxpar, numProcessos);							// Inicialização das variáveis de condição
	condInit(&cond_num_proc, numProcessos);							// Inicialização das variáveis de condição
	
	list = lst_new();												// Nova lista onde vão ser guardados os elementos
	inicializa();													// Inicializa a Head e a Tail da lista de pids

	readFileF(FICHEIRO_OUTPUT);										// Leitura do ficheiro 
																	
	if (pthread_create(&tid[0], NULL, monitora, NULL) != 0) {		// Criação da Tarefa Monitora
		perror("");													// Erro na criação da Tarefa Monitora
		exit(EXIT_FAILURE);											
	}
		
	unlink(myfifo);													// Remove o pipe caso exista
	
	if (mkfifo(myfifo, FD_PERMISSIONS) == -1) {						// Cria o pipe com o nome da variável "myfifo"
		perror("");													// Erro na criação do pipe
		exit(EXIT_FAILURE);
	}

	ficheiro1 = open(myfifo, O_RDONLY);								// Abre o pipe para leitura com o nome da variável "myfifo" 
	openedFile(ficheiro1);
	closeFile(0);													// Fecha o standard input
	dupFile(ficheiro1);												// Redirecciona o standard input do ficheiro
			
	while(1) {														

		if ((readLineArguments(argVector, NARGUMENTOS)) > 0) {   	// Verifica se foram introduzidos argumentos

			if (strncmp(argVector[0], "listaPids", 9) == 0) {		// Verifica se o primeiro argumento introduzido foi "listaPids"
				inserePid(criaPid(atoi(argVector[1])));				// Insere o pid do terminal na lista de pids 
			}

			else if (strncmp(argVector[0], COMANDO_EXIT_GLOBAL, 11) == 0) {	// Verifica se o comando exit-global foi introduzido

				unlink(myfifo);										// Remove o pipe caso exista
				killPids(Head);										// Faz Kill de todos os pids da lista (removendo-os da mesma)

				terminou = 1;										// Flag que indica à tarefa monitora que foi introduzido o comando exit
				mutexLock(&mutex);									// Início da secção crítica da variável de condição
				condSignal(&cond_num_proc);							// Signal à variável de condição que o exit foi introduzido
				mutexUnlock(&mutex);								// Final da secção crítica da variável de condição												
				
				if (pthread_join(tid[0], NULL)) {					// Suspende a tarefa principal até a tarefa monitora terminar
					perror("");										// Erro na suspensão da tarefa principal					
					exit(EXIT_FAILURE);								
				}				

				closeFileF(file);									// Fecha o ficheiro												
				lst_print(list);									// Imprime o pid, o estado e o tempo de execução de cada processo
				lst_destroy(list);									// Liberta a memória alocada pela lista
				free(argVector[0]);									// Liberta a memória do primeiro argumento do vetor
				free(argVector);									// Liberta a memória alocada pelo vetor dos argumentos
				mutexDestroy(&mutexsum, numProcessos);				// Elimina os trincos
				mutexDestroy(&mutex, numProcessos);					// Elimina os trincos
				condDestroy(&cond_maxpar);							// Elimina as variáveis de condição
				condDestroy(&cond_num_proc);						// Elimina as variáveis de condição
				exit(EXIT_SUCCESS);									
			}

			else if (strncmp(argVector[0], COMANDO_EXIT, 5) == 0) {		// Comando "exit" foi introduzido pela par-shell-terminal
				removePid(atoi(argVector[1]));							// Remove o pid da par-shell-terminal da lista de pids
			}

			else if (strncmp(argVector[0], COMANDO_STATS, 5) == 0) { 	// Verifica se o comando "status" foi

				strcpy(string6, "/tmp/");
				strcat(string6, argVector[1]);
				ficheiro2 = open(string6, O_WRONLY);				// Abre o pipe referente ao terminal que enviou a mensagem
				openedFile(ficheiro2);								

				mutexLock(&mutexsum);													// Início da secção crítica
				sprintf(string4, "O número de processos a correr é %d", numProcessos);
				sprintf(string5, " e o tempo total é %d.", tempoTotal);
				strcat(string4, string5);									// String a enviar para a par-shell-terminal
            	mutexUnlock(&mutexsum);										// Final da secção crítica

				if (write(ficheiro2, string4, strlen(string4)) < 0) {	// Envia para o par-shell-terminal as informações pedidas
			    	perror("");											// Erro na escrita no pipe
			    	exit(EXIT_FAILURE);
            	}
            	close(ficheiro2);									// Fecha o Pipe
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

					if (sprintf(string, "par-shell-out-%d.txt", getpid()) < 0) {	// Coloca na string o nome pretendido
						perror("");													// Não conseguiu colocar correctamente o pretendido
						exit(EXIT_FAILURE);
					}
					
					ficheiro = open(string, O_CREAT|O_WRONLY, FD_PERMISSIONS);	// Cria e abre um ficheiro para escrita		
					openedFile(ficheiro);		

    				closeFile(1);									// Fecha o file descriptor associado ao output do processo corrente
    				dupFile(ficheiro);								// Redirecciona o output do processo para o ficheiro pretendido			
    				closeFile(ficheiro);						 	// Fecha o ficheiro

					execv(argVector[0], argVector);					// Substitui o programa e espaço de endereçamento do processo
																	
					perror("");										
					if (remove(string) < 0) {									// Não executou o exec mas o filho ficou com os dados do pai, sendo
						perror("");
						exit(EXIT_FAILURE);
					}
					free(argVector); 								// necessário fazer free dos mesmos
					closeFileF(file);								// Caso o exec não tenha sido bem sucedido, fecha-se o ficheiro e
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
		else if ((readLineArguments(argVector, NARGUMENTOS)) == -1) {	// Fica à espera que uma nova ligação entre o par-shell e
			ficheiro1 = open(myfifo, O_RDONLY);							// o par-shell-terminal seja estabelecida (caso em que
			openedFile(ficheiro1);									// todas as ligações foram removidas)
		}			
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
																												
							list = update_terminated_process(list, pid, endtime, WEXITSTATUS(status)); 	// Actualiza os dados da lista
							item = encontra_pid(list, pid);												// Item que contem o pid na lista
							tempoTotal = tempoTotal + (int)(endtime - item->starttime);					// Actualiza o tempo total
							atualizaFileF(file);
						}														
					}												
					else {											
						perror("");									// Filho não terminou correctamente								
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
																	
void closeFileF(FILE *file) {										
	if (fclose(file)) {												// Fecha o ficheiro
		perror("");													// Erro ao fechar o ficheiro
		exit(EXIT_FAILURE);											
	}																
}

void closeFile(int j) {
	if (close(j) == -1) {											// Fecha o ficheiro
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

void condInit(pthread_cond_t *cond, int numProcessos) {								
	if (pthread_cond_init(cond, NULL) && numProcessos > 0) {		// Inicialização das variáveis de condição
		perror("Error initializing condition");						// Erro na inicialização das variáveis de condição
		exit(EXIT_FAILURE);
	}
}

void mutexDestroy(pthread_mutex_t *mutex, int numProcessos) {							
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

void readFileF(char *nome) {

	int i = 0;

	if ((file = fopen(nome, "a+")) == NULL) {						// Abre o ficheiro 'log.txt' para leitura e concatenação do mesmo
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
		closeFileF(file);											// Fecha o ficheiro
	}																
																	
	if (str != NULL) {												
		sscanf(str, "%s %s %s %d", str1, str1, str1, &tempoTotal);	// Guarda o valor do tempo total da última iteração
	}																
}

void atualizaFileF(FILE *file) {

	fprintf(file, "iteracao %d\n pid: %d execution time: %d s\n total execution time: %d s\n",
			++iteracao, pid, (int)(endtime - item->starttime), tempoTotal);						
	if (fflush(file)) {																	// Limpa o buffer
		perror("");																		
		exit(EXIT_FAILURE);					
	}
}


void openedFile(int j) {
	if (j == -1) {													
		perror("");													// Erro ao abrir o ficheiro
		exit(EXIT_FAILURE);
	} 
}

void dupFile(int j) {
	if (dup(j) == -1 ) {											// Redireccionamento							
    	perror("");													// Erro ao redireccionar
    	exit(EXIT_FAILURE);
    }	
}

void apanhaCTRLC () {
	killPids(Head);													
	exit(EXIT_SUCCESS);
}

void killPids(Link Head) {											// Faz Kill de todos os processos terminais 
	
	while (Head != NULL) {											// Enquanto houver processos terminais
		if (kill(Head -> item -> valor, SIGKILL) == -1) {				// Faz kill dos processos terminais
			perror("");
			exit(EXIT_FAILURE);
		}
		removePid(Head -> item -> valor);							// Remove os processos terminais da lista
		Head = Head->next;											
	}
}

void inicializa() { 												// Função que vai inicializar a Head e Tail da lista de pids 
	Head = NULL;
	Tail = NULL;
}