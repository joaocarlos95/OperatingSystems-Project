#include <stdio.h>
#include <stdlib.h>
#include "commandlinereader.h"
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "list.h"

#define NARGS 7			//constante para definir o número máximo de argumentos

int main (int argc, char **argv) {
	
	char** argVector = (char**) malloc (NARGS*sizeof(char*));		//alocação de memória para o vetor que guarda 
	int pid, status, numProcessos = 0;								//os argumentos introduzidos
	list_t *list;													// ponteiro para estrutura list_t 
	lst_iitem_t *item;												// ponteiro para estrutura lst_iitem_t

	while(1) {

		if (readLineArguments(argVector, NARGS) > 0) {   			//verifica se foram introduzidos argumentos

			if (strcmp(argVector[0], "exit") == 0) {				//instruções para quando for introduzido o exit
				
				list = lst_new();									//nova lista onde vão ser guardados os elementos
				while(numProcessos > 0) {
					pid = wait(&status);							//espera pela terminação do processo filho
					if (pid > 0) {
						if (WIFEXITED(status) == 1) {				//caso o filho tenha terminado normalmente insere
							insert_new_process(list, pid, WEXITSTATUS(status));		//o pid e respectivo status na lista
						}
					}
					else {
						perror("Erro no wait.\n");
					}
					numProcessos--;
				}

				for (item = list->first ; item != NULL ; item = item->next) {
					//imprime todos os pids e inteiros devolvidos pelos filhos
					fprintf(stdout,"O filho do Pid é: %d e o seu valor é: %d\n", item->pid, item->valor);
				}

				lst_destroy(list);									//libertar a memória alocada pela lista
				free(argVector[0]);									//libertar memória do primeiro argumento do vetor
				free(argVector);									//libertar a memória alocada pelo vetor dos argumentos
				exit(EXIT_SUCCESS);
			}
			
			else {

				pid = fork();										//cria processo filho

				if (pid < 0) {										//processo mal criado
					perror("");
					free(argVector);
				}
				else if (pid == 0) {								//processo bem criado 
					execv(argVector[0], argVector);					//substitui o programa e espaço de endereçamento 
					perror("");										//do processo onde é invocado
					free(argVector); 								//não executou o exec mas o filho ficou com os dados
					exit(EXIT_FAILURE);								//do pai, sendo preciso fazer free dos mesmos
				}													
				else {
					numProcessos++;
				}
			}
		}
		else {
			printf("Não inseriu argumentos.\n");
		}
	}

return 0;
}