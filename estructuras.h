/*
Archivo: estructuras.h
Realizado por: Paula Juliana Rojas, Carlos Loreto, Juan Pablo Ortiz.
Contiene: definición de las estructuras Reserva, Agente y Horas 
y definición de las constantes del programa.
Fecha última modificación: 06/05/2021
*/

//----------------------------------------
//-------------- Constantes --------------
//----------------------------------------

#define TAMMENSAJE 100
#define TAMNOMBRES 20

//----------------------------------------
//------------- Estructuras --------------
//----------------------------------------

typedef struct Reserva
{
    char nombre_familia[TAMNOMBRES];
    int hora;
    int num_personas;
    //1 - Reserva ok
    //2 - Reserva garantizada para otras horas
    //3 - Reserva negada por tarde, reserva garantizada para otras horas.
    //4 - Reserva negada, debe volver otro día
    int mensaje_respuesta;
    int finalizo;    //Booleano para informar se finalizó el proceso
    int hora_sistema; //Para compartir la hora del sistema del controlador.

} reserva;

typedef struct Agente
{
    char nombre[TAMNOMBRES];
    char pipe_receptor[TAMNOMBRES];
    char pipe_emisor[TAMNOMBRES];
    int pid;

} agente;

typedef struct Horas
{
    int hora_inicial;
    int hora_final;
    int segundos_hora;

} horas;