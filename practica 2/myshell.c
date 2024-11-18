#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
char input[1024];
while(1){ //while para que el programa sea un bucle
	//prompt que se muestra automáticamente
	printf("msh> ");
	fflush(stdout);

	if (fgets(input, sizeof(input), stdin) == NULL){
		printf("msh> ");
		fflush(stdout);
	}// fin del if
} // fin del while
return 0;
}
