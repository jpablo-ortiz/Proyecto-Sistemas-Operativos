#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "estructuras.h"

//./controlador –i 1 –f 1 –s 1 –t 1 –p pipecrecibe

void verificarErrorEntrada(int argc, char **argv)
{
    /*
    int ver1 = strcmp(argv[1], "-i") != 0;
    int ver2 = strcmp(argv[3], "-f") != 0;
    int ver3 = strcmp(argv[5], "-s") != 0;
    int ver4 = strcmp(argv[7], "-t") != 0;
    int ver5 = strcmp(argv[9], "-p") != 0;

    printf("%d %d %d %d %d", ver1, ver2, ver3, ver4, ver5);
    */

    if (argc != 11 /*|| ver1 || ver2 || ver3 || ver4 || ver5*/)
    {
        printf("\nArgumentos inválidos\n");
        printf("USO CORRECTO:\n");
        printf("./controlador –i horaInicio –f horafinal –s segundoshora –t totalpersonas –p pipecrecibe\n");
        exit(1);
    }
}

int abrirPipe(char *pipecrecibe, int lecturaEscritura)
{
    int fd, creado = 0;
    do
    {
        fd = open(pipecrecibe, O_RDONLY);
        if (fd == -1)
        {
            perror("\nError abriendo el pipe\n");
            printf("Se volvera a intentar en 2 segundos\n");
            sleep(2);
        }
        else
            creado = 1;
    } while (creado == 0);

    return fd;
}

int crearPipe(char *pipecrecibe, mode_t fifo_mode, int lecturaEscritura)
{
    //Crear Pipe
    //unlink(pipecrecibe);
    if (mkfifo(pipecrecibe, fifo_mode) == -1)
    {
        perror("Controlador mkfifo");
        exit(1);
    }

    //Abrir Pipe en lectura (Archivo)
    //int fd = abrirPipe(pipecrecibe, lecturaEscritura);
    int fd = open(pipecrecibe, O_RDONLY);
    return fd;
}

int obtenerPipeEscrituraCliente(char *pipecrecibe, int fdLectura)
{
    int leer, fdEscritura, creado = 0;
    datap respuesta;

    // Recibir estructura datap para recibir el PipeEscritura del Cliente
    leer = read(fdLectura, &respuesta, sizeof(respuesta));
    if (leer == -1)
    {
        perror("proceso lector:");
        exit(1);
    }
    printf("Server lee el string %s\n", respuesta.segundopipe);
    printf("Server el pid %d\n", respuesta.pid);

    //Obtener el Pipe para escritura al Cliente
    do
    {
        fdEscritura = open(respuesta.segundopipe, O_WRONLY);
        if (fdEscritura == -1)
        {
            perror(" Server Abriendo el segundo pipe ");
            printf(" Se volvera a intentar despues\n");
            sleep(2);
        }
        else
            creado = 1;
    } while (creado == 0);

    return fdEscritura;
}

int main(int argc, char **argv)
{
    //verificarErrorEntrada(argc, argv);

    int horaInicio = atoi(argv[2]);
    int horaFinal = atoi(argv[4]);
    int segundoshora = atoi(argv[6]);
    int totalpersonas = atoi(argv[8]);

    char *pipecrecibe = argv[10];

    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    int fdLectura, fdEscritura;

    //Crear Pipe de lectura
    fdLectura = crearPipe(pipecrecibe, fifo_mode, O_RDONLY);

    //Leer Pipe de Lectura Para Obtener Pipe de escritura del Cliente
    fdEscritura = obtenerPipeEscrituraCliente(pipecrecibe, fdLectura);

    write(fdEscritura, "Hola", 5);

    exit(0);
}
