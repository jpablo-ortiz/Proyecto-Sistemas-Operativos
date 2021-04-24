# PROYECTO #1 - Makefile para generar 2 ejecutables
# 

all: parqueH parqueP

parqueH: TomarTiempo.o parqueH.o
	gcc TomarTiempo.o parqueH.o -o parqueH -lpthread

parqueH.o: parqueH.c TomarTiempo.c TomarTiempo.h
	gcc -c parqueH.c -lpthread

parqueP: TomarTiempo.o parqueP.o
	gcc TomarTiempo.o parqueP.o -o parqueP

parqueP.o: parqueP.c TomarTiempo.c TomarTiempo.h
	gcc -c parqueP.c

TomarTiempo.o: TomarTiempo.c
	gcc -c TomarTiempo.c

clean: rm *.o parqueP parqueH
