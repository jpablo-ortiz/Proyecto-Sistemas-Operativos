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

    printf("%s %d %s %d %d", ver1, ver2, ver3, ver4, ver5);
    */

    if (argc != 11 /*|| ver1 || ver2 || ver3 || ver4 || ver5*/)
    {
        printf("\nArgumentos inválidos\n");
        printf("USO CORRECTO:\n");
        printf("./controlador –i horaInicio –f horafinal –s segundoshora –t totalpersonas –p pipecrecibe\n");
        exit(1);
    }
}

int crearPipeLecturaServidor(char *pipecrecibe, mode_t fifo_mode)
{
    if (mkfifo(pipecrecibe, fifo_mode) == -1)
    {
        perror("Server mkfifo");
        exit(1);
    }

    //Abrir Pipe Servidor en lectura (Archivo)
    int fd = open(pipecrecibe, O_RDONLY);
    return fd;
}

void leerPipeLecturaServidor(char *pipecrecibe, int fd)
{
    int leer;
    datap respuesta;

    // Recibir estructura datap
    leer = read(fd, &respuesta, sizeof(respuesta));
    if (leer == -1)
    {
        perror("proceso lector:");
        exit(1);
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

    int fdServerLectura;
    datap datos;

    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    fdServerLectura = crearPipeLecturaServidor(pipecrecibe, fifo_mode);

    leerPipe(pipecrecibe, fdServerLectura);
}
