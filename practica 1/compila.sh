#!/bin/bash

#control de parametros 
if [ $# -ge 2 ]
then
	echo "Demasiados parámetros, no hay que introducir ninguno";
	exit 1;
fi

#compilación de los archivos
	gcc libreria.c -c -Wall -Wextra -Werror -o libreria.o
	gcc test.c -c -Wall -Wextra -Werror -o test.o
	ar -rv libreria.a libreria.o
	gcc test.c libreria.a -Wall -Wextra -Werror -o test


