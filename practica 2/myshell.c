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

int main() {
    char input[1024];  // HAY QUE HACERLO CON MALLOC
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
        if (fgets(input, sizeof(input), stdin) == NULL) { // Si solo se hace Enter
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
                perror("Error al crear el proceso hijo");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) { // Proceso hijo
                // Redirección de entrada
                if (line->redirect_input != NULL) {
                    int fichero = open(line->redirect_input, O_RDONLY);
                    if (fichero < 0) {
                        perror("Error al abrir el fichero de entrada");
                        exit(-1);
                    }
                    dup2(fichero, STDIN_FILENO);
                    close(fichero);
                }

                // Redirección de salida
                if (line->redirect_output != NULL) {
                    int fichero_salida = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fichero_salida < 0) {
                        perror("Error al abrir el fichero de salida");
                        exit(-1);
                    }
                    dup2(fichero_salida, STDOUT_FILENO);
                    close(fichero_salida);
                }

                // Ejecutar el mandato
                execvp(mandato.filename, mandato.argv);
                perror("Error al ejecutar el mandato");
                exit(-1);
            } else {
                // Proceso padre
                int estado;
                waitpid(pid, &estado, 0); // Esperar por el hijo
            }
        } else if (line->ncommands == 2) {
            // Caso de dos mandatos enlazados con pipe
            int tuberia[2];
            if (pipe(tuberia) < 0) {
                perror("Error al crear el pipe");
                exit(-1);
            }

            pid_t pid1 = fork();
            if (pid1 == 0) { // Primer hijo (comando 1)
                close(tuberia[0]); // Cerrar extremo de lectura

                // Redirección de entrada
                if (line->redirect_input != NULL) {
                    int fichero = open(line->redirect_input, O_RDONLY);
                    if (fichero < 0) {
                        perror("Error al abrir el fichero de entrada");
                        exit(-1);
                    }
                    dup2(fichero, STDIN_FILENO);
                    close(fichero);
                }

                // Redirigir salida estándar al extremo de escritura del pipe
                dup2(tuberia[1], STDOUT_FILENO);
                close(tuberia[1]);

                tcommand comando1 = line->commands[0];
                execvp(comando1.filename, comando1.argv);
                perror("Error al ejecutar el primer mandato");
                exit(-1);
            }

            pid_t pid2 = fork();
            if (pid2 == 0) { // Segundo hijo (comando 2)
                close(tuberia[1]); // Cerrar extremo de escritura

                // Redirigir entrada estándar al extremo de lectura del pipe
                dup2(tuberia[0], STDIN_FILENO);
                close(tuberia[0]);

                // Redirección de salida
                if (line->redirect_output != NULL) {
                    int salida = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (salida < 0) {
                        perror("Error al abrir el fichero de salida");
                        exit(-1);
                    }
                    dup2(salida, STDOUT_FILENO);
                    close(salida);
                }

                tcommand comando2 = line->commands[1];
                execvp(comando2.filename, comando2.argv);
                perror("Error al ejecutar el segundo mandato");
                exit(-1);
            }

            // Proceso padre
            close(tuberia[0]);
            close(tuberia[1]);
            waitpid(pid1, NULL, 0); // Esperar al primer hijo
            waitpid(pid2, NULL, 0); // Esperar al segundo hijo
        } else {
            printf("Solo se permiten 2 mandatos enlazados\n");
        }
    }

    return 0;
}
