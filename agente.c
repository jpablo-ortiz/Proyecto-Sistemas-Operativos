#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "estructuras.h"

//./agente –s agentazo –a ejemplo.txt –p pipecrecibe

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

void leerArchivo(char *archivoSolicitudes, int fdEmisor)
{
    FILE *f = fopen(archivoSolicitudes, "r");
    reserva reservaActual;

    if (f != NULL)
    {
        char delimitador[] = ",";
        while (fscanf(f, "%s,%d,%d\n", reservaActual.nombreFamilia, &reservaActual.hora, &reservaActual.numPersonas) != EOF)
        {
            strcpy(reservaActual.nombreFamilia, strtok(reservaActual.nombreFamilia, delimitador));
            printf("Nombre: %s, Hora: %d, NuMPersonas: %d\n", reservaActual.nombreFamilia, reservaActual.hora, reservaActual.numPersonas);
            reservaActual.terminate = 0;

            //Enviar la reserva recien leida al controlador (servidor)
            write(fdEmisor, &reservaActual, sizeof(reservaActual));
        }
    }
    else
    {
        printf("Error al abrir el archivo \n");
        exit(1);
    }
    fclose(f);

    //Enviar mensaje con terminate true para terminar
    reservaActual.terminate = 1;
    write(fdEmisor, &reservaActual, sizeof(reservaActual));
}

int main(int argc, char **argv)
{
    verificarErrorEntrada(argc, argv);

    char nombreAgenteReceptor[20];
    strcpy(nombreAgenteReceptor, argv[2]);

    char nombreAgenteEmisor[20];
    strcpy(nombreAgenteEmisor, argv[2]);

    char *archivoSolicitudes = argv[4];
    char *pipecrecibe = argv[6];

    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    int fdEscritura, fdEmisor, fdReceptor, pid, creado = 0;
    char mensaje[100];

    agente datosAgente;

    //Obtener Pipe de escritura al Servidor
    fdEscritura = abrirPipe(pipecrecibe, O_WRONLY);

    //Poner los datos del pipe para enviar al controlador (servidor)
    pid = getpid();
    datosAgente.pid = pid;
    strcpy(datosAgente.pipeReceptor, strcat(nombreAgenteReceptor, "Receptor"));
    strcpy(datosAgente.pipeEmisor, strcat(nombreAgenteEmisor, "Emisor"));

    //Crear Pipe receptor de cada reserva de este agente
    crearPipe(datosAgente.pipeReceptor, fifo_mode);

    //Crear Pipe emisor de cada reserva de este agente
    crearPipe(datosAgente.pipeEmisor, fifo_mode);

    //Se envia el dato del pipe para recibir mensajes del servidor al cliente
    write(fdEscritura, &datosAgente, sizeof(datosAgente));

    //Abrir el Pipe receptor creado
    fdReceptor = abrirPipe(datosAgente.pipeReceptor, O_RDONLY);

    //Abrir el Pipe emisor creado
    fdEmisor = abrirPipe(datosAgente.pipeEmisor, O_WRONLY);

    //Leer y enviar cada reserca del archivo ingresado
    leerArchivo(archivoSolicitudes, fdEmisor);

    //Se recibe la respuesta del controlador (servidor)
    read(fdReceptor, mensaje, TAMMENSAJE);
    printf("El proceso cliente termina y lee %s \n", mensaje);
    exit(0);
}
