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

int abrirPipe(char *pipe, int lecturaEscritura)
{
    int fd, creado = 0;
    do
    {
        fd = open(pipe, O_RDONLY);
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

int crearPipe(char *pipe, mode_t fifo_mode, int lecturaEscritura)
{
    //Crear Pipe
    //unlink(pipecrecibe);
    if (mkfifo(pipe, fifo_mode) == -1)
    {
        perror("Controlador mkfifo");
        exit(1);
    }

    //Abrir Pipe en lectura (Archivo)
    int fd = abrirPipe(pipe, lecturaEscritura);
    return fd;
}

agente obtenerAgente(int fdLectura)
{
    int leer;
    agente agenteActual;

    // Recibir estructura datap para recibir el PipeEscritura del Cliente
    leer = read(fdLectura, &agenteActual, sizeof(agenteActual));
    if (leer == -1)
    {
        perror("proceso lector:");
        exit(1);
    }
    printf("Controlador lee el nombre del Pipe Receptor %s\n", agenteActual.pipeReceptor);
    printf("Controlador lee el nombre del Pipe Emisor %s\n", agenteActual.pipeEmisor);
    printf("Controlador el pid %d\n", agenteActual.pid);

    return agenteActual;
}

int obtenerPipe(int fdLectura, int lecturaEscritura, char *pipe)
{
    int fd, creado = 0;

    //Obtener Pipe
    do
    {
        fd = open(pipe, lecturaEscritura);
        if (fd == -1)
        {
            perror("Controlador Abriendo pipe ");
            printf(" Se volvera a intentar despues\n");
            sleep(2);
        }
        else
            creado = 1;
    } while (creado == 0);

    return fd;
}

void procesoVerificacionReserva()
{
}

void leerReservas()
{
    int leer, terminate = 0;
    while (terminate == 0)
    {
        leer = read(fdEmisorAgente, &reservaActual, sizeof(reservaActual));
        if (leer == -1)
        {
            perror("proceso lector:");
            exit(1);
        }
        printf("Nombre: %s, Hora: %d, NuMPersonas: %d\n", reservaActual.nombreFamilia, reservaActual.hora, reservaActual.numPersonas);
        terminate = reservaActual.terminate;

        procesoVerificacionReserva();
    }
}

int main(int argc, char **argv)
{
    verificarErrorEntrada(argc, argv);

    int horaInicio = atoi(argv[2]);
    int horaFinal = atoi(argv[4]);
    int segundoshora = atoi(argv[6]);
    int totalpersonas = atoi(argv[8]);

    char *pipecrecibe = argv[10];

    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    int fdLectura, fdReceptorAgente, fdEmisorAgente;

    agente agenteActual;
    reserva reservaActual;

    //Crear Pipe de lectura
    fdLectura = crearPipe(pipecrecibe, fifo_mode, O_RDONLY);

    //TODO EL PROCESO DE ABAJO ES SOLO PARA UN AGENTE (ADAPTAR PARA VARIOS)

    //Se lee el receptor que haya llegado al Pipe de lectura
    agenteActual = obtenerAgente(fdLectura);

    //Leer Pipe de Lectura Para Obtener Pipe receptor del Cliente
    fdReceptorAgente = obtenerPipe(fdLectura, O_WRONLY, agenteActual.pipeReceptor);

    //Leer Pipe de Lectura Para Obtener Pipe emisor del Cliente
    fdEmisorAgente = obtenerPipe(fdLectura, O_RDONLY, agenteActual.pipeEmisor);

    // Recibir estructura datap para recibir el PipeEscritura del Cliente
    leerReservas();

    write(fdReceptorAgente, "Finalizó el proceso de reservas para el agente", TAMMENSAJE);

    exit(0);
}