# Proyecto-1-SO
Repositorio para SO

Este es nuestra minishell, en donde se implementa y se hace el uso de pipes y una funcion personalizada llamada favs.

Dentro de la minishell, se puede ver el prompt en donde uno puede escribir los comandos que quiera y que sean validos, en caso de no ser un comando arrojará un error, si no se escribe nada estará esperando por una entrada y si se presiona la tecla "enter", realizará un salto de línea.

# Instrucciones de Configuración y Compilación

## Pre-requisitos

Asegúrate de tener instalados los siguientes componentes en tu sistema antes de compilar el proyecto:

- **gcc**: El compilador de C/C++. Puedes instalarlo en Ubuntu con el siguiente comando:

    ```bash
    sudo apt-get install gcc
    ```

## Compilación

Ingresar la siguiente línea de código en el directorio donde se encuentre el archivo:


    ```bash
    gcc -o minishell minishell.c -pethreads
    ```

Una vez completados estos pasos, se generará 1 ejecutable:

El ejecutable minishell, el cual contiene el intérprete de comandos. Se puede ejecutar usando:

    ```bash
    ./minishell}]]
    ```

Los comandos disponibles y más información se puede encontrar en el PDF adjunto.
