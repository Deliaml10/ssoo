#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

// Manejo de las señales SIGINT y SIGTSTP
void controlsigint() {
    printf("\nmsh> ");
    fflush(stdout);
}

void controlsigtstp() {
    printf("\nmsh> ");
    fflush(stdout);
}

// Manjedo de procesos hijos en background
void controlsighijo(){
int status;
pid_t pid;

	while((pid = waitpid(-1, &status, WNOHANG)) > 0){
		if(WIFEXITED(status)){
			printf("El proceso %d ha terminado\n", pid);
		}else if(WIFSIGNALED(status)){
			printf("\nEl proceso %d ha terminado debido a una señal\n", pid);
		}
		fflush(stdout);
	}

	if(pid == -1){
		fprintf(stderr, "Error al esperar en background\n");
		exit(-1);
	}
}


int main() {
    char *input = malloc(1024 * sizeof(char)); // Usar malloc para la entrada
    if (input == NULL) {
        perror("Error al asignar memoria\n");
        exit(-1);
    }
    tline *line;

    // Configuración de las señales SIGINT y SIGTSTP
    if(signal(SIGINT, controlsigint) == SIG_ERR ||
       signal(SIGTSTP, controlsigtstp) == SIG_ERR ||
       signal(SIGCHLD, controlsighijo) == SIG_ERR){
	fprintf(stderr, "Error al configurar las señales\n");
	free(input);
	exit(-1);
	}

    while (1) { // Bucle principal del shell
        // Mostrar el prompt
        printf("msh> ");
        fflush(stdout);

        // Leer entrada del usuario
        if (fgets(input, 1024, stdin) == NULL) { // Si solo se hace Enter
            printf("msh> ");
            fflush(stdout);
            continue;
        }

        line = tokenize(input); // Tokenizar la entrada

        // Si no se pasan mandatos, continuar
        if (line == NULL || line->ncommands == 0) {
            continue;
        }

        // Salir si el usuario escribe "exit"
        if (line->ncommands == 1) {
            tcommand mandato = line->commands[0];

            if (strcmp(mandato.argv[0], "exit") == 0) {
                printf("Se ha terminado el programa, hasta luego!\n");
                break;
            }

            // Comando interno "cd"
            if (strcmp(mandato.argv[0], "cd") == 0) {
                char *nuevodir;

                if (mandato.argc == 1) { // Si no se proporcionan argumentos
                    nuevodir = getenv("HOME");
                    if (nuevodir == NULL) {
                        fprintf(stderr, "Error: La variable HOME no está definida.\n");
                        continue;
                    }
                } else {
                    nuevodir = mandato.argv[1]; // Usar el argumento como ruta
                }

                // Cambiar de directorio
                if (chdir(nuevodir) != 0) {
                    fprintf(stderr, "Error al cambiar el directorio\n");
                } else {
                    char diractual[1024];
                    if (getcwd(diractual, sizeof(diractual)) != NULL) {
                        printf("Directorio actual: %s\n", diractual);
                    } else {
                        fprintf(stderr, "Error al obtener el directorio actual\n");
                    }
                }
                continue; // Volver al prompt
            }

            // Caso de un solo mandato
            pid_t pid = fork();
            if (pid < 0) {
                fprintf(stderr, "Error al crear el proceso hijo");
                exit(-1);
            }

            if (pid == 0) { // Proceso hijo
                // Redirección de entrada
                if (line->redirect_input != NULL) {
                    int fichero = open(line->redirect_input, O_RDONLY);
                    if (fichero < 0) {
                        fprintf(stderr, "Error al abrir el fichero de entrada\n");
                        exit(-1);
                    }
                    dup2(fichero, 0);
                    close(fichero);
                }

                // Redirección de salida
                if (line->redirect_output != NULL) {
                    int fichero_salida = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fichero_salida < 0) {
                        fprintf(stderr, "Error al abrir el fichero de salida\n");
                        exit(-1);
                    }
                    dup2(fichero_salida, 1);
                    close(fichero_salida);
                }

                // Ejecutar el mandato
                execvp(mandato.filename, mandato.argv);
                fprintf(stderr, "Error al ejecutar el mandato\n");
                exit(-1);
            } else {
                // Proceso padre
		if(!line->background){
			int estado;
                	waitpid(pid, &estado, 0); // Esperar por el hijo
            	}else{
			printf("Proceso %d se está ejecutando en background\n", pid);
	    	    }
		}

	} else {
            // Caso de múltiples mandatos enlazados con pipes
            int num_pipes = line->ncommands - 1; // Número de pipes necesarios

            // Asignar memoria para los pipes
            int **pipes = malloc(num_pipes * sizeof(int *));
            if (pipes == NULL) {
                perror("Error al asignar memoria para pipes");
                exit(-1);
            }

            for (int i = 0; i < num_pipes; i++) {
                pipes[i] = malloc(2 * sizeof(int));
                if (pipes[i] == NULL) {
                    perror("Error al asignar memoria para un pipe");
                    exit(-1);
                }
                if (pipe(pipes[i]) < 0) {
                    perror("Error al crear el pipe");
                    exit(-1);
                }
            }

            for (int i = 0; i < line->ncommands; i++) {
                pid_t pid = fork();
                if (pid < 0) {
                    perror("Error al crear el proceso hijo");
                    exit(-1);
                }

                if (pid == 0) { // Proceso hijo
                    // Redirección de entrada
                    if (i == 0 && line->redirect_input != NULL) { // Primer comando con entrada redirigida
                        int fichero = open(line->redirect_input, O_RDONLY);
                        if (fichero < 0) {
                            perror("Error al abrir el fichero de entrada");
                            exit(-1);
                        }
                        dup2(fichero, 0);
                        close(fichero);
                    } else if (i > 0) { // Conectar al extremo de lectura del pipe anterior
                        dup2(pipes[i - 1][0], 0);
                    }

                    // Redirección de salida
                    if (i == line->ncommands - 1 && line->redirect_output != NULL) { // Último comando con salida redirigida
                        int fichero_salida = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (fichero_salida < 0) {
                            perror("Error al abrir el fichero de salida");
                            exit(-1);
                        }
                        dup2(fichero_salida, 1);
                        close(fichero_salida);
                    } else if (i < num_pipes) { // Conectar al extremo de escritura del pipe
                        dup2(pipes[i][1], 1);
                    }

                    // Cerrar todos los pipes en el hijo
                    for (int j = 0; j < num_pipes; j++) {
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }

                    // Ejecutar el mandato
                    tcommand comando = line->commands[i];
                    execvp(comando.filename, comando.argv);
                    perror("Error al ejecutar el mandato");
                    exit(-1);
                }
            }

            // Cerrar todos los pipes en el padre
            for (int i = 0; i < num_pipes; i++) {
                close(pipes[i][0]);
                close(pipes[i][1]);
                free(pipes[i]); // Liberar memoria de cada pipe
            }
            free(pipes); // Liberar memoria del array de pipes

            // Esperar a todos los procesos hijos
	    if(!line->background){
        	for (int i = 0; i < line->ncommands; i++) {
                wait(NULL);
		}
	    }else{
		printf("[Proceso ejecutandose en background]\n");

            }
        }
    }

    free(input); // Liberar memoria de la entrada
    return 0;
}

