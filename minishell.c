#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

// Para saber que parte del pipe se cierra
#define READ 0
#define WRITE 1

// Estructura que contiene el mensaje y el temporizador para desplegar dicho mensaje
struct Recordatorio
{
    int tiempo;
    char mensaje[256];
};

// Función recordatorio que se ejecutara en un nuevo hilo
void *recordatorio(void *arg)
{
    struct Recordatorio *rec = (struct Recordatorio *)arg;
    printf("Recordatorio establecido: en %d segundos recibirás un aviso.\n", rec->tiempo);
    sleep(rec->tiempo); // Espera por el tiempo indicado
    printf("Recordatorio: %s\n", rec->mensaje);
    free(rec); // Liberar memoria cuando el hilo termine
    return NULL;
}

// Función para setear un recordatorio
void iniciar_recordatorio(int tiempo, const char *mensaje)
{
    pthread_t hilo; // se crea el nuevo hilo donde se ejecutara la funcion recordatorio
    struct Recordatorio *rec = malloc(sizeof(struct Recordatorio));
    rec->tiempo = tiempo;
    strncpy(rec->mensaje, mensaje, sizeof(rec->mensaje));

    // Crear el hilo y ejecutar la función recordatorio en segundo plano
    if (pthread_create(&hilo, NULL, recordatorio, rec) != 0)
    {
        perror("Error al crear el hilo");
        free(rec); // Liberar memoria si hay error
    }

    // No se hace "pthread_join" porque no queremos esperar a que termine,
    // el hilo se ejecutará en segundo plano mientras la shell sigue operando.
}

// Estructura favorito que contiene el comando y el número adjunto al mismo
struct Favorito
{
    int numero;
    char comando[256];
};

struct Favorito favoritos[100]; // Máximo 100 favoritos
int total_favoritos = 0;

// crear un archivo de favoritos
void crear_archivo_favoritos(const char *ruta)
{
    FILE *archivo = fopen(ruta, "w");
    if (archivo == NULL)
    {
        perror("Error al crear archivo");
        return;
    }
    fclose(archivo);
}

// Cada vez que el usuario ejecuta un comando, si no está en la lista de favoritos, se agrega
void agregar_a_favoritos(const char *comando)
{
    for (int i = 0; i < total_favoritos; i++)
    {
        if (strcmp(favoritos[i].comando, comando) == 0)
        {
            return; // Ya existe
        }
    }
    favoritos[total_favoritos].numero = total_favoritos + 1;
    strcpy(favoritos[total_favoritos].comando, comando);
    total_favoritos++;
}

// comando mostrar favoritos
void mostrar_favoritos()
{
    for (int i = 0; i < total_favoritos; i++)
    {
        printf("%d: %s\n", favoritos[i].numero, favoritos[i].comando);
    }
}

// elimina favoritos según sus numeros
void eliminar_favoritos(int num)
{
    for (int i = 0; i < total_favoritos; i++)
    {
        if (favoritos[i].numero == num)
        {
            for (int j = i; j < total_favoritos - 1; j++)
            {
                favoritos[j] = favoritos[j + 1];
            }
            total_favoritos--;
            break;
        }
    }
}

// busca comandos que contengan el subtring asociado y los despliega
void buscar_comandos(const char *substring)
{
    for (int i = 0; i < total_favoritos; i++)
    {
        if (strstr(favoritos[i].comando, substring) != NULL)
        {
            printf("%d: %s\n", favoritos[i].numero, favoritos[i].comando);
        }
    }
}

// borra todos los favoritos
void borrar_favoritos()
{
    total_favoritos = 0;
}

// ejecuta un favorito según su número
void ejecutar_favorito(int num)
{
    for (int i = 0; i < total_favoritos; i++)
    {
        if (favoritos[i].numero == num)
        {
            system(favoritos[i].comando);
            break;
        }
    }
}

// guarda favoritos desde la memoria al archivo
void guardar_favoritos(const char *ruta)
{
    FILE *archivo = fopen(ruta, "w");
    if (archivo == NULL)
    {
        perror("Error al abrir archivo");
        return;
    }
    for (int i = 0; i < total_favoritos; i++)
    {
        fprintf(archivo, "%d:%s\n", favoritos[i].numero, favoritos[i].comando);
    }
    fclose(archivo);
}

// carga los favoritos desde el archivo a la memoria
void cargar_favoritos(const char *ruta)
{
    FILE *archivo = fopen(ruta, "r");
    if (archivo == NULL)
    {
        perror("Error al abrir archivo");
        return;
    }
    char linea[256];
    total_favoritos = 0;
    while (fgets(linea, sizeof(linea), archivo))
    {
        sscanf(linea, "%d:%[^\n]", &favoritos[total_favoritos].numero, favoritos[total_favoritos].comando);
        total_favoritos++;
    }
    fclose(archivo);
}
// Esta funcion me permite visualizar el prompt de la shell, en que se muestra en todo momento mientras no se haya escrito un comando o hecho un enter
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
        token = strtok(NULL, " "); // Con esto me permite obtener los fragmentos siguientes (tokens)
    }
    // Cuando llega al final asignamos NULL para dar termino
    argumentos[i] = NULL;
}

