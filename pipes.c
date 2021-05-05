#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "pipes.h"

int CrearPipe(char *pipe)
{
    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    //Crear Pipe
    unlink(pipe);
    if (mkfifo(pipe, fifo_mode) == -1)
    {
        perror("Mkfifo fallido");
        exit(1);
    }
}

int AbrirPipe(char *pipe, int lectura_escritura)
{
    int fd, creado = 0;
    do
    {
        fd = open(pipe, lectura_escritura);
        if (fd == -1)
        {
            perror("\nError abriendo el pipe\n");
            exit(1);
        }
        else
            creado = 1;
    } while (creado == 0);

    return fd;
}