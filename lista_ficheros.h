/*
 * TITLE: P3 SISTEMAS OPERATIVOS
 * SUBTITLE: Practica 1
 * AUTHOR 1: ISMAEL BREA ARIAS          LOGIN 1: ismael.brea
 * AUTHOR 2: BORJA CASTELEIRO GOTI      LOGIN 2: borja.casteleiro
 * GROUP: 4.1
 */


#ifndef LISTA_FICHEROS_H
#define LISTA_FICHEROS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>  // Para fcntl y F_GETFL
#include <unistd.h> // Para close

#define MAXNAME 256 // Definir el tamaño máximo para nombres de archivos

////////DEFINICIÓN DE LOS TIPOS DE DATOS A USAR DE UNA LISTA SIMPLEMENTE ENLAZADA PARA USAR EN LAS FUNCIONES OPEN,CLOSE Y DUP////////

//Estructura que almacena la información sobre un fichero
typedef struct {
    int descriptor;  // Descriptor de archivo
    char nombre[MAXNAME]; // Nombre del archivo
    int modo; // Modo de apertura
} FicheroAbierto;

typedef struct NodeFichero* PositionFichero;
struct NodeFichero {
    FicheroAbierto item;
    PositionFichero next;
};

typedef PositionFichero ListFicheros;

extern ListFicheros ficheros;

////DECLARACIÓN DE LAS FUNCIONES BÁSICAS PARA OPERAR CON EL TAD////
void createEmptyListFicheros(ListFicheros *);
void deleteListFicheros(ListFicheros *);
bool isEmptyListFicheros(ListFicheros);
void listarFicherosAbiertos(ListFicheros);
bool anadirAFicherosAbiertos(int, int, const char *, ListFicheros *);
bool eliminarDeFicherosAbiertos(int, ListFicheros *);
const char* nombreFicheroDescriptor(int, ListFicheros);
void listarDescriptores();

#endif
