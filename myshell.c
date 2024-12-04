#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h> // Para open()

int main() {
    pid_t pid;
    char input[1024]; // Esto hay que hacerlo con malloc
    tline *line;

    while (1) { // Bucle principal del shell
        // Mostrar el prompt
        printf("msh> ");
        fflush(stdout);

        // Leer entrada del usuario
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }

        line = tokenize(input); // Tokenizar la entrada

	if(line == NULL || line->ncommands == 0){
		continue;
	}

	//comando cd
	if(line->ncommands == 1 && strcmp(line->commands[0].argv[0], "cd") == 0){
		char *nuevodir;
		tcommand mandato = line->commands[0];
		if(mandato.argc == 1){
			nuevodir = getenv("HOME");
			if(nuevodir == NULL){
				fprintf(stderr, "El directorio no está definido\n");
				continue;
			}
		}else{
			nuevodir = mandato.argv[1];
		}

		if(chdir(nuevodir) != 0){
			fprintf(stderr, "Error al cambiar el directorio\n");
			continue;
		}else{
			char diractual[1024];
			if(getcwd(diractual, sizeof(diractual)) != NULL){
				printf("El directorio actual es: %s\n", diractual);
			}else{
				fprintf(stderr, "Error al cambiar el directorio actual\n");
				continue;
			}
		}
	}



        // Salir si el usuario escribe "exit"
        else if (line->ncommands == 1 && strcmp(line->commands[0].argv[0], "exit") == 0) {
            printf("Se ha terminado el programa, hasta luego!\n");
            break;
// BORRAR TODOS LOS MANDATOS "LIBERAR MEMORIA"
        }
        // Caso de un solo mandato
        else if (line->ncommands == 1) {
            tcommand mandato = line->commands[0];

	    if(mandato.filename == NULL){
		fprintf(stderr, "El mandato no existe");
	    }

            pid = fork();
            if (pid < 0) {
                fprintf(stderr, "Error al crear el proceso hijo");
                exit(-1);
            }
	    // proceso hijo
            if (pid == 0) {
                // Redirección de entrada
                if (line->redirect_input != NULL) {
                    int fichero = open(line->redirect_input, O_RDONLY);
                    if (fichero < 0) {
                        fprintf(stderr, "Error al abrir el fichero de entrada");
                        exit(-1);
                    }
		    printf("FICHERO ABIERTO");
                    dup2(fichero, 0);
                    close(fichero);

                }

                // Redirección de salida
                if (line->redirect_output != NULL) {
                    int fichero = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fichero < 0) {
                        fprintf(stderr, "Error al abrir el fichero de salida");
                        exit(-1);
                    }
                    dup2(fichero, 1);
                    close(fichero);
                }

		// Redireccion de salida error
		if(line->redirect_error != NULL){
			int fichero = open(line->redirect_error, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if(fichero < 0){
				fprintf(stderr, "Error al abrir el fichero de salida error");
				exit(-1);
			}
			dup2(fichero, 2);
			close(fichero);
		}

                // Ejecutar el mandato
                execvp(mandato.filename, mandato.argv);
                fprintf(stderr, "Error al ejecutar el mandato");
                exit(-1);
            } else {
                // Proceso padre
                int estado;
                waitpid(pid, &estado, 0); // Esperar a que el hijo termine
            }
        }

        // Caso de mas de dos
        else if (line->ncommands >= 2) {
		int numero_pipes = line->ncommands - 1;
		int **tuberia = malloc(numero_pipes * sizeof(int *));
		if(tuberia == NULL){
			fprintf(stderr, "Error al asignar memoria para crear la tuberia");
			exit(-1);
		}
		for (int i = 0; i < numero_pipes; i++){
			tuberia[i] = malloc(2 * sizeof(int));
			if(pipe(tuberia[i]) < 0){
				fprintf(stderr, "Error al crear los extremos del pipe");
				exit(-1);
			}
		}

		for (int i = 0; i < line->ncommands; i++){
			pid = fork();

			if (pid < 0) {
				fprintf(stderr, "Error al crear el primer proceso hijo");
				exit(-1);
			}

			if (pid == 0) {
				if(i == 0){
					//redireccion entrada estandar
					if(line->redirect_input != NULL){
						int fichero = open(line->redirect_input, O_RDONLY);
						if(fichero < 0){
							fprintf(stderr, "Error al abrir el fichero de entrada");
							exit(-1);
						}
						dup2(fichero, 0);
						close(fichero);
					}
					//salida al pipe
					dup2(tuberia[i][1], 1);
					for(int j = 0; j < line->ncommands-1; j++){
						close(tuberia[j][0]);
						close(tuberia[j][1]);
					}
				}else if(i > 0 && i < (line->ncommands - 1)){

					//entrada pipe
					dup2(tuberia[i-1][0], 0);
					//salida pipe
					dup2(tuberia[i][1], 1);
					for(int j = 0; j < line->ncommands-1; j++){
						close(tuberia[j][0]);
						close(tuberia[j][1]);
					}

				}else if( i == line->ncommands - 1){
					//entrada pipe
					dup2(tuberia[i-1][0], 0);
					//salida estandar
					if(line->redirect_output != NULL){
						int fichero = open(line->redirect_output, O_WRONLY | O_TRUNC | O_CREAT, 0644);
						if(fichero < 0){
							fprintf(stderr, "Error al abrir el fichero de salida");
							exit(-1);
						}
						dup2(fichero, 1);
						close(fichero);
					}
					if(line->redirect_error != NULL){
						int fichero = open(line->redirect_error, O_WRONLY | O_TRUNC | O_CREAT, 0644);
						if(fichero < 0){
							fprintf(stderr, "Error al abrir el fichero de salida error");
							exit(-1);
						}
						dup2(fichero, 2);
						close(fichero);
					}

					for(int j=0; j<line->ncommands-1; j++){
						close(tuberia[j][0]);
						close(tuberia[j][1]);
					}
				}


				tcommand mandato = line->commands[i];
				execvp(mandato.filename, mandato.argv);
				fprintf(stderr, "Error al ejecutar los mandatos");
				exit(-1);

			}
	}
            	// Proceso padre
		for(int i = 0; i < line->ncommands-1; i++){
            		close(tuberia[i][0]);
            		close(tuberia[i][1]);
		}
		for(int i = 0; i < line->ncommands; i++){
            		wait(NULL); // Esperar al primer hijo
		}
	free(tuberia);
        } else {
		fprintf(stderr, "Hay algún error");
		exit(-1);
        }
    }
    return 0;
}
