#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "estructuras.h"
#include "pipes.h"

//Ejemplo de ejecución
//./controlador –i 7 –f 19 –s 1 –t 10 –p pipecrecibe

sem_t sem_hora, sem_termino, sem_guardar;
int fd_lectura;
int tam, total_personas, hora_inicial, hora_final, segundos_hora;
int num_solicitudes_negadas = 0, num_solicitudes_aceptadas = 0, num_solicitudes_reprogramadas = 0;

int hora_global;        //integer mostrando la hora global del controlador
int termino_global = 0; //Booleano mostrando fin del controlador

//Declarar los vectores y la matriz
reserva **reservas;
int *num_personas;
int *num_reservas;

void VerificarErrorEntrada(int argc, char **argv)
{
    int hora_inicial, hora_final, ver1, ver2, ver3;
    if (argc != 11)
    {
        printf("\nArgumentos inválidos\n");
        printf("USO CORRECTO:\n");
        printf("./controlador –i horaInicio –f horafinal –s segundoshora –t totalpersonas –p pipecrecibe\n");
        exit(1);
    }

    hora_inicial = atoi(argv[2]);
    hora_final = atoi(argv[4]);

    ver1 = !(hora_inicial >= 7 && hora_inicial <= 19);
    ver2 = !(hora_final >= 7 && hora_final <= 19);
    ver3 = !(hora_final > hora_inicial);

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

agente ObtenerAgente(int fd_lectura)
{
    agente agente_actual;

    // Recibir estructura agente para recibir el PipeEscritura del Cliente
    if (read(fd_lectura, &agente_actual, sizeof(agente)) == -1)
    {
        perror("Error leyendo el agente recibido");
        exit(1);
    }
    if (agente_actual.pipe_receptor[0] != '\0')
    {
        printf("\nControlador lee el nombre del Pipe Receptor%sA\n", agente_actual.pipe_receptor);
        printf("Controlador lee el nombre del Pipe Emisor %s\n", agente_actual.pipe_emisor);
        printf("Controlador el pid %d\n", agente_actual.pid);
    }

    return agente_actual;
}

void AsignarReservaEnHorario(reserva reserva_actual)
{
    sem_wait(&sem_guardar);
    int hora1 = reserva_actual.hora - hora_inicial;
    int hora2 = hora1 + 1;

    //Asigna el número de personas a esa hora
    num_personas[hora1] += reserva_actual.num_personas;
    num_personas[hora2] += reserva_actual.num_personas;

    //Poner la reserva en las horas descritas
    strcpy(reservas[hora1][num_reservas[hora1]].nombre_familia, reserva_actual.nombre_familia);
    reservas[hora1][num_reservas[hora1]].num_personas = reserva_actual.num_personas;

    strcpy(reservas[hora2][num_reservas[hora2]].nombre_familia, reserva_actual.nombre_familia);
    reservas[hora2][num_reservas[hora2]].num_personas = reserva_actual.num_personas;

    //Contabiliza una nueva reserva aceptada a esa hora
    num_reservas[hora1]++;
    num_reservas[hora2]++;
    sem_post(&sem_guardar);
}

void SimularHoras(horas *horas)
{
    sem_wait(&sem_hora);
    hora_global = horas->hora_inicial;
    sem_post(&sem_hora);

    for (int i = horas->hora_inicial; i < horas->hora_final; i++)
    {
        sleep(horas->segundos_hora);
        sem_wait(&sem_hora);
        hora_global++;
        sem_post(&sem_hora);
    }

    sem_wait(&sem_termino);
    termino_global = 1;
    sem_post(&sem_termino);

    pthread_exit(0);
}

void InicializarEstructurasDeDatosGlobales()
{
    //Tamaño de filas y columnas
    int filas = tam;
    int columnas = total_personas;

    //Se reserva el espacio para la matriz de las reservas
    reservas = (reserva **)malloc(filas * sizeof(reserva *));

    for (int i = 0; i < filas; i++)
        reservas[i] = (reserva *)malloc(columnas * sizeof(reserva));

    //Se reserva el espacio para los vectores de num_personas y num_reservas

    num_personas = (int *)malloc(filas * sizeof(int));
    num_reservas = (int *)malloc(filas * sizeof(int));

    //Inicializar los vectores en 0
    for (int i = 0; i < filas; i++)
    {
        num_personas[i] = 0;
        num_reservas[i] = 0;
    }
}

void *RealizarProcesoDeUnAgente(void *pArgs)
{
    agente *agente_actual = (agente *)pArgs;

    int finalizo = 0, finalizoHoraSistema = 0;
    int fd_receptor_agente, fd_emisor_agente, hora_actual_sistema;
    char mensaje_respuesta[TAMMENSAJE];
    int disponibles_hora1, disponibles_hora2, bandera = 0;
    reserva reserva_actual;

    //Abrir Pipe en lectura
    fd_receptor_agente = AbrirPipe(agente_actual->pipe_receptor, O_WRONLY);

    //Leer Pipe de Lectura Para Obtener Pipe emisor del Cliente
    fd_emisor_agente = AbrirPipe(agente_actual->pipe_emisor, O_RDONLY);

    //Recibir las reservas del agente y hacer las verificaciones correspondientes
    do
    {
        sem_wait(&sem_termino);
        finalizoHoraSistema = termino_global;
        sem_post(&sem_termino);

        if (!finalizoHoraSistema)
        {
            if (read(fd_emisor_agente, &reserva_actual, sizeof(reserva_actual)) == -1)
            {
                perror("Error leyendo la reserva del agente");
                exit(1);
            }
            finalizo = reserva_actual.finalizo;

            if (!finalizo)
            {
                printf("\nAgente: %s\n", agente_actual->nombre);
                printf("Nombre: %s, Hora: %d, NuMPersonas: %d\n", reserva_actual.nombre_familia, reserva_actual.hora, reserva_actual.num_personas);

                //Verificaciónes necesarias
                bandera = 0;
                //Si la segunda hora de la hora pedida es menor o igual a la hora final o
                //Si el número de personas es menor o igual a la del aforo general continue
                //En caso contrario la reserva se niega
                if (reserva_actual.hora + 1 <= hora_final && reserva_actual.num_personas <= total_personas)
                {
                    //Si la fecha recibida es mayor a la actual del sistema continue
                    sem_wait(&sem_hora);
                    hora_actual_sistema = hora_global;
                    sem_post(&sem_hora);
                    if (reserva_actual.hora > hora_actual_sistema)
                    {
                        disponibles_hora1 = total_personas - num_personas[reserva_actual.hora - hora_inicial];
                        disponibles_hora2 = total_personas - num_personas[reserva_actual.hora - hora_inicial + 1];

                        //Si en ambas horas que se pidieron hay disponibilidad de cupo Reserva OK
                        if (reserva_actual.num_personas <= disponibles_hora1 && reserva_actual.num_personas <= disponibles_hora2)
                        {
                            reserva_actual.mensaje_respuesta = 1;
                            AsignarReservaEnHorario(reserva_actual);
                            num_solicitudes_aceptadas++;
                            printf("Reserva OK.\n");
                        }
                        else
                        {
                            for (int i = hora_actual_sistema - hora_inicial; (i < tam - 1) && (bandera == 0); i++)
                            {
                                disponibles_hora1 = total_personas - num_personas[i];
                                disponibles_hora2 = total_personas - num_personas[i + 1];

                                if (reserva_actual.num_personas <= disponibles_hora1 && reserva_actual.num_personas <= disponibles_hora2)
                                {
                                    reserva_actual.hora = hora_inicial + i;
                                    reserva_actual.finalizo = 1;
                                    reserva_actual.mensaje_respuesta = 2;
                                    AsignarReservaEnHorario(reserva_actual);
                                    num_solicitudes_reprogramadas++;
                                    bandera = 1;
                                    printf("Reserva garantizada para otras horas.\n");
                                }
                            }
                            if (!bandera)
                            {
                                reserva_actual.mensaje_respuesta = 4;
                                num_solicitudes_negadas++;
                                printf("Reserva negada, No se encontró otras horas disponibles.\n");
                            }
                        }
                    }
                    else
                    {
                        for (int i = hora_actual_sistema - hora_inicial; (i < tam - 1) && (bandera == 0); i++)
                        {
                            disponibles_hora1 = total_personas - num_personas[i];
                            disponibles_hora2 = total_personas - num_personas[i + 1];

                            if (reserva_actual.num_personas <= disponibles_hora1 && reserva_actual.num_personas <= disponibles_hora2)
                            {
                                reserva_actual.hora = hora_inicial + i;
                                reserva_actual.finalizo = 1;
                                reserva_actual.mensaje_respuesta = 3;
                                AsignarReservaEnHorario(reserva_actual);
                                num_solicitudes_reprogramadas++;
                                bandera = 1;
                                printf("Reserva negada por tarde. Reserva garantizada para otras horas.\n");
                            }
                        }
                        if (!bandera)
                        {
                            reserva_actual.mensaje_respuesta = 4;
                            num_solicitudes_negadas++;
                            printf("Reserva negada por tarde, No se encontró otras horas disponibles.\n");
                        }
                    }
                }
                else
                {
                    reserva_actual.mensaje_respuesta = 4;
                    num_solicitudes_negadas++;
                    printf("Reserva negada, Hora fuera de la jornada o numPersonas mayor al aforo.\n");
                }

                if (write(fd_receptor_agente, &reserva_actual, sizeof(reserva_actual)) == -1)
                {
                    perror("Error en escritura de la reserva");
                    exit(1);
                }
            }
        }
    } while (!finalizo && !finalizoHoraSistema);

    if (finalizo)
    {
        if (write(fd_receptor_agente, "Finalizó el proceso de reservas para el agente", TAMMENSAJE) == -1)
        {
            perror("Error en escritura del mensaje de finalización");
            exit(1);
        }
    }
    else if (finalizoHoraSistema)
    {
        if (write(fd_receptor_agente, "Finalizó el controlador por horario", TAMMENSAJE) == -1)
        {
            perror("Error en escritura del mensaje de finalización del controlador");
            exit(1);
        }
    }

    free(agente_actual);
    pthread_exit(0);
}

void imprimirResultados()
{
    int horas_pico[tam], horas_mas_desocupadas[tam];
    int num_horas_pico = 0, num_horas_desocupadas = 0;
    int max = 0, min = total_personas;

    printf("\n");
    for (int i = 0; i < tam; i++)
    {
        printf("Hora: %d, Número de Personas reservadas: %d\n", hora_inicial + i, num_personas[i]);
        if (num_personas[i] >= max)
            max = num_personas[i];

        if (num_personas[i] <= min)
            min = num_personas[i];
    }

    for (int i = 0; i < tam; i++)
    {
        if (num_personas[i] == max)
        {
            horas_pico[num_horas_pico] = hora_inicial + i;
            num_horas_pico++;
        }

        if (num_personas[i] == min)
        {
            horas_mas_desocupadas[num_horas_desocupadas] = hora_inicial + i;
            num_horas_desocupadas++;
        }
    }

    printf("\nHoras pico: ");
    for (int l = 0; l < num_horas_pico; l++)
    {
        printf("%d", horas_pico[l]);
        printf((l + 1 < num_horas_pico) ? ", " : ".");
    }
    printf("\nHoras con menor numero de personas: ");
    for (int m = 0; m < num_horas_desocupadas; m++)
    {
        printf("%d", horas_mas_desocupadas[m]);
        printf((m + 1 < num_horas_desocupadas) ? ", " : ".");
    }
    printf("\nNúmero solicitudes negadas: %d\n", num_solicitudes_negadas);
    printf("Número solicitudes aceptadas en su hora: %d\n", num_solicitudes_aceptadas);
    printf("Número solicitudes reprogramadas: %d\n", num_solicitudes_reprogramadas);
}

int main(int argc, char **argv)
{
    VerificarErrorEntrada(argc, argv);

    //inicialización datos
    hora_inicial = atoi(argv[2]);
    hora_final = atoi(argv[4]);
    segundos_hora = atoi(argv[6]);
    total_personas = atoi(argv[8]);
    tam = hora_final - hora_inicial;

    //Otras declaraciones e inicializaciones
    char *pipecrecibe = argv[10];
    agente *agente_actual;
    agente agente_actual_temp;
    pthread_t thread_horas;
    horas horas;

    sem_init(&sem_hora, 1, 1);
    sem_init(&sem_termino, 1, 1);
    sem_init(&sem_guardar, 1, 1);

    //Estructuras para los datos del sistema
    InicializarEstructurasDeDatosGlobales();

    //----------------------------------------
    //---- Proceso para simular el tiempo ----
    //----------------------------------------

    horas.hora_inicial = hora_inicial;
    horas.hora_final = hora_final;
    horas.segundos_hora = segundos_hora;

    //Creación proceso para simular el tiempo
    if (pthread_create(&thread_horas, NULL, (void *)SimularHoras, (void *)&horas))
    {
        perror("Error en la ejecución del hilo simulador de horas");
        exit(1);
    }

    //----------------------------------------
    //---- Proceso para simular el tiempo ----
    //----------------------------------------

    //Crear Pipe de lectura
    CrearPipe(pipecrecibe);
    //Abrir Pipe en lectura
    fd_lectura = AbrirPipe(pipecrecibe, O_RDONLY);

    //------------------------------------------------
    //---- Proceso para la llegada de los agentes ----
    //------------------------------------------------

    int termino = 0;
    while (!termino)
    {
        //Se lee el receptor que haya llegado al Pipe de lectura
        do
        {
            sem_wait(&sem_termino);
            termino = termino_global;
            sem_post(&sem_termino);

            if (!termino)
            {
                agente_actual_temp = ObtenerAgente(fd_lectura);
            }

        } while (agente_actual_temp.pipe_receptor[0] == '\0' && !termino);

        if (!termino)
        {
            agente_actual = malloc(sizeof(agente));
            memcpy(agente_actual, &agente_actual_temp, sizeof(agente));

            pthread_t thread_actual;
            if (pthread_create(&thread_actual, NULL, &RealizarProcesoDeUnAgente, agente_actual))
            {
                perror("Error en la ejecución del hilo de un nuevo agente");
                exit(1);
            }
        }
    }

    //------------------------------------------------
    //---- Proceso para la llegada de los agentes ----
    //------------------------------------------------

    //-------------------------------------------------
    //---- Proceso para mostrar resultados finales ----
    //-------------------------------------------------

    imprimirResultados();

    //-------------------------------------------------
    //---- Proceso para mostrar resultados finales ----
    //-------------------------------------------------

    //--------------------------------------------------------
    //---- Proceso para cerrar y eliminar recursos usados ----
    //--------------------------------------------------------

    //Se cierran los pipes del agente y se eliminan los archivos creados en el directorio
    close(fd_lectura);
    remove(pipecrecibe);

    //Se destruyen los semáforos
    sem_destroy(&sem_hora);
    sem_destroy(&sem_termino);

    free(num_personas);
    num_personas = NULL;

    free(num_reservas);
    num_reservas = NULL;

    for (int i = 0; i < tam; i++)
        free(reservas[i]);

    free(reservas);

    exit(0);

    //--------------------------------------------------------
    //---- Proceso para cerrar y eliminar recursos usados ----
    //--------------------------------------------------------
}