// Si es que pasamos a encontrar un pipe, separamos y parseamos el comando que escribamos cuando encontremos un '|'
void parsear_p(char *entrada, char **comandos)
{
    char *token = strtok(entrada, "|"); // Para tomar cada fragmento del comando queremos ingresar y los separamos en base un "|"
    int i = 0;
    // A continuacion lo que hacemos es que por cada fragmento de la entrada, lo vamos a ir ingresando dentro del arreglo de comandos
    while (token != NULL)
    {
        comandos[i++] = token;
        token = strtok(NULL, "|"); // Con esto me permite obtener los fragmentos siguientes (tokens)
    }
    // Cuando llega al final asignamos NULL para dar termino
    comandos[i] = NULL;
}
// Funcion esencial para poder usar pipes y ejectuar los comandos que escriben
void pipes(char **comandos)
{
    int cant_comandos = 0; // Variable que me ayuda a decidir, cuantos pipes son necesarios y cuantos procesos hijos se realizan
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
    // Por cada comando que tengamos vamos a realizar un proceso hijo
    for (int i = 0; i < cant_comandos; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {

            /*Si se se crea un hijo notamos que dependiendo las condiciones se lee o escribe del pipe
            En primera instancia se escribe por lo que */

            // Redirigimos la entrada en caso de no ser el primer comando que escribimos (salida anterior)
            if (i > 0)
            {
                // Se da cuando tenemos que leer un comando en el pipe
                dup2(descr[(i - 1) * 2 + READ], STDIN_FILENO);
            }
            // Redirigimos la salida del comando (siguiente entrada)
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
            execvp(a[0], a);
            // En caso de que el comando no existe
            if (execvp(a[0], a) < 0)
            {
                printf("No existe ese comando \n");
            }
            // Si pasa hubo un error en el comando
            // perror("Error al ejecutar el comando \n");
            // exit(1);
        }
        else if (pid < 0) // En caso de no poder crear un hijo exitosamente
        {
            perror("Error al crear el hijo hijo");
            exit(1);
        }
    }
    // Cerramos los descriptores del proceso padre
    for (int i = 0; i < 2 * (cant_comandos - 1); i++)
    {
        close(descr[i]);
    }
    // Y luego esperamos a los procesos hijos.
    for (int i = 0; i < cant_comandos; i++)
    {
        wait(NULL);
    }
}

int main()
{
    char comando[1024];
    char *argumentos[1024];
    char archivo_favoritos[] = "favoritos.txt"; // archivo donde se guardadrán los favoritos

    // cargar favoritos
    cargar_favoritos(archivo_favoritos);

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
            guardar_favoritos(archivo_favoritos); // guardan favoritos antes de salir
            break;
        }
        // para setear recordatorio
        if (strncmp(comando, "recordatorio", 12) == 0)
        {
            int tiempo;
            char mensaje[256];
            sscanf(comando + 13, "%d %[^\n]", &tiempo, mensaje);

            // Iniciar recordatorio en un hilo
            iniciar_recordatorio(tiempo, mensaje);
        }
        // testear
        if (strncmp(comando, "favs crear", 10) == 0)
        {
            char *ruta = comando + 11;
            crear_archivo_favoritos(ruta);
        }
        else if (strcmp(comando, "favs mostrar") == 0)
        {
            mostrar_favoritos();
        }
        else if (strncmp(comando, "favs eliminar", 13) == 0)
        {
            int num = atoi(comando + 14);
            eliminar_favoritos(num);
        }
        else if (strncmp(comando, "favs buscar", 11) == 0)
        {
            char *substring = comando + 12;
            buscar_comandos(substring);
        }
        else if (strcmp(comando, "favs borrar") == 0)
        {
            borrar_favoritos();
        }
        else if (strncmp(comando, "favs ejecutar", 13) == 0)
        {
            int num = atoi(comando + 14);
            ejecutar_favorito(num);
        }
        else
        {
            agregar_a_favoritos(comando); // Agregar cualquier comando ejecutado a favoritos
            // system(comando);  // Ejecutar el comando en la shell
        }

        // Guardar favoritos antes de salir
        guardar_favoritos(archivo_favoritos);

        // Una vez ingresado los comandos estos se parsean para saber si contienen pipes
        parsear_p(comando, argumentos);
        // Dependiendo si haya o no pipes, se ejecuta con los comandos que se escriban
        // Si no hay pipes el comando se ejecuta con normalidad.
        // Si hay pipes empieza a crear los procesos hijos y los pipes necesarios, para despues poder ejecutar los comandos que requiera
        pipes(argumentos);
    }

    return 0;
}