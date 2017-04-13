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
																	

#define TAMSTRING 50													// Tamanho máximo da String
#define COMANDO_STATS "stats"											// Comando stats
#define COMANDO_EXIT "exit"												// Comando exit
#define COMANDO_EXIT_GLOBAL "exit-global"								// Comando exit global
#define NARGUMENTOS 7													// Número máximo de argumentos
#define FD_PERMISSIONS 0666												// Permissões de leitura e escrita												


// |-------------------|											
// | Variáveis Globais |											
// |-------------------|-------------------------------------------|

char string[TAMSTRING], string1[TAMSTRING], string2[TAMSTRING], string3[TAMSTRING];		// Declaração de strings
char *myfifo, myfifo1[TAMSTRING], myfifo2[TAMSTRING];													
int pidTerminal, ficheiro, ficheiro1;													// Declaração de inteiros

// |------------|													
// | Protótipos |													
// |------------|--------------------------------------------------|


void apanhaCTRLC ();													// Protótipo para a rotina de tratamento


// |------------------|												
// | Função Principal |												
// |------------------|--------------------------------------------|

int main(int argc, char **argv) {

	signal (SIGINT, apanhaCTRLC);										// Redefinição do signal SIGINT

	pidTerminal = getpid();												// Obtem o pid do terminal
	myfifo = argv[1];													// Guarda o nome do pipe

	if ((ficheiro = open(myfifo, O_WRONLY)) < 0) {						// Abre o pipe para leitura
		perror("");
		exit(EXIT_FAILURE);
	}

	if (sprintf(string2, "listaPids %d\n", pidTerminal) < 0) {			// Transforma o pid numa string e junta-o a "listPids"
		perror("");
		exit(EXIT_FAILURE);
	}
	if (write(ficheiro, string2, strlen(string2)) < 0) {				// Escreve no pipe a string2
		perror("");
		exit(EXIT_FAILURE);
    }

	while(1) {

		fgets(string, TAMSTRING, stdin);								// Vai buscar os comandos introduzidos no terminal

		if (strncmp(string, COMANDO_EXIT_GLOBAL, 11) == 0 && strlen(string) == 12) { // Comando exit-global introduzido
		if (write(ficheiro, string, strlen(string)) < 0) {			// Escreve no pipe "exit-global" para enviar à par-shell
				perror("");
				exit(EXIT_FAILURE);
   			}
		}

		else if (strncmp(string, COMANDO_STATS, 5) == 0) {				// Comando stats introduzido

			if (sprintf(myfifo2, "/tmp/%d", pidTerminal) < 0) {				// Transforma o pid numa string
				perror("");
				exit(EXIT_FAILURE);
			}

			unlink(myfifo2);											// Remove o pipe caso exista											
			if (mkfifo(myfifo2, FD_PERMISSIONS) == -1) {				// Cria o pipe com permissões de leitura e escrita
				perror("");
				exit(EXIT_FAILURE);
			}
			
			if (sprintf(myfifo1, "stats %d\n", pidTerminal) == -1) {	// Transforma o pid numa string e junta-o a "listPids"
				perror("");
				exit(EXIT_FAILURE);
			}
			if (write(ficheiro, myfifo1, strlen(myfifo1)) < 0) {		// Escreve no pipe "stats" e o pid do terminal 
				perror("");
			    exit(EXIT_FAILURE);
			}

			if((ficheiro1 = open(myfifo2, O_RDONLY)) < 0) {				// Abre o pipe com o nome do seu pid para leitura
				perror("");
				exit(EXIT_FAILURE);
			}
			
			if (read(ficheiro1, string1, 200) < 0) {					// Lê do pipe as informações referentes ao stats enviada 
																		// pela par-shell
			    perror("");
			    exit(EXIT_FAILURE);
            }
			printf("%s\n", string1);           
			unlink(myfifo2);											// Remove o pipe caso exista
		}
		
		else if (strncmp(string, COMANDO_EXIT, 4) == 0 && strlen(string) == 5) { // Comando exit introduzido
			if (sprintf(string3, "exit %d\n", pidTerminal) == -1) {		// Transforma o pid numa string e junta-o a "exit"
				perror("");
				exit(EXIT_FAILURE);
			}

			if (write(ficheiro, string3, strlen(string3)) < 0) {		// Escreve no pipe "exit" e o pid do terminal
				perror("");
				exit(EXIT_FAILURE);
   			}

			unlink(myfifo2);											// Remove o pipe caso exista
			exit(EXIT_SUCCESS);
		}

		else if (strncmp(string, COMANDO_EXIT , 4) != 0 && strncmp(string, "listaPids", 9) != 0) { 

			if (write(ficheiro, string, strlen(string)) < 0) {			// Escreve no pipe a restante informação
			    perror("");
			    exit(EXIT_FAILURE);
            }
		}
	}
	return 0;															
}

void apanhaCTRLC () {

	if (sprintf(string3, "exit %d\n", pidTerminal) == -1) {				// Transforma o pid numa string e junta-o a "exit"
		perror("");
		exit(EXIT_FAILURE);
	}

	if (write(ficheiro, string3, strlen(string3)) < 0) {				// Escreve no pipe "exit" e o pid do terminal
		perror("");
		exit(EXIT_FAILURE);
   	}

	unlink(myfifo2);													// Remove o pipe caso exista
	exit(EXIT_SUCCESS);
}