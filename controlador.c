#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "estructuras.h"

//Ejemplo de ejecución
//./controlador –i 7 –f 19 –s 1 –t 10 –p pipecrecibe

int tam, totalpersonas;

void verificarErrorEntrada(int argc, char **argv)
{
    if (argc != 11)
    {
        printf("\nArgumentos inválidos\n");
        printf("USO CORRECTO:\n");
        printf("./controlador –i horaInicio –f horafinal –s segundoshora –t totalpersonas –p pipecrecibe\n");
        exit(1);
    }

    int horaInicio = atoi(argv[2]);
    int horaFinal = atoi(argv[4]);

    int ver1 = !(horaInicio >= 7 && horaInicio <= 19);
    int ver2 = !(horaFinal >= 7 && horaFinal <= 19);
    int ver3 = !(horaFinal > horaInicio);

    if (ver1)
    {
        printf("\nError\n");
        printf("Hora de inicio debe estar en un rango de 7 a 19 horas\n");
        exit(1);
    }
    if (ver2)
    {
        printf("\nError\n");
        printf("Hora final debe estar en un rango de 7 a 19 horas\n");
        exit(1);
    }
    if (ver3)
    {
        printf("\nError\n");
        printf("La hora final debe ser mayor que la hora de inicio\n");
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

    // Recibir estructura agente para recibir el PipeEscritura del Cliente
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

void asignarReservaEnHorario(int *numPersonas, int *numReservas, reserva reservas[tam][totalpersonas], int horaInicio, reserva reservaActual)
{
    int hora1 = reservaActual.hora - horaInicio;
    int hora2 = hora1 + 1;

    //Asigna el número de personas a esa hora
    numPersonas[hora1] += reservaActual.numPersonas;
    numPersonas[hora2] += reservaActual.numPersonas;

    //printf("PREUBA Hora: %d, Vuelta: %d, Valor: %d\n", reservaActual.numPersonas, numPersonas[horaInicio + reservaActual.hora]);

    //Poner la reserva en las horas descritas
    strcpy(reservas[hora1][numReservas[hora1]].nombreFamilia, reservaActual.nombreFamilia);
    reservas[hora1][numReservas[hora1]].numPersonas = reservaActual.numPersonas;

    strcpy(reservas[hora2][numReservas[hora2]].nombreFamilia, reservaActual.nombreFamilia);
    reservas[hora2][numReservas[hora2]].numPersonas = reservaActual.numPersonas;

    //Contabiliza una nueva reserva aceptada a esa hora
    numReservas[hora1]++;
    numReservas[hora2]++;
}

int main(int argc, char **argv)
{
    verificarErrorEntrada(argc, argv);

    int horaInicio = atoi(argv[2]);
    int horaFinal = atoi(argv[4]);
    int segundoshora = atoi(argv[6]);
    totalpersonas = atoi(argv[8]);
    char *pipecrecibe = argv[10];

    //TODO implementar la hora actual del sistema
    int horaActualDelSistema = horaInicio;

    tam = horaFinal - horaInicio;
    reserva reservas[tam][totalpersonas];
    int numPersonas[tam];
    int numReservas[tam];
    for (int i = 0; i < tam; i++)
    {
        numPersonas[i] = 0;
        numReservas[i] = 0;
    }

    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    int fdLectura, fdReceptorAgente, fdEmisorAgente;
    int numSolicitudesNegadas = 0, numSolicitudesAceptadas = 0, numSolicitudesReprogramadas = 0;

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

    //Recibir las reservas del agente y hacer las verificaciones correspondientes
    int leer, terminate = 0;
    do
    {
        leer = read(fdEmisorAgente, &reservaActual, sizeof(reservaActual));
        if (leer == -1)
        {
            perror("proceso lector:");
            exit(1);
        }
        terminate = reservaActual.terminate;

        if (!terminate)
        {
            printf("\nAgente: %s\n", agenteActual.nombre);
            printf("Nombre: %s, Hora: %d, NuMPersonas: %d\n", reservaActual.nombreFamilia, reservaActual.hora, reservaActual.numPersonas);

            //Verificaciónes necesarias
            char mensajeRespuesta[TAMMENSAJE];
            int disponiblesHora1, disponiblesHora2, bandera = 0;
            //Si la segunda hora de la hora pedida es menor o igual a la hora final o
            //Si el número de personas es menor o igual a la del aforo general continue
            //En caso contrario la reserva se niega
            if (reservaActual.hora + 1 <= horaFinal && reservaActual.numPersonas <= totalpersonas)
            {
                //Si la fecha recibida es mayor a la actual del sistema continue
                if (reservaActual.hora > horaActualDelSistema)
                {
                    disponiblesHora1 = totalpersonas - numPersonas[reservaActual.hora - horaInicio];
                    disponiblesHora2 = totalpersonas - numPersonas[reservaActual.hora - horaInicio + 1];

                    //Si en ambas horas que se pidieron hay disponibilidad de cupo Reserva OK
                    if (reservaActual.numPersonas <= disponiblesHora1 && reservaActual.numPersonas <= disponiblesHora2)
                    {
                        reservaActual.mensajeRespuesta = 1;
                        asignarReservaEnHorario(numPersonas, numReservas, reservas, horaInicio, reservaActual);
                        printf("Reserva OK.\n");
                        numSolicitudesAceptadas++;
                    }
                    else
                    {
                        for (int i = 0; i < tam - 1; i++)
                        {
                            disponiblesHora1 = totalpersonas - numPersonas[i];
                            disponiblesHora2 = totalpersonas - numPersonas[i + 1];

                            if (reservaActual.numPersonas <= disponiblesHora1 && reservaActual.numPersonas <= disponiblesHora2)
                            {
                                reservaActual.hora = horaInicio + i;
                                reservaActual.terminate = 1;
                                reservaActual.mensajeRespuesta = 2;
                                asignarReservaEnHorario(numPersonas, numReservas, reservas, horaInicio, reservaActual);
                                bandera = 1;
                                printf("Reserva garantizada para otras horas.\n");
                                numSolicitudesReprogramadas++;
                                break;
                            }
                        }
                        if (!bandera)
                        {
                            printf("Reserva negada, No se encontró otras horas disponibles.\n");
                            reservaActual.mensajeRespuesta = 4;
                            numSolicitudesNegadas++;
                        }
                    }
                }
                else
                {
                    for (int i = 0; i < tam - 1; i++)
                    {
                        disponiblesHora1 = totalpersonas - numPersonas[i];
                        disponiblesHora2 = totalpersonas - numPersonas[i + 1];

                        if (reservaActual.numPersonas <= disponiblesHora1 && reservaActual.numPersonas <= disponiblesHora2)
                        {
                            reservaActual.hora = horaInicio + i;
                            reservaActual.terminate = 1;
                            reservaActual.mensajeRespuesta = 3;
                            asignarReservaEnHorario(numPersonas, numReservas, reservas, horaInicio, reservaActual);
                            bandera = 1;
                            printf(" Reserva negada por tarde, No se encontró otras horas disponibles.\n");
                            numSolicitudesReprogramadas++;
                            break;
                        }
                    }
                    if (!bandera)
                    {
                        printf("Reserva negada, Hora posterior a la del sistema y No se encontró otras horas disponibles.\n");
                        reservaActual.mensajeRespuesta = 4;
                        numSolicitudesNegadas++;
                    }
                }
            }
            else
            {
                printf("Reserva negada, Hora fuera de la jornada o numPersonas mayor al aforo.\n");
                reservaActual.mensajeRespuesta = 4;
                numSolicitudesNegadas++;
            }

            write(fdReceptorAgente, &reservaActual, sizeof(reservaActual));
        }
    } while (!terminate);

    write(fdReceptorAgente, "Finalizó el proceso de reservas para el agente", TAMMENSAJE);

    //TODO EL PROCESO DE ARRIBA ES SOLO PARA UN AGENTE (ADAPTAR PARA VARIOS)

    int horasPico[12], horasMasDesocupadas[12];
    int j, k = 0;
    int max = 0, imax;
    int min = totalpersonas, imin;
    printf("\n");
    for (int i = 0; i < tam; i++)
    {
        printf("Hora: %d, Número de Personas reservadas: %d\n", horaInicio + i, numPersonas[i]);
        if (numPersonas[i] >= max)
        {
            max = numPersonas[i];

            if (i != 0)
            {
                horasPico[j] = horaInicio + i;
                j++;
            }
        }

        if (numPersonas[i] <= min)
        {
            min = numPersonas[i];
            horasMasDesocupadas[k] = horaInicio + i;
            k++;
        }
    }

    printf("\nHoras pico: ");
    for (int l = 0; l < j; l++)
    {
        printf("%d", horasPico[l]);
        printf((l + 1 < j) ? ", " : ".");
    }
    printf("\nHoras con menor numero de personas: ");
    for (int m = 0; m < k; m++)
    {
        printf("%d", horasMasDesocupadas[m]);
        printf((m + 1 < k) ? ", " : ".");
    }
    printf("\nNúmero solicitudes negadas: %d\n", numSolicitudesNegadas);
    printf("Número solicitudes aceptadas en su hora: %d\n", numSolicitudesAceptadas);
    printf("Número solicitudes reprogramadas: %d\n", numSolicitudesReprogramadas);

    //Se cierran los pipes del agente y se eliminan los archivos creados en el directorio
    close(fdLectura);
    remove(pipecrecibe);

    exit(0);
}
