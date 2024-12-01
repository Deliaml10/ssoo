#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

// Estructura para guardar información de trabajos
typedef struct {
    int id;         // ID del trabajo
    pid_t pgid;     // PID del grupo de procesos
    char *command;  // Línea de comando
    char status[16];// Estado: "Running" o "Stopped"
} Job;

Job *jobs = NULL;  // Lista dinámica de trabajos
int jobCount = 0;  // Número de trabajos

// Manejo de las señales SIGINT y SIGTSTP
void handleCtrlC() {
    printf("\nmsh> ");
    fflush(stdout);
}

// Agregar un nuevo trabajo a la lista
void addJob(pid_t pgid, char *command, char *status) {
    jobs = realloc(jobs, sizeof(Job) * (jobCount + 1));
    jobs[jobCount].id = jobCount + 1;
    jobs[jobCount].pgid = pgid;
    jobs[jobCount].command = strdup(command);
    strncpy(jobs[jobCount].status, status, sizeof(jobs[jobCount].status) - 1);
    jobs[jobCount].status[sizeof(jobs[jobCount].status) - 1] = '\0';
    jobCount++;
}

void handleCtrlZ() {
    pid_t fgPgid = tcgetpgrp(STDIN_FILENO); // Obtener el grupo de procesos en foreground
    if (fgPgid != getpgid(0)) { // Si hay un proceso en foreground
        kill(-fgPgid, SIGTSTP); // Enviar señal SIGTSTP al grupo de procesos
        addJob(fgPgid, "Proceso detenido con CTRL+Z", "Stopped"); // Registrar el trabajo detenido
        printf("\n[Job Detenido] PID: %d\n", fgPgid);
        tcsetpgrp(STDIN_FILENO, getpgid(0)); // Restaurar control del terminal al shell
    }
    printf("\nmsh> ");
    fflush(stdout);
}

// Mostrar la lista de trabajos
void showJobs() {
    for (int i = 0; i < jobCount; i++) {
        printf("[%d] %s %s\n", jobs[i].id, jobs[i].status, jobs[i].command);
    }
}

// Buscar el último trabajo detenido
int findStoppedJob() {
    for (int i = jobCount - 1; i >= 0; i--) {
        if (strcmp(jobs[i].status, "Stopped") == 0) {
            return jobs[i].id;
        }
    }
    return -1;
}

// Reanudar un trabajo en background
void resumeJob(int jobId) {
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].id == jobId && strcmp(jobs[i].status, "Stopped") == 0) {
            kill(-jobs[i].pgid, SIGCONT); // Reanudar el grupo de procesos detenido
            strncpy(jobs[i].status, "Running", sizeof(jobs[i].status) - 1);
            jobs[i].status[sizeof(jobs[i].status) - 1] = '\0'; // Marcar como "Running"
            printf("[%d] %s\n", jobs[i].id, jobs[i].command);
            return;
        }
    }
    printf("No se encontró el trabajo %d o no está detenido.\n", jobId);
}

int main() {
    char input[1024];
    tline *line;

    // Configurar las señales SIGINT y SIGTSTP
    if (signal(SIGINT, handleCtrlC) == SIG_ERR) {
        fprintf(stderr, "Error al configurar SIGINT\n");
        exit(1);
    }
    if (signal(SIGTSTP, handleCtrlZ) == SIG_ERR) {
        fprintf(stderr, "Error al configurar SIGTSTP\n");
        exit(1);
    }

    while (1) {
        // Mostrar el prompt
        printf("msh> ");
        fflush(stdout);

        // Leer entrada del usuario
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }

        // Tokenizar la entrada
        line = tokenize(input);
        if (line == NULL || line->ncommands == 0) {
            continue;
        }

        // Comando interno "exit"
        if (strcmp(line->commands[0].argv[0], "exit") == 0) {
            printf("Se ha terminado el programa, hasta luego!\n");
            break;
        }

        // Comando interno "jobs"
        if (strcmp(line->commands[0].argv[0], "jobs") == 0) {
            showJobs();
            continue;
        }

        // Comando interno "bg"
        if (strcmp(line->commands[0].argv[0], "bg") == 0) {
            int jobId = -1;
            if (line->commands[0].argc > 1) {
                jobId = atoi(line->commands[0].argv[1]);
            } else {
                jobId = findStoppedJob();
            }

            if (jobId > 0) {
                resumeJob(jobId);
            } else {
                printf("No hay trabajos detenidos.\n");
            }
            continue;
        }

        // Determinar si es en background
        int isBackground = 0;
        if (line->background) {
            isBackground = 1;
        }

        // Manejar múltiples mandatos con pipes
        int numPipes = line->ncommands - 1;
        int **pipes = malloc(numPipes * sizeof(int *));
        for (int i = 0; i < numPipes; i++) {
            pipes[i] = malloc(2 * sizeof(int));
            if (pipe(pipes[i]) < 0) {
                perror("Error al crear el pipe");
                exit(1);
            }
        }

        for (int i = 0; i < line->ncommands; i++) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("Error al crear el proceso hijo");
                exit(1);
            }

            if (pid == 0) { // Proceso hijo
                setpgid(0, 0); // Establecer un nuevo grupo de procesos

                // Redirección de entrada
                if (i == 0 && line->redirect_input != NULL) {
                    int inFile = open(line->redirect_input, O_RDONLY);
                    if (inFile < 0) {
                        perror("Error al abrir el archivo de entrada");
                        exit(1);
                    }
                    dup2(inFile, 0);
                    close(inFile);
                } else if (i > 0) {
                    dup2(pipes[i - 1][0], 0);
                }

                // Redirección de salida
                if (i == line->ncommands - 1 && line->redirect_output != NULL) {
                    int outFile = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (outFile < 0) {
                        perror("Error al abrir el archivo de salida");
                        exit(1);
                    }
                    dup2(outFile, 1);
                    close(outFile);
                } else if (i < numPipes) {
                    dup2(pipes[i][1], 1);
                }

                // Cerrar todos los pipes
                for (int j = 0; j < numPipes; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                tcommand cmd = line->commands[i];
                execvp(cmd.filename, cmd.argv);
                perror("Error al ejecutar el comando");
                exit(1);
            } else { // Proceso padre
                setpgid(pid, pid);
                if (i > 0) {
                    close(pipes[i - 1][0]);
                }
                if (i < numPipes) {
                    close(pipes[i][1]);
                }

                if (i == 0 && isBackground) {
                    addJob(pid, input, "Running");
                }
            }
        }

        // Liberar memoria de pipes
        for (int i = 0; i < numPipes; i++) {
            free(pipes[i]);
        }
        free(pipes);

        if (!isBackground) {
            for (int i = 0; i < line->ncommands; i++) {
                wait(NULL);
            }
        }
    }

    // Liberar memoria de trabajos
    for (int i = 0; i < jobCount; i++) {
        free(jobs[i].command);
    }
    free(jobs);

    return 0;
}
