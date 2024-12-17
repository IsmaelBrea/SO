#ifndef LISTA_BUSQUEDA_H
#define LISTA_BUSQUEDA_H

#include <stdbool.h>

////////DEFINICIÓN DE LOS TIPOS DE DATOS A USAR DE UNA LISTA SIMPLEMENTE ENLAZADA PARA USAR EN LAS FUNCIONES DE BÚSQUEDA DE ARCHIVOS EJECUTABLES (similar a PATH) PLANO(P3)////////

// Estructura para un nodo de la lista
typedef struct NodoBusqueda {
    char *directorio;
    struct NodoBusqueda *siguiente;
} NodoBusqueda;

// Estructura para la lista de búsqueda
typedef struct {
    NodoBusqueda *cabeza;
    NodoBusqueda *actual; // Para iterar sobre la lista
} ListaBusqueda;


// Funciones para manejar la lista de búsqueda
void inicializarListaBusqueda(ListaBusqueda *);
bool directorioExiste(ListaBusqueda *, const char *);
void agregarDirectorio(ListaBusqueda *, const char *);
bool eliminarDirectorio(ListaBusqueda *, const char *);
void limpiarListaBusqueda(ListaBusqueda *);
void mostrarListaBusqueda(const ListaBusqueda *);

// Funciones para iterar sobre la lista
char *SearchListFirst(ListaBusqueda *);
char *SearchListNext(ListaBusqueda *);

// Función para importar directorios del PATH
void importarPATH(ListaBusqueda *);

#endif // LISTA_BUSQUEDA_H