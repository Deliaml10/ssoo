#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h> // Para open()

// Manejo de las señales SIGINT y SIGTSTP
void controlsigint() {
    printf("\nmsh> ");
    fflush(stdout);
}

void controlsigtstp() {
    printf("\nmsh> ");
    fflush(stdout);
}

//programa principal
int main() {
    char input[1024]; // Buffer de entrada del usuario
    tline *line;

    // Configuración de las señales SIGINT y SIGTSTP
    if (signal(SIGINT, controlsigint) == SIG_ERR) {
        fprintf(stderr, "Error al configurar SIGINT\n");
        exit(-1);
    }

    if (signal(SIGTSTP, controlsigtstp) == SIG_ERR) {
        fprintf(stderr, "Error al configurar SIGTSTP\n");
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

        // Si no se pasan mandatos, continuar
        if (line == NULL || line->ncommands == 0) {
            continue;
        }

        // Salir si el usuario escribe "exit"
        if (line->ncommands == 1 && strcmp(line->commands[0].argv[0], "exit") == 0) {
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
        if (line->ncommands == 1) {
            tcommand mandato = line->commands[0];
            if (mandato.filename == NULL) {
                fprintf(stderr, "El mandato no existe\n");
                continue;
            }

            pid_t pid = fork();
            if (pid < 0) {
                perror("Error al crear el proceso hijo");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) {
                // Redirección de entrada
                if (line->redirect_input != NULL) {
                    int fd = open(line->redirect_input, O_RDONLY);
                    if (fd < 0) {
                        perror("Error al abrir el fichero de entrada");
                        exit(EXIT_FAILURE);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }

                // Redirección de salida
                if (line->redirect_output != NULL) {
                    int fd = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd < 0) {
                        perror("Error al abrir el fichero de salida");
                        exit(EXIT_FAILURE);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }

                // Ejecutar el mandato
                execvp(mandato.filename, mandato.argv);
                perror("Error al ejecutar el mandato");
                exit(EXIT_FAILURE);
            } else {
                // Proceso padre
                int estado;
                waitpid(pid, &estado, 0); // Esperar a que el hijo termine
            }
        }

        // Caso de dos mandatos enlazados con pipe
        else if (line->ncommands == 2) {
            int pipefd[2]; // Crear el pipe
            if (pipe(pipefd) < 0) {
                perror("Error al crear el pipe");
                exit(EXIT_FAILURE);
            }

            // Primer hijo (mandato 1)
            pid_t pid1 = fork();
            if (pid1 < 0) {
                perror("Error al crear el primer proceso hijo");
                exit(EXIT_FAILURE);
            }

            if (pid1 == 0) {
                close(pipefd[0]); // Cerrar extremo de lectura del pipe

                // Redirección de entrada
                if (line->redirect_input != NULL) {
                    int fd = open(line->redirect_input, O_RDONLY);
                    if (fd < 0) {
                        perror("Error al abrir el fichero de entrada");
                        exit(EXIT_FAILURE);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }

                // Redirigir salida estándar al extremo de escritura del pipe
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);

                tcommand comando1 = line->commands[0];
                execvp(comando1.filename, comando1.argv);
                perror("Error al ejecutar el primer mandato");
                exit(EXIT_FAILURE);
            }

            // Segundo hijo (mandato 2)
            pid_t pid2 = fork();
            if (pid2 < 0) {
                perror("Error al crear el segundo proceso hijo");
                exit(EXIT_FAILURE);
            }

            if (pid2 == 0) {
                close(pipefd[1]); // Cerrar extremo de escritura del pipe

                // Redirigir entrada estándar al extremo de lectura del pipe
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);

                // Redirección de salida
                if (line->redirect_output != NULL) {
                    int fd = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd < 0) {
                        perror("Error al abrir el fichero de salida");
                        exit(EXIT_FAILURE);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }

                tcommand comando2 = line->commands[1];
                execvp(comando2.filename, comando2.argv);
                perror("Error al ejecutar el segundo mandato");
                exit(EXIT_FAILURE);
            }

            // Proceso padre
            close(pipefd[0]);
            close(pipefd[1]);
            waitpid(pid1, NULL, 0); // Esperar al primer hijo
            waitpid(pid2, NULL, 0); // Esperar al segundo hijo
        } else {
            fprintf(stderr, "Solo se permiten hasta 2 mandatos enlazados con '|'\n");
        }
    }

    return 0;
}
