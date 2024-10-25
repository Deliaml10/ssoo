#include <stdio.h>
#include "libreria.h"

int main(int argc, char *argv[]){
	int entrada;

	if (argc < 2){
		int entrada = 10;
	}else{
		int entrada = atoi(argv[2]);
	}



	if(argv[1] == "-head"){
		head(entrada);
	}else if(argv[1] == "-tail"){
		tail(entrada);
	}else if(argv[1] == "-longlines"){
		longlines(atoi(argv[2]));
	}else{
		printf("Error en los argumentos");
		return -1;
	}

return 0;
}
