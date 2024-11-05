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

    char line[1024];
    int count = 0;

    char **memoria = malloc(entrada * sizeof(char *));
    if (memoria == NULL) {
        fprintf(stderr, "Error al reservar memoria para punteros\n");
        return -1;
    }

    for (int i = 0; i < entrada; i++) {
        memoria[i] = malloc(1024 * sizeof(char));
        if (memoria[i] == NULL) {
            fprintf(stderr, "Error al reservar memoria para línea\n");

            for (int j = 0; j < i; j++) {
                free(memoria[j]);
            }
            free(memoria);
            return -1;
        }
    }

    while (fgets(line, sizeof(line), stdin) != NULL) {
        strcpy(memoria[count % entrada], line);
        count++;
    }

    int start = count > entrada ? count % entrada : 0;
    int numlineas = count < entrada ? count : entrada;

    for (int i = 0; i < numlineas; i++) {
        printf("%s", memoria[(start + i) % entrada]);
    }

    for (int i = 0; i < entrada; i++) {
        free(memoria[i]);
    }
    free(memoria);

    return 0;
}



int longlines(int entrada2) {

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

    char line[1024];
    int count = 0;

    char **memoria = malloc(entrada * sizeof(char *));
    int *longitudes = malloc(entrada * sizeof(int));
    if (memoria == NULL || longitudes == NULL) {
        fprintf(stderr, "Error al reservar memoria para punteros");
        free(memoria);
        free(longitudes);
        return -1;
    }

    for (int i = 0; i < entrada; i++) {
        memoria[i] = malloc(1024 * sizeof(char));
        if (memoria[i] == NULL) {
            fprintf(stderr, "Error al reservar memoria para línea");

            for (int j = 0; j < i; j++) {
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
        int len = strlen(line);

        int min_index = 0;
        for (int i = 1; i < entrada; i++) {
            if (longitudes[i] < longitudes[min_index]) {
                min_index = i;
            }
        }

        if (len > longitudes[min_index]) {
            strcpy(memoria[min_index], line);
            longitudes[min_index] = len;
        }

        count++;
    }

    for (int i = 0; i < entrada - 1; i++) {
        for (int j = 0; j < entrada - i - 1; j++) {
            if (longitudes[j] < longitudes[j + 1]) {
                int temp_len = longitudes[j];
                longitudes[j] = longitudes[j + 1];
                longitudes[j + 1] = temp_len;

                char *temp_line = memoria[j];
                memoria[j] = memoria[j + 1];
                memoria[j + 1] = temp_line;
            }
        }
    }

    for (int i = 0; i < entrada; i++) {
        if (longitudes[i] > 0) {
            printf("%s", memoria[i]);
        }
    }

    for (int i = 0; i < entrada; i++) {
        free(memoria[i]);
    }
    free(memoria);
    free(longitudes);

    return 0;
}
