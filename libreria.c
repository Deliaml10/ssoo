#include "libreria.h"
#include <stdio.h>
#include <stdlib.h>


int head(int entrada){
	char line[1024];
	int count = 0;

	while(fgets(line, sizeof(line), stdin) != NULL && count > entrada) {
		printf("%s", line);
		count--;
	}
	return 0;
}

int tail(int entrada){
	char line[1024];
	int count = 0;
	char **memoria = malloc(entrada * sizeof(char *));

	while(fgets(line, sizeof(line), stdin) != NULL){
		if(memoria == NULL){
			perror("Error al reservar memoria");
			return -1;
		}

//Hay que copiar la linea en el puntero memoria con strcopy
		for(int i = 0; i < entrada; i++){
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
