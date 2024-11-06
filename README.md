Aquí tienes un listado de los casos de prueba junto con una breve explicación para cada uno:

1. **Ejecución básica del script `./compila.sh`**: Comprueba que el archivo de script de compilación funcione correctamente sin errores y genere los archivos de salida esperados.

2. **`./test -head`**: Muestra todas las líneas de entrada en orden ascendente para verificar la funcionalidad de lectura y salida de datos básicos con el comando `-head`.

3. **`./test -head -3`**: Muestra solo las primeras 3 líneas de la entrada, probando que el programa respete la limitación de líneas cuando se especifica un número negativo.

4. **`./test -head --3` y `./test -head 3`**: Evalúa el manejo de errores al introducir parámetros incorrectos (números sin signo negativo), verificando que el programa responda con un mensaje de error si el número no es un entero positivo.

5. **`./test -head -3 h` y `./test -head --3 h`**: Verifica la detección de demasiados parámetros y asegura que el programa avise de forma adecuada cuando hay un exceso de argumentos.

6. **`./test -head < hola.txt`**: Prueba la lectura desde un archivo de texto con el comando `-head` para asegurar que el programa lee correctamente desde fuentes externas.

7. **`./test -head -3 < hola.txt`**: Prueba la lectura de las primeras tres líneas desde un archivo externo, validando la limitación de líneas especificada y la lectura externa.

8. **`./test -tail`**: Prueba la funcionalidad básica del comando `-tail` para listar líneas finales desde la entrada, asegurando el orden correcto en la salida.

9. **`./test -tail -3`**: Muestra las últimas 3 líneas de la entrada para verificar la función de limitación de líneas en la salida final.

10. **`./test -tail --3` y `./test -tail 3`**: Evalúa el manejo de errores con parámetros incorrectos en el comando `-tail`, probando el mensaje de error ante entradas no permitidas.

11. **`./test -tail -3 < hola.txt`**: Lee las últimas tres líneas de un archivo de texto para validar la lectura externa y la funcionalidad de límite de líneas.

12. **`./test -longlines`**: Ejecuta el comando `-longlines` sin parámetros, probando que se muestren las líneas largas de entrada en orden.

13. **`./test -longlines -3`**: Valida la funcionalidad de `-longlines` limitando a 3 líneas largas, probando que respete la cantidad especificada en la salida.

14. **`./test -longlines --3` y `./test -longlines 3`**: Comprueba el manejo de errores en el comando `-longlines` con números no válidos (números positivos) y asegura que se despliegue el mensaje adecuado.

15. **`./test -longlines -5 < hola.txt`**: Prueba el comando `-longlines` con un límite de 5 líneas largas desde un archivo externo, asegurando que se respeten las entradas de archivo y el límite especificado.

16. **Casos adicionales (`p2.txt`) con `./test -longlines -7`, `-8`, y `-9`**: Comprueban la función `-longlines` con varios límites de líneas largas desde un archivo de texto. Se asegura que el comando lee y muestra correctamente según el límite solicitado, especialmente con diferentes números de líneas en el archivo.

Cada caso de prueba asegura la cobertura funcional y de manejo de errores, validando que el programa se comporte de acuerdo con las especificaciones y que detecte entradas o parámetros inválidos con mensajes adecuados.
