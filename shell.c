#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Creamos la funcion para mostrar el prompt y leer el comando que queremos
void visualizar_leer(char *comando)
{

    printf("shell_propia:~$ ");
    //  fflush(stdout);                          // Mediante esto podemos mostrar el siguiente prompt instantaneamente
    if (fgets(comando, 1024, stdin) == NULL) // Si el comando que se escribe es nulo genera error
    {
        perror("No existe ese comando");
        exit(1);
    }
    comando[strcspn(comando, "\n")] = '\0'; // Con esto podemos eliminar el salto de linea
}
void parsear(char *comando, char **argumentos)
{
    char *token = strtok(comando, " "); // Para tomar cada fragmento del comando queremos ingresar y los separamos en base un " "
    int i = 0;
    // A continuacion lo que hacemos es que por cada fragmento del comando, lo vamos a ir ingresando dentro del arreglo de argumentos
    do
    {
        argumentos[i++] = token;
        token = strtok(NULL, " "); // Con esto me permite obtener los fragmentos siguientes (tokens), y que con esto me permite seguir almancenando mis fragmentos
    } while (token != NULL);
    if (sizeof(argumentos) == i)
        argumentos[i] = NULL; // Cuando no se siga escribiendo un comando terminamos con un NULL
}

// Para visualizar los argumentos que hay dentro del arreglo
//  void imprimir_argumentos(char **argumentos)
//  {
//      int i = 0;
//      while (argumentos[i] != NULL)
//      {
//          printf("Argumento %d: %s\n", i, argumentos[i]);
//          i++;
//      }
//  }
void ejecuta_comando(char **argumentos)
{
    pid_t pid = fork();
    int espera;

    if (pid == 0) // Se puedo crear al proceso hijo
    {
        if (execvp(argumentos[0], argumentos) < 0)
        {
            printf("Error al ejecutar el comando \n");
        }
        exit(1);
    }
    else if (pid < 0) // No se pudo crear
    {
        printf("No se pudo crear el proceso hijo\n");
        exit(1);
    }
    else
    {
        wait(&espera);
    }
}
int main()
{
    char comando[1024];
    char *argumentos[1024];
    while (1)
    {
        visualizar_leer(comando);
        // Para salir de la shell
        if (strcmp(comando, "exit") == 0)
        {
            break;
        }

        parsear(comando, argumentos);

        // imprimir_argumentos(argumentos);
        ejecuta_comando(argumentos);
    }

    return 0;
}
