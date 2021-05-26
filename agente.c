/*
Archivo: agente.c
Realizado por: Paula Juliana Rojas, Carlos Loreto, Juan Pablo Ortiz.
Contiene: implementación de las funcionalidades que realiza un agente.
Fecha última modificación: 06/05/2021
*/

//----------------------------------------
//--------------- Includes ---------------
//----------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "estructuras.h"
#include "pipes.h"

//----------------------------------------
//-------------- Funciones ---------------
//----------------------------------------

/*
Función: VerificarErrorEntrada
Parámetros de Entrada: entero con la cantidad de argumentos
y apuntador apuntador con el contenido de los argumentos.
Valor de salida: no tiene
Descripción: realiza la verificación de los argumentos
ingresados al controlador.
*/
void VerificarErrorEntrada(int argc, char **argv)
{
    if (argc != 7)
    {
        printf("\nArgumentos inválidos\n");
        printf("USO CORRECTO:\n");
        printf("./agente –s nombre –a archivosolicitudes –p pipecrecibe\n");
        exit(1);
    }
}

/*
Función: LeerArchivo
Parámetros de Entrada: apuntador de caracteres del nombre del archivo a leer,
entero con el descriptor del pipe emisor, entero con el descriptor del pipe receptor,
entero con la hora del sistema del controlador.
Valor de salida: no tiene
Descripción: realiza la lectura de las reservas en el archivo para ser enviadas 
al controlador y recibir la respuesta dada.
*/
void LeerArchivo(char *archivo_solicitudes, int fd_emisor, int fd_receptor, int hora_sistema)
{
    reserva reserva_actual;
    char mensaje[TAMMENSAJE];
    char delimitador[] = ",";
    char *token;
    FILE *f = fopen(archivo_solicitudes, "r");

    if (f != NULL)
    {
        while (fscanf(f, "%s\n", mensaje) != EOF)
        {
            //Lectura del archivo para guardar los datos en una estructura reserva
            token = strtok(mensaje, delimitador);
            strcpy(reserva_actual.nombre_familia, token);
            token = strtok(NULL, delimitador);
            if (token != NULL)
                reserva_actual.hora = atoi(token);
            token = strtok(NULL, delimitador);
            if (token != NULL)
                reserva_actual.num_personas = atoi(token);
            if (reserva_actual.hora > hora_sistema)
            {
                printf("\nNombre: %s, Hora: %d, NuMPersonas: %d\n", reserva_actual.nombre_familia, reserva_actual.hora, reserva_actual.num_personas);
                reserva_actual.finalizo = 0;

                //Enviar la reserva leida del archivo al controlador (servidor)
                if (write(fd_emisor, &reserva_actual, sizeof(reserva_actual)) == -1)
                {
                    perror("Error en escritura de la reserva");
                    exit(1);
                }

                //Lee la respuesta dada por el Controlador (servidor)
                if (read(fd_receptor, &reserva_actual, sizeof(reserva_actual)) == -1)
                {
                    perror("Error en la lectura de la respuesta del controlador");
                    exit(1);
                }

                printf("Respuesta: ");

                switch (reserva_actual.mensaje_respuesta)
                {
                case 1:
                    printf("Reserva ok.\n");
                    break;
                case 2:
                    printf("Sin Cupo. Reserva garantizada para otras horas.\n");
                    break;
                case 3:
                    printf("Reserva negada por tarde. Reserva garantizada para otras horas.\n");
                    break;
                case 4:
                    printf("Reserva negada, debe volver otro día.\n");
                    break;

                default:
                    break;
                }
                //Si se quiere agregar una relentización por x segundos por cada reserva
                //activar la linea de abajo con x cantidad de segundos.
                //sleep(10);
            }
        }
    }
    else
    {
        perror("Error al abrir el archivo \n");
        exit(1);
    }
    fclose(f);

    //Enviar mensaje con finalizo true para terminar
    reserva_actual.finalizo = 1;
    if (write(fd_emisor, &reserva_actual, sizeof(reserva_actual)) == -1)
    {
        perror("Error en escritura del mensaje finalización agente");
        exit(1);
    }
}

//----------------------------------------
//----------------- Main -----------------
//----------------------------------------

int main(int argc, char **argv)
{
    VerificarErrorEntrada(argc, argv);

    //Declaración e inicialización de datos
    int fd_escritura, fd_emisor, fd_receptor, pid, creado = 0;
    char mensaje[TAMMENSAJE];
    agente datos_agente;
    reserva reserva_actual;

    char nombre_agente_receptor[TAMNOMBRES];
    strcpy(nombre_agente_receptor, argv[2]);

    char nombre_agente_emisor[TAMNOMBRES];
    strcpy(nombre_agente_emisor, argv[2]);

    char nombre_agente[TAMNOMBRES];
    strcpy(nombre_agente, argv[2]);

    char *archivo_solicitudes = argv[4];
    char *pipecrecibe = argv[6];

    //Obtener Pipe de escritura al Servidor
    fd_escritura = AbrirPipe(pipecrecibe, O_WRONLY);

    //Poner los datos del pipe para enviar al controlador (servidor)
    pid = getpid();
    datos_agente.pid = pid;
    strcpy(datos_agente.nombre, nombre_agente);
    strcpy(datos_agente.pipe_receptor, strcat(nombre_agente_receptor, "Receptor"));
    strcpy(datos_agente.pipe_emisor, strcat(nombre_agente_emisor, "Emisor"));

    //Crear Pipe receptor de cada reserva de este agente
    CrearPipe(datos_agente.pipe_receptor);

    //Crear Pipe emisor de cada reserva de este agente
    CrearPipe(datos_agente.pipe_emisor);

    //Se envia el dato del agente para recibir mensajes del servidor
    if (write(fd_escritura, &datos_agente, sizeof(datos_agente)) == -1)
    {
        perror("Error en escritura de los datos del agente");
        exit(1);
    }

    //Abrir el Pipe receptor creado
    fd_receptor = AbrirPipe(datos_agente.pipe_receptor, O_RDONLY);

    //Abrir el Pipe emisor creado
    fd_emisor = AbrirPipe(datos_agente.pipe_emisor, O_WRONLY);

    //leer respuesta de hora del controlador
    if (read(fd_receptor, &reserva_actual, sizeof(reserva_actual)) == -1)
    {
        perror("Error en la lectura de la respuesta del controlador");
        exit(1);
    }
    printf("\nHora actual del controlador: %d\n", reserva_actual.hora_sistema);

    //Leer y enviar cada reserca del archivo ingresado
    LeerArchivo(archivo_solicitudes, fd_emisor, fd_receptor, reserva_actual.hora_sistema);

    //Se recibe la respuesta del controlador (servidor)
    if (read(fd_receptor, mensaje, TAMMENSAJE) == -1)
    {
        perror("Error en la lectura del mensaje final del controlador");
        exit(1);
    }
    printf("\nCONTROLADOR - SERVIDOR: %s\n", mensaje);

    //Se cierran los pipes del agente y se eliminan los archivos creados en el directorio
    close(fd_receptor);
    remove(datos_agente.pipe_receptor);
    close(fd_emisor);
    remove(datos_agente.pipe_emisor);

    printf("Agente %s termina\n", datos_agente.nombre);
    exit(0);
}

//----------------------------------------
//---------------- FINAL -----------------
//----------------------------------------