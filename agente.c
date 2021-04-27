#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "estructuras.h"

//Ejemplo de ejecución
//./agente –s agenteA –a ejemplo.txt –p pipecrecibe

void verificarErrorEntrada(int argc, char **argv)
{
    if (argc != 7)
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

void leerArchivo(char *archivoSolicitudes, int fdEmisor, int fdReceptor)
{
    FILE *f = fopen(archivoSolicitudes, "r");
    reserva reservaActual;
    char mensaje[TAMMENSAJE];

    if (f != NULL)
    {
        char delimitador[] = ",";
        while (fscanf(f, "%s\n", mensaje) != EOF)
        {
            char *token = strtok(mensaje, delimitador);
            strcpy(reservaActual.nombreFamilia, token);
            token = strtok(NULL, delimitador);
            if (token != NULL)
                reservaActual.hora = atoi(token);
            token = strtok(NULL, delimitador);
            if (token != NULL)
                reservaActual.numPersonas = atoi(token);

            printf("\nNombre: %s, Hora: %d, NuMPersonas: %d\n", reservaActual.nombreFamilia, reservaActual.hora, reservaActual.numPersonas);
            reservaActual.terminate = 0;

            //Enviar la reserva recien leida al controlador (servidor)
            write(fdEmisor, &reservaActual, sizeof(reservaActual));

            //Lee la respuesta dada por el Controlador (servidor)
            read(fdReceptor, &reservaActual, sizeof(reservaActual));
            printf("Respuesta: ");

            switch (reservaActual.mensajeRespuesta)
            {
            case 1:
                printf("Reserva ok.\n");
                break;
            case 2:
                printf("Reserva garantizada para otras horas.\n");
                break;
            case 3:
                printf("Reserva negada por tarde.\n");
                break;
            case 4:
                printf("Reserva negada, debe volver otro día.\n");
                break;

            default:
                break;
            }
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

    char nombreAgente[20];
    strcpy(nombreAgente, argv[2]);

    char *archivoSolicitudes = argv[4];
    char *pipecrecibe = argv[6];

    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    int fdEscritura, fdEmisor, fdReceptor, pid, creado = 0;
    char mensaje[TAMMENSAJE];

    agente datosAgente;

    //Obtener Pipe de escritura al Servidor
    fdEscritura = abrirPipe(pipecrecibe, O_WRONLY);

    //Poner los datos del pipe para enviar al controlador (servidor)
    pid = getpid();
    datosAgente.pid = pid;
    strcpy(datosAgente.nombre, nombreAgente);
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
    leerArchivo(archivoSolicitudes, fdEmisor, fdReceptor);

    //Se recibe la respuesta del controlador (servidor)
    read(fdReceptor, mensaje, TAMMENSAJE);
    printf("\nCONTROLADOR - SERVIDOR: %s\n", mensaje);

    //Se cierran los pipes del agente y se eliminan los archivos creados en el directorio
    close(fdReceptor);
    remove(datosAgente.pipeReceptor);
    close(fdEmisor);
    remove(datosAgente.pipeEmisor);

    printf("Agente %s termina\n", datosAgente.nombre);
    exit(0);
}
