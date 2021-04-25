# PROYECTO #1 - Makefile para generar 2 ejecutables

all: controlador agente

controlador: controlador.o
	gcc controlador.o -o controlador

controlador.o: controlador.c
	gcc -c controlador.c

agente: agente.o
	gcc agente.o -o agente

agente.o: agente.c
	gcc -c agente.c

clean: rm *.o controlador agente
