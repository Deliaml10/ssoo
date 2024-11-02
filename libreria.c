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
		if(memoria[count %(-1 * entrada)] == NULL){


//Hay que copiar la linea en el puntero memoria con strcopy
			for(int i = 0; i < entrada; i++){
				free(memoria[i]);
			}
			free(memoria);
		return -1;
		}
	count++;
	}

	for(int i = 0; i < entrada; i++){
		printf("%s", memoria[i]);
	}

	for(int i = 0; i < entrada; i++){
		free(memoria[i]);
	}
	free(memoria);
	return 0;
}
