#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h> // Para open()
#include <signal.h>
#include <sys/stat.h>

typedef struct job {
    int id;  //numero del trabajo
    pid_t pid;
    char *command;
    char *estado;
}job;

job *trabajos = NULL;  //inicializo la lista de trabajos a NULL
int cuenta = 0;  //inicializo la cuenta de los trabajos a 0


void funcionumask(tline *line) {
    if (line->ncommands == 1 && line->commands[0].argc > 2) {
        fprintf(stderr, "El comando umask solo puede tener un argumento o ninguno.\n");
        return;
    }
    if (line->ncommands == 1 && line->commands[0].argc == 2) {
        char *arg = line->commands[0].argv[1];

        for (int i = 0; arg[i] != '\0'; i++) {
            if (arg[i] < '0' || arg[i] > '7') {
                fprintf(stderr, "El argumento de umask debe ser un número octal válido.\n");
                return;
            }
        }

        int mask = strtol(arg, NULL, 8);

        if (mask < 0 || mask > 0777) {
            fprintf(stderr, "El valor de umask debe ser un número octal.\n");
            return;
        }
        umask(mask); // Establece la máscara de permisos
        printf("Máscara de permisos cambiada a: %d\n", mask);
    } else {
        // Si no se pasa argumento, mostrar la máscara actual
        mode_t actualumask = umask(0); // Obtiene la máscara actual
        umask(actualumask); // Restaura la máscara
        printf("Máscara de permisos actual: %d\n", actualumask);
    }
}

void addjob(pid_t pid, char *command, char *estado){
    trabajos = realloc(trabajos, sizeof(struct job) * (cuenta + 1));
    if(trabajos == NULL){
        fprintf(stderr, "Error al asignar memoria para jobs\n");
        exit(-1);
    }
    trabajos[cuenta].id = cuenta + 1;
    trabajos[cuenta].pid = pid;
    trabajos[cuenta].command = strdup(command); // se crea una copia de los comandos y se guarda en el tipo job
    trabajos[cuenta].estado = strdup(estado); //duplica el valor del estado y lo guarda

    if(trabajos[cuenta].command == NULL || trabajos[cuenta].estado == NULL){
        fprintf(stderr, "Error al duplicar el mandato");
        exit(-1);
    }
    cuenta++;
}

void showjob(){
    for(int i = 0; i < cuenta; i++){
        printf("[%d] %s %s\n", trabajos[i].id, trabajos[i].estado, trabajos[i].command);
    }
	if(cuenta == 0){
		printf("No hay trabajos en segundo plano, ni parados ni ejecutandose\n");
	}
}

void borrarjob(pid_t pid){
    for(int i = 0; i < cuenta ; i++){
        if(trabajos[i].pid == pid){
            free(trabajos[i].command);
            free(trabajos[i].estado);
            for(int j = i; j < cuenta - 1; j++){
                trabajos[j] = trabajos[j + 1];
            }
            cuenta--;

            trabajos = realloc(trabajos, sizeof(struct job) * cuenta);
            // Aquí se tiene en cuenta la variable "cuenta" porque si es igual a 0 el realloc daría NULL si o si, pero eso no sería un error
            if(trabajos == NULL && cuenta > 0){
                fprintf(stderr, "Error al reasignar memoria para los trabajos\n");
                exit(-1);
            }
        }
    }
}

void cambiarestado(pid_t pid, char *nuevoestado){
    for(int i = 0; i < cuenta; i++){
        if(trabajos[i].pid == pid){
            free(trabajos[i].estado);
            trabajos[i].estado = strdup(nuevoestado);
            if(trabajos[i].estado == NULL){
                fprintf(stderr, "Error al cambiar el estado del mandato");
                exit(-1);
            }
        }
    }
}

void borrartrabajos(){
    for(int i = 0; i < cuenta; i++){
        free(trabajos[i].estado);
        free(trabajos[i].command);
    }
    free(trabajos);
    trabajos = NULL;
    cuenta = 0;
}

void trabajosterminados(){
    int estado;
    pid_t pid;

    while((pid = waitpid(-1, &estado, WNOHANG)) > 0){
        borrarjob(pid);
	printf("El trabajo con PID: %d, ha terminado\n", pid);
    }
}

void moverforeground(pid_t pid){
    for(int i = 0; i < cuenta; i++){
        if(trabajos[i].pid == pid){
            cambiarestado(pid, "Running");
            int estado;
            waitpid(pid, &estado, 0);
            borrarjob(pid);
        }
    }
}

