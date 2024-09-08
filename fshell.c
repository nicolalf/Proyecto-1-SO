#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Para saber que parte del pipe se cierra
#define READ 0
#define WRITE 1

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
// Vamos parseando el comando que escribamos a medida y se separa cuando vaya encontrando un " "
int parsear(char *comando, char **argumentos)
{
    char *token = strtok(comando, " "); // Para tomar cada fragmento del comando queremos ingresar y los separamos en base un " "
    int i = 0;
    // A continuacion lo que hacemos es que por cada fragmento del comando, lo vamos a ir ingresando dentro del arreglo de argumentos
    while (token != NULL)
    {
        argumentos[i++] = token;
        token = strtok(NULL, " "); // Con esto me permite obtener los fragmentos siguientes (tokens), y que con esto me permite seguir almancenando mis fragmentos
    }

    argumentos[i] = NULL;
}

// Si es que pasamos a encontrar un pipe, separamos y parseamos el comando que escribamos cuando encontremos un '|'
void parsear_p(char *entrada, char **comandos)
{
    char *token = strtok(entrada, "|"); // Para tomar cada fragmento del comando queremos ingresar y los separamos en base un " "
    int i = 0;
    // A continuacion lo que hacemos es que por cada fragmento del comando, lo vamos a ir ingresando dentro del arreglo de argumentos
    while (token != NULL)
    {
        comandos[i++] = token;
        token = strtok(NULL, "|"); // Con esto me permite obtener los fragmentos siguientes (tokens), y que con esto me permite seguir almancenando mis fragmentos
    }

    comandos[i] = NULL;
}

void pipes(char **comandos)
{
    int cant_comandos = 0;
    do
    {
        cant_comandos++;
    } while (comandos[cant_comandos] != NULL);
    // Contamos la cantidad de descriptores en base a que si tenemos 3 comandos, se tienen 2 pipes, por lo que se generan 2 espacios por pipe, que corresponden a lectura escritura
    int descr[2 * (cant_comandos - 1)];
    // Realizamos un for para ver la cantidad de pipes que se necesitan
    for (int i = 0; i < cant_comandos - 1; i++)
    {
        pipe(descr + i * 2);
    }
    // pid_t pid1, pid2, pid3;

    // Se podria crear una funcion para poder saber si se pudo realizar el pipe correctamente
    // pipe(f_des1);
    // pipe(f_des2);
    for (int i = 0; i < cant_comandos; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // Redirigimos la entrada en caso de no ser el primer coamando que escribimos
            if (i > 0)
            {
                // Se da cuando tenemos que leer un comando en el pipe
                dup2(descr[(i - 1) * 2 + READ], STDIN_FILENO);
            }
            // En este caso la salida si no tenemos el ultimo comando
            if (i < cant_comandos - 1)
            {
                // Para este caso cuando queremos escribir en el pipe
                dup2(descr[i * 2 + WRITE], STDOUT_FILENO);
            }
            // Cerramos los descriptores de los pipes que pedimos
            for (int j = 0; j < 2 * (cant_comandos - 1); j++)
            {
                close(descr[j]);
            }
            // Parseamos el comando que queremos para despues ejecutarlo
            char *a[1024];
            parsear(comandos[i], a);
            if (execvp(a[0], a) < 0)
            {
                printf("Error al ejecutar el comando \n");
                exit(1);
            }
        }
        else if (pid < 0)
        {
            printf("Error al crear el hijo hijo");
            exit(1);
        }
    }
    for (int i = 0; i < 2 * (cant_comandos - 1); i++)
    {
        close(descr[i]);
    }
    for (int i = 0; i < cant_comandos; i++)
    {
        wait(NULL);
    }

    // pid1 = fork();

    // if (pid1 == 0)
    // {
    //     close(f_des1[READ]); // Cerramos la parte de lectura para el pipe
    //     dup2(f_des1[WRITE], STDOUT_FILENO);
    //     close(f_des1[WRITE]); // Cerramos la parte de escritura luego ya ocupado
    //     // Luego verificamos si es se pudo ejecutar el comando de entrada
    //     if (execvp(a1[0], a1) < 0)
    //     {
    //         printf("Error al ejecutar el comando \n");
    //         exit(1);
    //     }
    // }
    // else if (pid1 < 0)
    // {
    //     printf("Error al crear el 1er hijo");
    //     exit(1);
    // }

    // pid2 = fork();

    // if (pid2 == 0)
    // {
    //     close(f_des1[WRITE]); // Cerramos la parte de escritura para el pipe
    //     dup2(f_des1[READ], STDIN_FILENO);
    //     close(f_des1[READ]); // Cerramos la parte de lectura luego ya ocupado

    //     close(f_des2[READ]);                // Cerramos la parte de lectura del 2do pipe
    //     dup2(f_des2[WRITE], STDOUT_FILENO); // Escribimos
    //     close(f_des2[WRITE]);               // Cerramos la parte de escritura del 2do pipe

    //     // Luego verificamos si es se pudo ejecutar el comando de entrada
    //     if (execvp(a2[0], a2) < 0)
    //     {
    //         printf("Error al ejecutar el comando \n");
    //         exit(1);
    //     }
    // }

    // else if (pid2 < 0)
    // {
    //     printf("Error al crear el 2do hijo");
    //     exit(1);
    // }

    // pid3 = fork();
    // if (pid3 == 0)
    // {
    //     // Cerramos el 1er descriptor ya que no lo ocupamos
    //     close(f_des1[WRITE]);
    //     close(f_des1[READ]);

    //     close(f_des2[WRITE]);             // Cerramos la parte de escritura del 3er pipe
    //     dup2(f_des2[READ], STDIN_FILENO); // Leemos
    //     close(f_des2[READ]);              // Cerramos la parte de lectura del 3er pipe

    //     // Luego verificamos si es se pudo ejecutar el comando de entrada
    //     if (execvp(a3[0], a3) < 0)
    //     {
    //         printf("Error al ejecutar el comando \n");
    //         exit(1);
    //     }
    // }
    // else if (pid3 < 0)
    // {
    //     printf("Error al crear el 2do hijo");
    //     exit(1);
    // }
    // close(f_des1[READ]);  // Cerraremos el descriptor del padre ya que no lo necesitamos
    // close(f_des1[WRITE]); // Cerraremos la parte de lectura para el pipe del padre
    // close(f_des2[READ]);  // Cerraremos el descriptor del padre ya que no lo necesitamos
    // close(f_des2[WRITE]); // Cerraremos la parte de lectura para el pipe del padre

    // if (a3 != NULL)
    // {
    //     waitpid(pid1, NULL, 0); // Para que se espere a que se termine el primer hijo
    //     waitpid(pid2, NULL, 0); // Para que se espere a que se termine el segundo hijo
    //     waitpid(pid3, NULL, 0); // Para que se espere a que se termine el tercer hijo
    // }
    // else
    // {
    //     waitpid(pid1, NULL, 0); // Para que se espere a que se termine el primer hijo
    //     waitpid(pid2, NULL, 0); // Para que se espere a que se termine el segundo hijo
    // }

    // // waitpid(pid1, NULL, 0); // Para que se espere a que se termine el primer hijo
    // // waitpid(pid2, NULL, 0); // Para que se espere a que se termine el segundo hijo
    // // waitpid(pid3, NULL, 0); // Para que se espere a que se termine el tercer hijo
}
// Creamos la funcion para mostrar el prompt y leer el comando que queremos

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
// void ejecuta_comando(char **argumentos)
// {
//     pid_t pid = fork();
//     int estado;

