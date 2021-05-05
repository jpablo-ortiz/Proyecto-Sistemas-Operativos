/*#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void reservarInt(int **ptr, int num)
{
    // La funcion declara un doble puntero
    // Acceso al contenido del puntero para reservar el bloque de memoria.
    *ptr = (int *)malloc(sizeof(int) * num);
    if (*ptr == NULL)
    {
        puts("\nError de memoria");
        exit(1);
    }
    else
        printf("\nHe reservado en %p", *ptr);
}

void inicializarInt(int *ptr, int num)
{
    for (int i = 0; i < num; i++)
    {
        ptr[i] = 0;
    }
}

void liberarInt(int **ptr)
{
    free(*ptr);
    *ptr = NULL;
}

int main()
{
    int num;
    int *p = NULL;

    reservarInt(&p, num);

    for (int i = 0; i < num; i++)
    {
        p[i] = 0;
    }

    liberarInt(&p);
}

*/


int main(void)
{

    int filas = 2;
    int columnas = 3;
    int **x;

    int i; // Recorre filas
    int j; // Recorre columnas

    // Reserva de Memoria
    
    x = (int **)malloc(filas * sizeof(int *));

    for (i = 0; i < filas; i++)
        x[i] = (int *)malloc(columnas * sizeof(int));


    // Damos Valores a la Matriz
    x[0][0] = 1;
    x[0][1] = 2;
    x[0][2] = 3;

    x[1][0] = 4;
    x[1][1] = 5;
    x[1][2] = 6;

    // Dibujamos la Matriz en pantalla
    for (i = 0; i < filas; i++)
    {
        printf("\n");
        for (j = 0; j < columnas; j++)
            printf("\t%d", x[i][j]);
    }

    free_array(x, filas);
    return 0;
}