void controlZ() {
    pid_t pidHijo = 0;
    char *command;
    for (int i = 0; i < cuenta; i++) {
        if (strcmp(trabajos[i].estado, "Running") == 0) {
            pidHijo = trabajos[i].pid;
            command = trabajos[i].command;
	    break;
        }
    }

    if (pidHijo != 0) {
        kill(pidHijo, SIGSTOP);
        printf("\nEl proceso con PID %d ha sido detenido y enviado a segundo plano\n", pidHijo);
	for( int j = 0; j < cuenta; j++){
		if(trabajos[j].pid == pidHijo){
    		    	cambiarestado(pidHijo, "Stopped");
		}else{
			addjob(pidHijo, command, "Stopped");
		}
	}

    }

    printf("\nmsh> ");
    fflush(stdout);
}


void controlC() {
    pid_t pidHijo = 0;

    for (int i = 0; i < cuenta; i++) {
        if (strcmp(trabajos[i].estado, "Running") == 0) {
            pidHijo = trabajos[i].pid;
            break;
        }
    }

    if (pidHijo != 0) {
        kill(pidHijo, SIGKILL);
        printf("\nEl proceso con PID %d ha sido terminado\n", pidHijo);
        borrarjob(pidHijo);
    }
    printf("\nmsh> ");
    fflush(stdout);
}

void exitShell() {
    for (int i = 0; i < cuenta; i++) {
        if (trabajos[i].estado == NULL || strcmp(trabajos[i].estado, "Stopped") != 0) {
            pid_t pid = trabajos[i].pid;
            int estado;
            waitpid(pid, &estado, 0);
            printf("El trabajo con PID %d ha terminado\n", pid);
            borrarjob(pid);
        }
    }

    borrartrabajos();

    printf("Se ha terminado el programa, hasta luego!\n");
}

int main() {
    pid_t pid;
    char input[1024]; // Esto hay que hacerlo con malloc
    tline *line;

    if(signal(SIGTSTP, controlZ) == SIG_ERR){
        fprintf(stderr, "Error al configurar el Control+Z\n");
        exit(-1);
    }

    if(signal(SIGINT, controlC) == SIG_ERR){
        fprintf(stderr, "Error al configurar el Control+C\n");
        exit(-1);
    }

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

        int encontrado = 0;
        for (int i = 0; i < line->ncommands; i++) {
            if(strcmp(line->commands[i].argv[0], "exit") == 0 ||
	       strcmp(line->commands[i].argv[0], "cd") == 0 ||
	       strcmp(line->commands[i].argv[0], "umask") == 0 ||
	       strcmp(line->commands[i].argv[0], "bg") == 0) {
                  encontrado = 1;
                  break;
            }
        }

        if (encontrado && line->ncommands > 1) {
            fprintf(stderr, "Uno de los mandatos no puede ejecutarse con pipes.\n");
            continue;
        }

        if (line->ncommands == 1 && strcmp(line->commands[0].argv[0], "umask") == 0) {
            funcionumask(line);
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
	// Mandato interno jobs
        else if(line->ncommands == 1 && strcmp(line->commands[0].argv[0], "jobs") == 0){
            	trabajosterminados();
		showjob();
        }


        // Salir si el usuario escribe "exit"
        else if (line->ncommands == 1 && strcmp(line->commands[0].argv[0], "exit") == 0) {
            exitShell();
            break;
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
                if(line->background == 1){
                    printf("El proceso con PID: %d, se está ejecutando en background\n", pid);
                    addjob(pid, line->commands[0].argv[0], "Running"); // Cambié "line->commands[i]" por "line->commands[0].argv[0]"
                    continue;
                } else {
                    waitpid(pid, &estado, 0); // Esperar a que el hijo termine
                }
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
		if(line->background == 1){
			printf("El proceso con PID: %d, se está ejecutando en background\n", pid);
			for(int i = 0; i < line->ncommands -1; i++){
				addjob(pid, line->commands[i].argv[0], "Running");
			}
			continue;
		}else{
			for(int i = 0; i < line->ncommands; i++){
            			wait(NULL); // Esperar al primer hijo
			}
		}
	free(tuberia);
        } else {
		fprintf(stderr, "Hay algún error");
		exit(-1);
        }
    }
    return 0;
}
