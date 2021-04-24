#define TAMMENSAJE 100

typedef struct Reserva
{
    char nombreFamilia[40];
    int hora;
    int numPersonas;
    int terminate;

} reserva;

typedef struct data
{
    char segundopipe[20];
    int pid;

} datap;

typedef struct Agente
{
    char pipeReceptor[20];
    char pipeEmisor[20];
    int pid;

} agente;