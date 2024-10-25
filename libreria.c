#include "libreria.h"
#include <stdio.h>
#include <stdlib.h>


int head(int entrada){
	char line[1024];

	if(entrada < 0){
		printf("El numero tiene que ser positivo");
		return -1;
	}

	if (entrada == NULL){
		int numero = 10;
	}else if(entrada != NULL ){
			char line[1024];
	int count = 0;

	while(fgets(line, sizeof(line), stdin) != NULL && count < entrada) {
		printf("%s", line);
		count++;
	}
	return 0;
}

