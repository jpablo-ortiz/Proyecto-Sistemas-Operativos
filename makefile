# PROYECTO #1 - Makefile para generar 2 ejecutables

all: controlador agente

controlador: controlador.o pipes.o
	gcc controlador.o pipes.o -o controlador -pthread

agente: agente.o pipes.o
	gcc agente.o pipes.o -o agente

agente.o: agente.c
	gcc -c agente.c

pipes.o: pipes.c
	gcc -c pipes.c

clean: rm *.o controlador agente
