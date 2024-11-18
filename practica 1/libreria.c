#include "libreria.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Funcion head que imprime n primeras lineas
int head(int entrada2) {
        int entrada = 0;
        char line[1024];
        int count = 0;
        
        //Si la entrada es nula se asigna 10 de forma predeterminada
        if(entrada2 == '\0'){
                entrada = -10;
        }
        //Si la entrada es negativa da un error, sino se asigna la entrada
        if(entrada2 > 0) {
                fprintf(stderr, "El numero introducido tiene que ser positivo\n");
        }else{
                entrada = entrada2;
        }
        entrada = entrada2 * -1;
 
    //Se leen e imprimen las n lineas
    while (count <= entrada-1 && fgets(line, sizeof(line), stdin) != NULL) { 
        printf("%s", line);
        count++;
    }
    return 0;
}

//Funcion tail que imprime las n ultimas lineas
int tail(int entrada2) {
int i = 0;
int j = 0;
int inicio = 0;
int numlineas = 0;
char line[1024];
char **memoria;
int cont = 0;
int entrada = 0;
        //Si la entrada es nula se asigna el valor 10
        if(entrada2 == '\0'){
                entrada = -10;
        }

        //Si la entrada es negativa da un error, sino se asigna la entrada
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

    //Reserva memoria para las lineas que se van a almacenar   
    memoria = malloc(entrada * sizeof(char *));
    if (memoria == NULL) {
        fprintf(stderr, "Error al reservar memoria para punteros\n");
        return -1;
    }

    //Reserva de memoria para cada una de las lineas   
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

    //Mientras que haya lineas disponibles en la entrada guarda la linea leida    
    while (fgets(line, sizeof(line), stdin) != NULL) {
        strcpy(memoria[cont % entrada], line);
        cont++;
    }

    //Calculo del indice para mostrar las lineas    
    inicio = cont > entrada ? cont % entrada : 0;
    numlineas = cont < entrada ? cont : entrada;

    //Imprime las n ultimas lineas de la entrada    
    for (i = 0; i < numlineas; i++) {
        printf("%s", memoria[(inicio + i) % entrada]);
    }

    //Se libera la memoria asignada    
    for (i = 0; i < entrada; i++) {
        free(memoria[i]);
    }
    free(memoria);

    return 0;
}

//Funcion longlines que imprime las n lineas mas largas en orden descendente
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

        //Si la entrada es nula se asigna 10
        if(entrada2 == '\0'){
                entrada = -10;
        }
        //Si la entrada es negativa da un error, sino se asigna la entrada
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
        //Reserva memoria para las lineas y para la longitud de cada una
        memoria = malloc(entrada * sizeof(char *));
        longitudes = malloc(entrada * sizeof(int));

    if (memoria == NULL || longitudes == NULL) {
        fprintf(stderr, "Error al reservar memoria para punteros");
        free(memoria);
        free(longitudes);
        return -1;
    }

    //Reserva de memoria para cada una de las lineas
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

        //Inicialización de las lineas vacias y longitudes a 0    
        memoria[i][0] = '\0';
        longitudes[i] = 0;
    }
        
    //Mientras que haya lineas disponibles en la entrada se procesa cada linea
    while (fgets(line, sizeof(line), stdin) != NULL) {
        len = strlen(line);

        minimo = 0;
        //Deteccion del indice de la linea mas corta    
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
        
    //Se ordenan las lineas almacenadas de mayor a menor longitud
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

    //Se imprimen las lineas    
    for (i = 0; i < entrada; i++) {
        if (longitudes[i] > 0) {
            printf("%s", memoria[i]);
        }
    }

    //Se libera la memoria asignada   
    for (i = 0; i < entrada; i++) {
        free(memoria[i]);
    }
    free(memoria);
    free(longitudes);
    return 0;
}
