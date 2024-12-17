/*
 * TITLE: P3 SISTEMAS OPERATIVOS
 * SUBTITLE: Practica 1
 * AUTHOR 1: ISMAEL BREA ARIAS          LOGIN 1: ismael.brea
 * AUTHOR 2: BORJA CASTELEIRO GOTI      LOGIN 2: borja.casteleiro
 * GROUP: 4.1
 */
#ifndef LIST_H
#define LIST_H

#define LNULL NULL
#define MAX_LENGTH 1024  

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

////////DEFINICIÓN DE LOS TIPOS DE DATOS A USAR DE UNA LISTA SIMPLEMENTE ENLAZADA PARA USAR EN LA FUNCIÓN HISTORIC////////

typedef int PositionNumber;

//Item almacena el número de posición y el comando introducido en un array de strings
typedef struct {
    PositionNumber posNum;
    char data[MAX_LENGTH][MAX_LENGTH];
} Item;

//Nodo de la lista enlazada
typedef struct Node* Position;

//Definición de la lista (historial de comandos)
struct Node{
    Item item;
    Position next;
};

typedef Position List;

/////DECLARACIÓN DE FUNCIONES BÁSICAS PARA OPERAR CON EL TAD/////
void createEmptyList(List *);
bool isEmptyList(List);
void deleteList(List *);
Position first(List);
Position last(List);
bool insertToList(Item, List *);
Position findItem(PositionNumber, List);
Item getItem(Position, List);
void printList(Position , List, int);


#endif