#define TAMMENSAJE 100
#define TAMNOMBRES 20

typedef struct Reserva
{
    char nombre_familia[20];
    int hora;
    int num_personas;
    int finalizo;
    //1 - Reserva ok
    //2 - Reserva garantizada para otras horas
    //3 - Reserva negada por tarde, reserva garantizada para otras horas.
    //4 - Reserva negada, debe volver otro día
    int mensaje_respuesta;

} reserva;

typedef struct Agente
{
    char nombre[20];
    char pipe_receptor[20];
    char pipe_emisor[20];
    int pid;

} agente;

typedef struct Horas
{
    int hora_inicial;
    int hora_final;
    int segundos_hora;

} horas;