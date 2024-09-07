#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Para saber que parte del pipe se cierra
#define READ 0
#define WRITE 1

// Funcion que me permite ejecutar comandos concurrentemenete, en donde a1 es el comando y a2 sirve para delimitar cuando tenemos un "|", y escribir otro coamando
void pipes(char **a1, char **a2)
{
    int f_des[2]; // Descriptores para el uso de pipes
    pid_t pid1, pid2;

    // Se podria crear una funcion para poder saber si se pudo realizar el pipe correctamente
    pipe(f_des);
    if (pid1 == 0 || pid2 == 0)
    {
        pid1 = fork();

        if (pid1 == 0)
        {
            close(f_des[READ]); // Cerramos la parte de lectura para el pipe
            dup2(f_des[WRITE], STDOUT_FILENO);
            close(f_des[WRITE]); // Cerramos la parte de escritura luego ya ocupado
            // Luego verificamos si es se pudo ejecutar el comando de entrada
            if (execvp(a1[0], a1) < 0)
            {
                printf("Error al ejecutar el comando \n");
                exit(1);
            }
        }
        else if (pid1 < 0)
        {
            printf("Error al crear el 1er hijo");
            exit(1);
        }
        close(f_des[WRITE]); // Cerraremos el descriptor del padre ya que no lo necesitamos
        pid2 = fork();

        if (pid2 == 0)
        {
            close(f_des[WRITE]); // Cerramos la parte de escritura para el pipe
            dup2(f_des[READ], STDIN_FILENO);
            close(f_des[READ]); // Cerramos la parte de lectura luego ya ocupado
            // Luego verificamos si es se pudo ejecutar el comando de entrada
            if (execvp(a2[0], a2) < 0)
            {
                printf("Error al ejecutar el comando \n");
                exit(1);
            }
        }
        else if (pid2 < 0)
        {
            printf("Error al crear el 2do hijo");
            exit(1);
        }
        close(f_des[READ]); // Cerraremos la parte de lectura para el pipe del padre
    }
    else
    {
        wait(NULL); // Para que se espere a que se termine el primer hijo
        wait(NULL); // Para que se espere a que se termine el segundo hijo
    }
}
// Creamos la funcion para mostrar el prompt y leer el comando que queremos
void visualizar_leer(char *comando)
{

    printf("shell_propia:~$ ");
    fflush(stdout);                          // Mediante esto podemos mostrar el siguiente prompt instantaneamente
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
    int estado;

    if (pid == 0) // Se puedo crear al proceso hijo
    {
        if (execvp(argumentos[0], argumentos) < 0)
        {
            printf("Error al ejecutar el comando \n");
        }
        exit(1);
    }
    else if (pid < 0) // No se pudo crear el proceso hijo
    {
        printf("No se pudo crear el proceso hijo\n");
        exit(1);
    }
    else
    {
        wait(&estado); // El proceso padre tiene que esperar al hijo para evitar procesos zombies
    }
}
int main()
{
    char comando[1024];
    char *argumentos[1024];
    int estado;
    while (1)
    {
        visualizar_leer(comando);
        if (strlen(comando) == 0)
        {
            continue;
        }
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
