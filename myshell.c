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

job *trabajos = NULL;

int cuenta = 0;  //inicializo la cuenta de los trabajos a 0
char *input = NULL;
pid_t pid;
pid_t proceso_fg;

void actualizar_fg(pid_t pid) {
  proceso_fg = pid;
}
void funcionumask(tline *line) {
int i;
char *arg;
int mask;
mode_t actualumask;

    if (line->ncommands == 1 && line->commands[0].argc > 2) {
        fprintf(stderr, "El comando umask solo puede tener un argumento o ninguno.\n");
        return;
    }
    if (line->ncommands == 1 && line->commands[0].argc == 2) {
        arg = line->commands[0].argv[1];

        for (i = 0; arg[i] != '\0'; i++) {
            if (arg[i] < '0' || arg[i] > '7') {
                fprintf(stderr, "El argumento de umask debe ser un número octal válido.\n");
                return;
            }
        }

        mask = strtol(arg, NULL, 8);

        if (mask < 0 || mask > 0777) {
            fprintf(stderr, "El valor de umask debe ser un número octal.\n");
            return;
        }
        umask(mask); // Establece la máscara de permisos
        printf("Máscara de permisos cambiada a: %03o\n", mask);
    } else {
        // Si no se pasa argumento, mostrar la máscara actual
        actualumask = umask(0); // Obtiene la máscara actual
        umask(actualumask); // Restaura la máscara
        printf("Máscara de permisos actual: %03o\n", actualumask);
    }
}
void cambiarestado(pid_t pid, char *nuevoestado){
int i;
    for(i = 0; i < cuenta; i++){
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

void addjob(pid_t pid, char *command, char *estado) {
int i;
    for (i = 0; i < cuenta; i++) {
        if (trabajos[i].pid == pid) {
            cambiarestado(pid, estado);
            return;
        }
    }

    trabajos = realloc(trabajos, sizeof(job) * (cuenta + 1));
    if (trabajos == NULL) {
        fprintf(stderr, "Error al asignar memoria para los trabajos\n");
        exit(-1);
    }

    trabajos[cuenta].id = cuenta + 1;
    trabajos[cuenta].pid = pid;
    trabajos[cuenta].command = strdup(command);
    trabajos[cuenta].estado = strdup(estado);

    if (trabajos[cuenta].command == NULL || trabajos[cuenta].estado == NULL) {
        fprintf(stderr, "Error al asignar memoria\n");
        exit(-1);
    }
    cuenta++;
}

void showjob(){
int i;
    for(i = 0; i < cuenta; i++){
        printf("[%d] %s %s\n", trabajos[i].id, trabajos[i].estado, trabajos[i].command);
    }
	if(cuenta == 0){
		printf("No hay trabajos en segundo plano, ni parados ni ejecutandose\n");
	}
}

void borrarjob(pid_t pid) {
int i, j;
job *temp;
    for(i = 0; i < cuenta; i++) {
        if(trabajos[i].pid == pid) {
            free(trabajos[i].command);
            free(trabajos[i].estado);

            // Mover todos los trabajos una posición hacia arriba
            for(j = i; j < cuenta - 1; j++) {
                trabajos[j] = trabajos[j + 1];
                trabajos[j].id = j + 1;
            }
            cuenta--;

            // Redimensionar el array
            if(cuenta > 0) {
                temp = realloc(trabajos, sizeof(job) * cuenta);
                if(temp != NULL) {
                    trabajos = temp;
                }
            } else {
                free(trabajos);
                trabajos = NULL;
            }
            return;
        }
    }
}

void manejador_SIGCHLD() {
    int estado;
    pid_t pid;

    while ((pid = waitpid(-1, &estado, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        if (WIFEXITED(estado) || WIFSIGNALED(estado)) {
            borrarjob(pid);
        } else if (WIFSTOPPED(estado)) {
            if (pid == proceso_fg) {
                cambiarestado(pid, "Stopped");
                proceso_fg = 0;
            }
        } else if (WIFCONTINUED(estado)) {
            cambiarestado(pid, "Running");
            printf("\nProceso %d ha continuado su ejecución\n", pid);
        }
    }
}

void trabajosterminados() {
    manejador_SIGCHLD(SIGCHLD);
}

void borrartrabajos(){
int i;
    for(i = 0; i < cuenta; i++){
        free(trabajos[i].estado);
        free(trabajos[i].command);
    }
    free(trabajos);
    trabajos = NULL;
    cuenta = 0;
}

void moverforeground(pid_t pid) {
int i, estado;
    for(i = 0; i < cuenta; i++) {
        if(trabajos[i].pid == pid) {
            actualizar_fg(pid);
            cambiarestado(pid, "Running");
            if(kill(pid, SIGCONT) == -1) {
                fprintf(stderr, "Error al continuar el proceso\n");
                return;
            }
            waitpid(pid, &estado, WUNTRACED);
            proceso_fg = 0;
            if (!WIFSTOPPED(estado)) {
                borrarjob(pid);
            }
        }
    }
}

void controlZ() {
pid_t pid_actual;
int i;
    if (proceso_fg > 0) {
        pid_actual = proceso_fg;
        proceso_fg = 0;

        for(i = 0; i < cuenta; i++) {
            if(trabajos[i].pid == pid_actual) {
                cambiarestado(pid_actual, "Stopped");
                kill(pid_actual, SIGTSTP);
                return;
            }
        }
        kill(pid_actual, SIGTSTP);
        addjob(pid_actual, input, "Stopped");
        printf("\nEl mandato con PID: %d ha sido detenido y mandado al background\n", pid_actual);
        fflush(stdout);
    } else {
        printf("\nmsh> ");
        fflush(stdout);
    }
}

void controlC() {
int i;
    if (proceso_fg > 0) {  // Solo si hay un proceso en foreground
        for(i = 0; i < cuenta; i++) {
            if(trabajos[i].pid == proceso_fg) {
                return;  // No hacer nada si el proceso está en background
            }
        }
        kill(proceso_fg, SIGKILL);
        printf("\nEl proceso con PID %d ha sido terminado\n", proceso_fg);
        fflush(stdout);
    } else {
        printf("\nmsh> ");
        fflush(stdout);
    }
}

void exitShell() {
int i;
pid_t pid;
    for (i = 0; i < cuenta; i++) {
        if (trabajos[i].estado == NULL || strcmp(trabajos[i].estado, "Stopped") != 0) {
            pid = trabajos[i].pid;
            borrarjob(pid);
        }
    }

	borrartrabajos();
	for(i = 0; i < cuenta; i++){
		free(trabajos[i].command);
		free(trabajos[i].estado);
	}
	free(trabajos);
	free(input);
	trabajos = NULL;
	input = NULL;

	printf("Se ha terminado el programa, hasta luego!\n");
	exit(0);
}

void bg(pid_t pid){
int i;
	if(cuenta == 0){
		printf("No hay trabajos detenidos o en segundo plano\n");
		fflush(stdout);
		return;
	}

	if(pid < 0){
		for(i = cuenta - 1; i >= 0; i--){
			if(strcmp(trabajos[i].estado, "Stopped") == 0) {
				pid = trabajos[i].pid;
				break;
			}
		}
	}
	for(i = 0; i < cuenta; i++){
		if(trabajos[i].pid == pid){
			if(strcmp(trabajos[i].estado, "Stopped") != 0){
				fprintf(stderr, "El trabajo con PID: %d ya esta en ejecución\n", pid);
				return;
			}
			if(kill(trabajos[i].pid, SIGCONT) == -1){
				fprintf(stderr, "Error al continuar con la ejecución del trabajo\n");
				exit(-1);
			}
			printf("El trabajo con PID: %d se ha reanudado en el background\n", pid);
			fflush(stdout);
			cambiarestado(pid, "Running");
			return;
		}
	}
	fprintf(stderr, "No se encontró ningún trabajo con el PID especificado\n");
	return;
	pid = -1;
}

int main() {
int i, j, encontrado, fichero, estado, numero_pipes;
int **tuberia;
char *nuevodir;
char diractual[1024];
pid_t pid;
char *end;
    tline *line;

    input = malloc(1024 * sizeof(char));
    if (input == NULL) {
        fprintf(stderr, "Error al asignar memoria para la entrada\n");
        exit(-1);
    }


    if(signal(SIGCHLD, manejador_SIGCHLD) == SIG_ERR){
        fprintf(stderr, "Error al configurar SIGCHLD\n");
        exit(-1);
    }

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
        trabajosterminados();
        printf("msh> ");
        fflush(stdout);

        // Leer entrada del usuario
        if (fgets(input, 1024, stdin) == NULL) {
            printf("\n");
            break;
        }

        line = tokenize(input); // Tokenizar la entrada

        if(line == NULL || line->ncommands == 0){
            continue;
        }

        encontrado = 0;
        for (i = 0; i < line->ncommands; i++) {
            if (strcmp(line->commands[i].argv[0], "umask") == 0 || strcmp(line->commands[i].argv[0], "cd") == 0  ){
                  encontrado = 1;
                  break;
            }
        }

        if (encontrado && line->ncommands > 1) {
            fprintf(stderr, "El mandato no puede ejecutarse con pipes.\n");
            continue;
        }

        if (line->ncommands == 1 && strcmp(line->commands[0].argv[0], "umask") == 0) {
            funcionumask(line);
            continue;
        }

        //comando cd
        if(line->ncommands == 1 && strcmp(line->commands[0].argv[0], "cd") == 0){
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

	// Mandato interno bg
	else if(line->ncommands == 1 && strcmp(line->commands[0].argv[0], "bg") == 0){
		pid = -1;
		if(line->commands[0].argc == 2){
			pid = strtol(line->commands[0].argv[1], &end, 10);
		}
		bg(pid);
	}

        // Caso de un solo mandato
        else if (line->ncommands == 1) {
            tcommand mandato = line->commands[0];

            if(mandato.filename == NULL){
                fprintf(stderr, "El mandato no existe\n");
            }

            pid = fork();
            if (pid < 0) {
                fprintf(stderr, "Error al crear el proceso hijo\n");
                exit(-1);
            }
            if(line->background == 0){
                actualizar_fg(pid);
            }
            // proceso hijo
            if (pid == 0) {
                // Redirección de entrada
                signal(SIGCHLD, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGINT, SIG_DFL);

                if (line->redirect_input != NULL) {
                    fichero = open(line->redirect_input, O_RDONLY);
                    if (fichero < 0) {
                        fprintf(stderr, "Error al abrir el fichero de entrada\n");
                        exit(-1);
                    }
                    dup2(fichero, 0);
                    close(fichero);
                }

                // Redirección de salida
                if (line->redirect_output != NULL) {
                    fichero = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fichero < 0) {
                        fprintf(stderr, "Error al abrir el fichero de salida\n");
                        exit(-1);
                    }
                    dup2(fichero, 1);
                    close(fichero);
                }

                // Redireccion de salida error
                if(line->redirect_error != NULL){
                    fichero = open(line->redirect_error, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if(fichero < 0){
                        fprintf(stderr, "Error al abrir el fichero de salida error\n");
                        exit(-1);
                    }
                    dup2(fichero, 2);
                    close(fichero);
                }

                // Ejecutar el mandato
                execvp(mandato.filename, mandato.argv);
                fprintf(stderr, "Error al ejecutar el mandato\n");
                exit(-1);
            } else {
    // Proceso padre
                  if(line->background == 1){
                      printf("El proceso con PID: %d, se está ejecutando en background\n", pid);
                      addjob(pid, input, "Running");
                  } else {
                      waitpid(pid, &estado, WUNTRACED);
                      proceso_fg = 0;  // Asegurarse de resetear proceso_fg
                  }
        }

}
// Caso de mas de dos
        else if (line->ncommands >= 2) {
		numero_pipes = line->ncommands - 1;
		tuberia = malloc(numero_pipes * sizeof(int *));
		if(tuberia == NULL){
			fprintf(stderr, "Error al asignar memoria para crear la tuberia\n");
			exit(-1);
		}
		for (i = 0; i < numero_pipes; i++){
			tuberia[i] = malloc(2 * sizeof(int));
			if(pipe(tuberia[i]) < 0){
				fprintf(stderr, "Error al crear los extremos del pipe\n");
				exit(-1);
			}
		}

		for (i = 0; i < line->ncommands; i++){
			pid = fork();

			if (pid < 0) {
				fprintf(stderr, "Error al crear el primer proceso hijo\n");
				exit(-1);
			}

                       if(line->background == 0){
                                actualizar_fg(pid);
                       }
			if (pid == 0) {
			signal(SIGCHLD, SIG_DFL);
                        signal(SIGTSTP, SIG_DFL);
                        signal(SIGINT, SIG_DFL);
				if(i == 0){
					//redireccion entrada estandar
					if(line->redirect_input != NULL){
						fichero = open(line->redirect_input, O_RDONLY);
						if(fichero < 0){
							fprintf(stderr, "Error al abrir el fichero de entrada\n");
							exit(-1);
						}
						dup2(fichero, 0);
						close(fichero);
					}
					//salida al pipe
					dup2(tuberia[i][1], 1);
					for(j = 0; j < line->ncommands-1; j++){
						close(tuberia[j][0]);
						close(tuberia[j][1]);
					}
				}else if(i > 0 && i < (line->ncommands - 1)){

					//entrada pipe
					dup2(tuberia[i-1][0], 0);
					//salida pipe
					dup2(tuberia[i][1], 1);
					for(j = 0; j < line->ncommands-1; j++){
						close(tuberia[j][0]);
						close(tuberia[j][1]);
					}

				}else if( i == line->ncommands - 1){
					//entrada pipe
					dup2(tuberia[i-1][0], 0);
					//salida estandar
					if(line->redirect_output != NULL){
						fichero = open(line->redirect_output, O_WRONLY | O_TRUNC | O_CREAT, 0644);
						if(fichero < 0){
							fprintf(stderr, "Error al abrir el fichero de salida\n");
							exit(-1);
						}
						dup2(fichero, 1);
						close(fichero);
					}
					if(line->redirect_error != NULL){
						fichero = open(line->redirect_error, O_WRONLY | O_TRUNC | O_CREAT, 0644);
						if(fichero < 0){
							fprintf(stderr, "Error al abrir el fichero de salida error\n");
							exit(-1);
						}
						dup2(fichero, 2);
						close(fichero);
					}

					for(j=0; j<line->ncommands-1; j++){
						close(tuberia[j][0]);
						close(tuberia[j][1]);
					}
				}


				tcommand mandato = line->commands[i];
				execvp(mandato.filename, mandato.argv);
				fprintf(stderr, "Error al ejecutar los mandatos\n");
				exit(-1);

			}
	}
            	// Proceso padre
		for(i = 0; i < line->ncommands-1; i++){
            		close(tuberia[i][0]);
            		close(tuberia[i][1]);
		}
		if(line->background == 1){
			printf("El proceso con PID: %d, se está ejecutando en background\n", pid);
			addjob(pid, input, "Running");
		}else{
			for(i = 0; i < line->ncommands; i++){
            		      wait(&estado); // Esperar al primer hijo
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
