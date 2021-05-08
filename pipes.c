/*
Archivo: pipes.c
Realizado por: Paula Juliana Rojas, Carlos Loreto, Juan Pablo Ortiz.
Contiene: implementación de las funciones que permiten crear y abrir pipes.
Fecha última modificación: 06/05/2021
*/

//----------------------------------------
//--------------- Includes ---------------
//----------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "pipes.h"

//----------------------------------------
//-------------- Funciones ---------------
//----------------------------------------

/*
Función: CrearPipe
Parámetros de Entrada: apuntador a la cadena de caracteres con el
nombre del pipe a crear.
Valor de salida: no tiene
Descripción: crea un pipe nominal con el nombre recibido.
*/
void CrearPipe(char *pipe)
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

/*
Función: AbrirPipe
Parámetros de Entrada: apuntador a la cadena de caracteres con el
nombre del pipe a abrir y el tipo de apertura del pipe (lectura y/o escritura).
Valor de salida: un entero con el descriptor del pipe.
Descripción: se abre la conexión al pipe recibido con el tipo de apertura
y devuelve el descriptor del pipe.
*/
int AbrirPipe(char *pipe, int lectura_escritura)
{
    int fd = open(pipe, lectura_escritura);
    if (fd == -1)
    {
        perror("\nError abriendo el pipe\n");
        exit(1);
    }
    return fd;
}