//     if (pid == 0) // Se puedo crear al proceso hijo
//     {
//         if (execvp(argumentos[0], argumentos) < 0)
//         {
//             printf("Error al ejecutar el comando \n");
//         }
//         exit(1);
//     }
//     else if (pid < 0) // No se pudo crear el proceso hijo
//     {
//         printf("No se pudo crear el proceso hijo\n");
//         exit(1);
//     }
//     else
//     {
//         wait(&estado); // El proceso padre tiene que esperar al hijo para evitar procesos zombies
//     }
// }
int main()
{
    char comando[1024];
    char *argumentos[1024];
    // int estado;
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
        parsear_p(comando, argumentos);
        pipes(argumentos);

        // char *found_p;// Variable que me permitira saber si es que tengo un '|'

        // char *found_p1 = strchr(comando, '|');
        // printf("%s ", found_p);
        // for (int i = 0; i < strlen(comando); i++)
        // {
        //     if (comando[i] == '|')
        //     {
        //         found_p = &comando[i];
        //         break;
        //     }
        // }
        // char *arr_comandos[8][100];
        // int cant_comandos = parsear(comando, argumentos);
        // if (cant_comandos > 0)
        // {
        //     pipes(argumentos, cant_comandos);
        // }
        // else
        // {
        //     ejecuta_comando(argumentos[0]);
        // }
        // if (found_p1 != NULL)
        // {
        //     // continue;
        //     //  De esta forma divimos el comadno en partes en este para poder tratar uno y despues el otro.
        //     *found_p1 = '\0';
        //     char *c1 = comando;
        //     char *c2 = found_p1 + 1;
        //     // Dividimos los argumentos en 2
        //     char *found_p2 = strchr(c2, '|');
        //     if (found_p2 != NULL)
        //     {
        //         *found_p2 = '\0';
        //         char *c3 = c2;
        //         char *c4 = found_p2 + 1;
        //         char *a1[100], *a2[100], *a3[100];
        //         // Luego parseamos cada comando con su argumento correspondiente
        //         parsear(c1, a1);
        //         parsear(c3, a2);
        //         parsear(c4, a3);
        //         // Y mediante la funcion de pipes podemos hacer la conexion entre los argumentos que tengamos
        //         pipes(a1, a2, a3);
        //     }
        //     else
        //     {
        //         char *a1[100], *a2[100];
        //         // Luego parseamos cada comando con su argumento correspondiente
        //         parsear(c1, a1);
        //         parsear(c2, a2);

        //         // Y mediante la funcion de pipes podemos hacer la conexion entre los argumentos que tengamos
        //         pipes(a1, a2, NULL);
        //         printf("Ingreso");
        //     }
        // }
        // else
        // {
        //     parsear(comando, argumentos);
        //     ejecuta_comando(argumentos);
        // }

        // parsear(comando, argumentos);

        // imprimir_argumentos(argumentos);
        // ejecuta_comando(argumentos);
    }

    return 0;
}