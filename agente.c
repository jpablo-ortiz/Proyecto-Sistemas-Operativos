#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "estructuras.h"

//./agente –s agentazo –a archivosolicitudes –p pipecrecibe

datap datosAgente;

void verificarErrorEntrada(int argc, char **argv)
{
    /*
    int ver1 = strcmp(argv[1], "-s") != 0;
    int ver2 = strcmp(argv[3], "-a") != 0;
    int ver3 = strcmp(argv[5], "-p") != 0;

    printf("%s %d %s %d %d", ver1, ver2, ver3, ver4, ver5);
    */

    if (argc != 7 /*|| ver1 || ver2 || ver3*/)
    {
        printf("\nArgumentos inválidos\n");
        printf("USO CORRECTO:\n");
        printf("./agente –s nombre –a archivosolicitudes –p pipecrecibe\n");
        exit(1);
    }
}

int abrirPipe(char *pipecrecibe, int lecturaEscritura)
{
    int fd, creado = 0;
    do
    {
        fd = open(pipecrecibe, lecturaEscritura);
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

void crearPipe(char *pipecrecibe, mode_t fifo_mode)
{
    //Crear Pipe
    unlink(pipecrecibe);
    if (mkfifo(pipecrecibe, fifo_mode) == -1)
    {
        perror("Agente mkfifo");
        exit(1);
    }
}

int main(int argc, char **argv)
{
    verificarErrorEntrada(argc, argv);

    char *agente = argv[2];
    char *archivoSolicitudes = argv[4];
    char *pipecrecibe = argv[6];

    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    int fdLectura, fdEscritura, pid;
    char mensaje[10];

    //Obtener Pipe de escritura al Servidor
    fdEscritura = abrirPipe(pipecrecibe, O_WRONLY);

    //Poner los datos del pipe para enviar al controlador (servidor)
    pid = getpid();
    datosAgente.pid = pid;
    strcpy(datosAgente.segundopipe, agente);

    //Crear Pipe de Lectura
    crearPipe(datosAgente.segundopipe, fifo_mode);

    //Se envia el dato del pipe para recibir mensajes del servidor al cliente
    write(fdEscritura, &datosAgente, sizeof(datosAgente));

    //Abrir el Pipe de Lectura creado
    fdLectura = abrirPipe(pipecrecibe, O_RDONLY);

    read(fdLectura, mensaje, 10);
    printf("El proceso cliente termina y lee %s \n", mensaje);

    exit(0);
}
