  GNU nano 7.2                                                                                                                                                                                       libreria.c *                                                                                                                                                                                               
#include "libreria.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int head(int entrada2) {
        int entrada = 0;
        if(entrada2 == '\0'){
                entrada = -10;
        }

        if(entrada2 > 0) {
                fprintf(stderr, "El numero introducido tiene que ser positivo\n");
        }else{
                entrada = entrada2;
        }
        entrada = entrada2 * -1;


    char line[1024];
    int count = 0;

    while (count <= entrada-1 && fgets(line, sizeof(line), stdin) != NULL) {
        printf("%s", line);
        count++;
    }
    return 0;
}

int tail(int entrada2) {
int i = 0;
int j = 0;
int inicio = 0;
int numlineas = 0;
char line[1024];
char **memoria;
int cont = 0;
int entrada = 0;
        if(entrada2 == '\0'){
                entrada = -10;
        }

        if(entrada2 > 0) {
                fprintf(stderr, "El numero introducido tiene que ser positivo\n");
        }else{
                entrada = entrada2;
        }
        entrada = entrada2 * -1;

    if (entrada <= 0) {
        fprintf(stderr, "Error: El número de líneas debe ser mayor que 0.\n");
        return -1;
    }

    memoria = malloc(entrada * sizeof(char *));
    if (memoria == NULL) {
        fprintf(stderr, "Error al reservar memoria para punteros\n");
        return -1;
    }

    for (i = 0; i < entrada; i++) {
        memoria[i] = malloc(1024 * sizeof(char));
        if (memoria[i] == NULL) {
            fprintf(stderr, "Error al reservar memoria para línea\n");

            for (j = 0; j < i; j++) {
                free(memoria[j]);
            }
            free(memoria);
            return -1;
        }
    }

    while (fgets(line, sizeof(line), stdin) != NULL) {
        strcpy(memoria[cont % entrada], line);
        cont++;
    }

    inicio = cont > entrada ? cont % entrada : 0;
    numlineas = cont < entrada ? cont : entrada;

    for (i = 0; i < numlineas; i++) {
        printf("%s", memoria[(inicio + i) % entrada]);
    }

    for (i = 0; i < entrada; i++) {
        free(memoria[i]);
    }
    free(memoria);

    return 0;
}
int longlines(int entrada2) {
int i = 0;
int j = 0;
char line[1024];
int count = 0;
char **memoria;
int *longitudes; 
int len = 0;
int minimo = 0;
int aux = 0;
char *temporal;
int entrada = 0;
        if(entrada2 == '\0'){
                entrada = -10;
        }

        if(entrada2 > 0) {
                fprintf(stderr, "El numero introducido tiene que ser positivo\n");
        }else{
                entrada = entrada2;
        }
        entrada = entrada2 * -1;

    if (entrada <= 0) {
        fprintf(stderr, "Error: El número de líneas debe ser mayor que 0.\n");
        return -1;
    }

        memoria = malloc(entrada * sizeof(char *));
        longitudes = malloc(entrada * sizeof(int));

    if (memoria == NULL || longitudes == NULL) {
        fprintf(stderr, "Error al reservar memoria para punteros");
        free(memoria);
        free(longitudes);
        return -1;
    }

    for (i = 0; i < entrada; i++) {
        memoria[i] = malloc(1024 * sizeof(char));
        if (memoria[i] == NULL) {
            fprintf(stderr, "Error al reservar memoria para línea");

            for (j = 0; j < i; j++) {
                free(memoria[j]);
            }
            free(memoria);
            free(longitudes);
            return -1;
        }
        memoria[i][0] = '\0';
        longitudes[i] = 0;
    }

    while (fgets(line, sizeof(line), stdin) != NULL) {
        len = strlen(line);

        minimo = 0;
        for (i = 1; i < entrada; i++) {
            if (longitudes[i] <= longitudes[minimo]) {
                minimo = i;
            }
        }

        if (len > longitudes[minimo]) {
            strcpy(memoria[minimo], line);
            longitudes[minimo] = len;
        }

        count++;
    }

    for (i = 0; i < entrada - 1; i++) {
        for (j = 0; j < entrada - i - 1; j++) {
            if (longitudes[j] < longitudes[j + 1]) {
                aux = longitudes[j];
                longitudes[j] = longitudes[j + 1];
                longitudes[j + 1] = aux;
                temporal = memoria[j];
                memoria[j] = memoria[j + 1];
                memoria[j + 1] = temporal;
            }
        }
    }

    for (i = 0; i < entrada; i++) {
        if (longitudes[i] > 0) {
            printf("%s", memoria[i]);
        }
    }
    for (i = 0; i < entrada; i++) {
        free(memoria[i]);
    }
    free(memoria);
    free(longitudes);
    return 0;
}
