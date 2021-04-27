#define TAMMENSAJE 50

typedef struct Reserva
{
    char nombreFamilia[20];
    int hora;
    int numPersonas;
    int terminate;
    //1 - Reserva ok
    //2 - Reserva garantizada para otras horas
    //3 - Reserva negada por tarde
    //4 - Reserva negada, debe volver otro día
    int mensajeRespuesta;

} reserva;

typedef struct Agente
{
    char nombre[20];
    char pipeReceptor[20];
    char pipeEmisor[20];
    int pid;

} agente;
