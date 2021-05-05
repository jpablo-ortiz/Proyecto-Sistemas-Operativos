#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include "estructuras.h"

sem_t sem_hora, sem_termino;
int hora_global, termino_global = 0;

void simulacionHoras(horas *horas)
{
    sem_wait(&sem_hora);
    hora_global = horas->horaInicial;
    sem_post(&sem_hora);

    for (int i = horas->horaInicial; i < horas->hora_final; i++)
    {
        sleep(horas->segundosHora);
        sem_wait(&sem_hora);
        hora_global++;
        sem_post(&sem_hora);
    }

    sem_wait(&sem_termino);
    termino_global = 1;
    sem_post(&sem_termino);

    pthread_exit(NULL);
}

int main()
{
    pthread_t thread1;
    horas horas;
    horas.horaInicial = 7;
    horas.hora_final = 10;
    horas.segundosHora = 2;

    sem_init(&sem_hora, 1, 1);    //Ver respuesta int
    sem_init(&sem_termino, 1, 1); //Ver respuesta int

    if (pthread_create(&thread1, NULL, (void *)simulacionHoras, (void *)&horas))
    {
        printf("Error en la ejecución del hilo simulador de horas");
    }

    int termino = 0;
    while (!termino)
    {
        sleep(0.2);
        sem_wait(&sem_hora);
        printf("%d\n", hora_global);
        sem_post(&sem_hora);

        sem_wait(&sem_termino);
        termino = termino_global;
        sem_post(&sem_termino);
    }

    pthread_join(thread1, NULL); //Ver respuesta int
    sem_destroy(&sem_hora);       //Ver respuesta int
    sem_destroy(&sem_termino);    //Ver respuesta int
    exit(0);
}