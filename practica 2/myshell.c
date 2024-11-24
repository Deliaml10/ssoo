#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

int main(){
char input[1024];
tline *line;
while(1){ //while para que el programa sea un bucle
	//mostrar el prompt
	printf("msh> ");
	fflush(stdout);

	if (fgets(input, sizeof(input), stdin) == NULL){ //mostrar el prompt de nuevo si solo se hace enter
		printf("msh> ");
		fflush(stdout);
	}// fin del if

	line = tokenize(input);
//si no se pasan mandatos
	if (line == NULL || line->ncommands == 0){
		continue;
	}
	if(line->ncommands == 1){
		//se guardan los distintos parametros del mandato
		tcommand mandato = line->commands[0];
		if(strcmp(mandato.argv[0], "exit") == 0){
			printf("Se ha termiando el programa, hasta luego!\n");
			break;
		}

		if(mandato.filename == NULL){
			fprintf(stderr, "El mandato no existe\n");
			continue;
		}

		pid_t pid = fork();
		if(pid < 0){
			fprintf(stderr, "No se ha creado el hijo correctamente\n");
			exit(-1);
		}

		if(pid == 0){
		//se ejecuta el hijo
				//redireccion de entrada
				if(line->redirect_input != NULL){
				FILE *file = fopen(line->redirect_input, "r");
				if(file == NULL){
					fprintf(stderr, "Error al abrir el fichero");
					exit(-1);
				}
				//coge el descriptor de fichero
				int fichero = fileno(file);
				if(dup2(fichero, 0)<0){
					fprintf(stderr, "Error al redirigir la entrada");
					fclose(file);
					exit(-1);
				}else{
				//se redirige bien la entrada
					fclose(file);
				}
			}

			//redireccion de salida
			if(line->redirect_output != NULL){
				FILE *fichero_salida = fopen(line->redirect_output, "w");
				if(fichero_salida == NULL){
					fprintf(stderr, "Error al abrir el fichero de salida");
					exit(-1);
				}
				int fich = fileno(fichero_salida);
				if(dup2(fich, 1) < 0){
					fprintf(stderr, "Error al redirigir la salida");
					fclose(fichero_salida);
					exit(-1);
				}else{
					fclose(fichero_salida);
				}
			}
			//ejecuta el mandato indicado con nombre guardado en filename
			execvp(mandato.filename, mandato.argv);
			// si exec falla
			fprintf(stderr, "Error al ejecutar el mandato");
			exit(-1);
		}else{
		//se ejecuta el padre
			int estado;
			waitpid(pid, &estado, 0);
		}
	}else{
		fprintf(stderr, "Solo se permite un parametro\n");
	}

} // fin del while
return 0;
}
