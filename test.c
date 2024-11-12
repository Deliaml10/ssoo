#include <stdio.h>
#include "libreria.h"
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
        int entrada = -10;
//control de argumentos
        if (argc == 3){
                //comprueba que el argumento sea entero y positivo 
                char *end;
                long valor = strtol(argv[2], &end, 10);

                if(*end != '\0' || valor > 0) {
                        printf("El numero introducido tiene que ser un entero positivo\n");
                        return -1;
                }else{
                         entrada = (int)valor;
                }
	}else if(argc >= 4){
		printf("Demasiados parametros\n");
		return -1;
        }else if(argc == 2){
                entrada = -10;
        }else if(argc == 1){
		printf("No has introducido parametros\n");
		return -1;
	}
//una vez comprobado los argumentos, se llama a la función correspondiente
        if(strcmp(argv[1], "-head") == 0){
                head(entrada);
        }else if(strcmp(argv[1], "-tail") == 0){
                tail(entrada);
        }else if(strcmp(argv[1], "-longlines") == 0){
                longlines(entrada);
        }else{
                printf("Error en los argumentos\n");
                return -1;
        }

return 0;
}
