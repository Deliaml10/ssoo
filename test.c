#include <stdio.h>
#include "libreria.h"
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
	int entrada = -10;

//control de argumentos
	if (argc >2){
		char *end;
		long valor = strtol(argv[2], &end, 10);

		if(*end != '\0' || valor > 0) {
			printf("El numero introducido tiene que ser un entero positivo");
			return -1;
		}else{
			 entrada = (int)valor;
		}
	}else if(argc <= 2){
		entrada = -10;
	}


	if(strcmp(argv[1], "-head") == 0){
		head(entrada);
//	}else if(strcmp(argv[1], "-tail") == 0){
//		tail(entrada);
//	}else if(strcmp(argv[1], "-longlines") == 0){
//		longlines(atoi(argv[2]));
//	}else{
//		printf("Error en los argumentos");
		return -1;
	}

return 0;
}
