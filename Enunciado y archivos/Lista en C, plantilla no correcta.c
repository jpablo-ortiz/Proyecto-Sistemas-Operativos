#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#include<alloc.h>

Aquí damos a conocer la Asignación de una lista como una estructura que almacena datos

struct nodo{
   int valor;        // Valor que va tener la estructura en cada uno en este caso es un numero entero
   struct nodo *siguiente; //Apuntador hacia el siguiente nodo o enlace
};

typedef struct nodo *TipoLista; //Definicion del nombre de la lista


//declaracion de funciones que posee la listas
void Insertar(TipoLista &l, int v  ); 
void EliminarRegistro(TipoLista &l, int v);
void VaciarLista(TipoLista &l);
int  ListaVacia(TipoLista l);
void MostrarLista(TipoLista l);
void Modificar(TipoLista l, int v,int n);

//*****************************

int main()
{
   TipoLista lista = NULL; Inicio de listas en Nulo
   int op,x,n;
   TipoLista p;
  
   do
   {
      clrscr(); //MENU DE OPCIONES
      printf("***MENU***\n\n");
      printf("1. Agregar dato\n");
      printf("2. Modificar dato\n");
      printf("3. Mostrar datos\n");
      printf("4. Eliminar dato\n");
      printf("5. Vaciar lista\n");
      printf("6. Salir\n\n");
      printf("Digite la opcion: ");
      scanf("%d",&op);
   
      switch(op)
      {
         case 1:
            do {  // SE CREAUNA LISTA HASTA QUE EL USUARIO PRECIONE 0
               clrscr();
               printf("Digite  Cero (0) para salir\n");
               printf("Digite el dato que desea agregar:");
               scanf("%d",&x);
               if (x!=0)
                  Insertar(lista,x);//ENVIO DE VALOR AL PROCESO DE INSERTAR DATOS A LISTA
            }while (x!=0);
         
            getch();|
            break;
      
         case 2:
            MostrarLista(lista);   
            printf("\n Digite el dato que desea modificar:");
            scanf("%d",&x);            //MODIFICACION DE DATOS
            printf("\n Digite el dato nuevo:");
            scanf("%d",&n);    //LEE NUEVO DATO
            Modificar(lista,x,n); //ENVIO A FUNCION DE ENCONTRAR DATO Y CAMBIAR A ACTUAL
            getch();
            break;
      
         case 3:
            MostrarLista(lista);
            getch();
            break;
      
         case 4:
            MostrarLista(lista);
            printf("Digite el dato que desea eliminar:");
            scanf("%d",&x);
            EliminarRegistro(lista,x);  //ELIMINA DATO QUE  VERIFICA EL USUARIO
            getch();
            break;
      
         case 5:
          //  BorrarLista(lista);
            break;
      
         default:
            printf("Opcion no valida!\n");
            getch();
      }
   
   
   } while(op!=6);


   getchar();
   return 0;
}



//***********
//Lista Vacia
int ListaVacia(TipoLista lista)
{
   return (lista == NULL);
}
 //**************************
//Insertar registro
void Insertar(TipoLista &lista, int valor)
{
   TipoLista nuevo;
   nuevo = new(struct nodo);
   nuevo->valor = valor;
   nuevo->siguiente = lista;
   lista  = nuevo;
}
 //**************************
//Imprimir lista
void MostrarLista(TipoLista lista)
{
   TipoLista nodo = lista;

   if(ListaVacia(nodo)) printf("Lista vacia\n");
   else {
      while(nodo) {
         printf("%d -> ", nodo->valor);
         nodo = nodo->siguiente;
      }
      printf("\n");
   }
}
 //**************************
//Modificar
void Modificar(TipoLista lista, int v,int n)
{
   TipoLista nodo;
   int ban = 0;
   nodo = lista;

   while(nodo)
   {
      if(nodo->valor == v)
      {
         nodo->valor=n;
         ban=1;
         printf("Registro Modificado\n");
         MostrarLista(lista);
      }
      nodo = nodo->siguiente;
   }
   if(ban== 0)
      printf("No se encontro el registro\n");

}
 //**************************
//Eliminar
void EliminarRegistro(TipoLista &lista, int v)
{
   TipoLista nodo, anterior;
   nodo= lista;
   int ban =0;

   if(lista!=NULL)
   {
      while(nodo!=NULL)
      {
         if(nodo->valor==v)
         {
            ban=1;
            printf("Registro Eliminado\n");
            if(nodo==lista)
               lista = lista->siguiente;
            else
               anterior->siguiente = nodo->siguiente;
         
            delete(nodo);
            return;
         }
         anterior = nodo;
         nodo =nodo->siguiente;
      }
   }
   else
      printf(" Lista vacia..!\n");

   if(ban==0 )
      printf("No se encontro el registro\n");
}

//Vaciar toda la lista
void VaciarLista(TipoLista &lista)
{
   TipoLista nodo;
   // nodo= lista;

   if(lista!=NULL)
   {
      while(lista!=NULL)
      {
         nodo =lista;
         lista = lista->siguiente;
         delete(nodo);
      
      }
   }
   else
      printf(" Lista vacia..!\n");

}