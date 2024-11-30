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
                fprintf(stderr, "Error al crear el proceso hijo");
                exit(-1);
            }

            if (pid == 0) { // Proceso hijo
                // Redirección de entrada
                if (line->redirect_input != NULL) {
                    int fichero = open(line->redirect_input, O_RDONLY);
                    if (fichero < 0) {
                        fprintf(stderr, "Error al abrir el fichero de entrada");
                        exit(-1);
                    }
                    dup2(fichero, 0);
                    close(fichero);
                }

                // Redirección de salida
                if (line->redirect_output != NULL) {
                    int fichero_salida = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fichero_salida < 0) {
                        fprintf(stderr, "Error al abrir el fichero de salida");
                        exit(-1);
                    }
                    dup2(fichero_salida, 1);
                    close(fichero_salida);
                }

                // Ejecutar el mandato
                execvp(mandato.filename, mandato.argv);
                fprintf(stderr, "Error al ejecutar el mandato");
                exit(-1);
            } else {
                // Proceso padre
                int estado;
                waitpid(pid, &estado, 0); // Esperar por el hijo
            }
        } else {
            // Caso de múltiples mandatos enlazados con pipes
            int num_pipes = line->ncommands - 1; // Número de pipes necesarios
            int pipes[num_pipes][2];            // Array de pipes

            // Crear los pipes
            for (int i = 0; i < num_pipes; i++) {
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
            }

            // Esperar a todos los procesos hijos
            for (int i = 0; i < line->ncommands; i++) {
                wait(NULL);
            }
        }
    }

    return 0;
}
