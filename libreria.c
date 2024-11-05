#include "libreria.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int head(int entrada) {
    char line[1024];
    int count = 0;

    while (count >= entrada+1 && fgets(line, sizeof(line), stdin) != NULL) {
        printf("%s", line);
        count--;
    }
    return 0;
}
/*
int tail(int entrada) {
    char line[1024];
    int count = 0;
    char **memoria = malloc(entrada * sizeof(char *));

    if (memoria == NULL) {
        perror("Error al reservar memoria");
        return -1;
    }

    for (int i = 0; i < entrada; i++) {
        memoria[i] = NULL;
    }

    while (fgets(line, sizeof(line), stdin) != NULL) {
        if (memoria[count % entrada] != NULL) {
            free(memoria[count % entrada]);
        }
        // Reserva memoria para la nueva línea y copia su contenido
        memoria[count % entrada] = strdup(line);
        if (memoria[count % entrada] == NULL) {
            perror("Error al copiar la línea");
            for (int i = 0; i < entrada; i++) {
                free(memoria[i]);
            }
            free(memoria);
            return -1;
        }
        count++;
    }

    int start = count > entrada ? count % entrada : 0;
    int numlineas = count < entrada ? count : entrada;

    for (int i = 0; i < numlineas; i++) {
        printf("%s", memoria[(start + i) % entrada]);
    }

    // Libera la memoria reservada
    for (int i = 0; i < entrada; i++) {
        free(memoria[i]);
    }
    free(memoria);

    return 0;
}

int longlines(int entrada) {
    char* linea;
    char** lineas_largas;
    int* longitudes;
    int contador = 0;

    lineas_largas = malloc(entrada * sizeof(char*));
    longitudes = malloc(entrada * sizeof(int));
    if (lineas_largas == NULL || longitudes == NULL) {
        return 1;
    }

    for (int i = 0; i < entrada; i++) {
        lineas_largas[i] = NULL;
        longitudes[i] = 0;
    }

    while (fgets(linea, sizeof(linea), stdin) != NULL) {
        int longitud = strlen(linea);
        if (contador < entrada || longitud > longitudes[entrada - 1]) {
            int pos = contador < entrada ? contador : entrada - 1;
            while (pos > 0 && longitud > longitudes[pos - 1]) {
                lineas_largas[pos] = lineas_largas[pos - 1];
                longitudes[pos] = longitudes[pos - 1];
                pos--;
            }
            if (contador >= entrada) {
                free(lineas_largas[pos]);
            }
            lineas_largas[pos] = linea;
            longitudes[pos] = longitud;
            if (contador < entrada) {
                contador++;
            }
        } else {
            free(linea);
        }
    }

    for (int i = 0; i < contador; i++) {
        printf("%s", lineas_largas[i]);
        free(lineas_largas[i]);
    }

    free(lineas_largas);
    free(longitudes);

    return 0;
} 
*/
