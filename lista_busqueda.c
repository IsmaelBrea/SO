#include "lista_busqueda.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Inicializa la lista de búsqueda
void inicializarListaBusqueda(ListaBusqueda *lista) {
    lista->cabeza = NULL;
    lista->actual = NULL;
}

// Función para verificar si un directorio ya está en la lista
bool directorioExiste(ListaBusqueda *lista, const char *directorio) {
    NodoBusqueda *actual = lista->cabeza;
    while (actual != NULL) {
        if (strcmp(actual->directorio, directorio) == 0) {
            return true;  // El directorio ya existe
        }
        actual = actual->siguiente;
    }
    return false;  // El directorio no está en la lista
}

// Agrega un directorio a la lista de búsqueda
void agregarDirectorio(ListaBusqueda *lista, const char *directorio) {
    // Verificar si el directorio ya está en la lista
    if (directorioExiste(lista, directorio)) {
        printf("El directorio '%s' ya está en la lista\n", directorio);
        return;
    }

    // Crear un nuevo nodo
    NodoBusqueda *nuevoNodo = malloc(sizeof(NodoBusqueda));
    if (nuevoNodo == NULL) {
        perror("Error al asignar memoria para nuevo nodo");
        return;
    }

    nuevoNodo->directorio = strdup(directorio);
    if (nuevoNodo->directorio == NULL) {
        perror("Error al duplicar el directorio");
        free(nuevoNodo);
        return;
    }

    nuevoNodo->siguiente = NULL;

    // Si la lista está vacía, el nuevo nodo será la cabeza
    if (lista->cabeza == NULL) {
        lista->cabeza = nuevoNodo;
    } else {
        // Buscar el último nodo de la lista
        NodoBusqueda *actual = lista->cabeza;
        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }
        // Insertar el nuevo nodo al final
        actual->siguiente = nuevoNodo;
    }
    printf("Directorio '%s' añadido a la lista de búsqueda\n", directorio);
}

// Elimina un directorio de la lista de búsqueda
bool eliminarDirectorio(ListaBusqueda *lista, const char *directorio) {
    NodoBusqueda *actual = lista->cabeza;
    NodoBusqueda *anterior = NULL;

    while (actual != NULL) {
        if (strcmp(actual->directorio, directorio) == 0) {
            if (anterior == NULL) {
                lista->cabeza = actual->siguiente;
            } else {
                anterior->siguiente = actual->siguiente;
            }
            free(actual->directorio);
            free(actual);
            return true;
        }
        anterior = actual;
        actual = actual->siguiente;
    }
    return false;
}

// Limpia la lista de búsqueda
void limpiarListaBusqueda(ListaBusqueda *lista) {
    NodoBusqueda *actual = lista->cabeza;
    while (actual != NULL) {
        NodoBusqueda *siguiente = actual->siguiente;
        free(actual->directorio);
        free(actual);
        actual = siguiente;
    }
    lista->cabeza = NULL;
    lista->actual = NULL;
}

// Muestra la lista de búsqueda
void mostrarListaBusqueda(const ListaBusqueda *lista) {
    NodoBusqueda *actual = lista->cabeza;
    if (actual == NULL) {
        printf("No hay directorios en la lista de búsqueda\n");
        return;
    }
    printf("Directorios en la lista de búsqueda:\n");
    while (actual != NULL) {
        printf("- %s\n", actual->directorio);
        actual = actual->siguiente;
    }
}

// Itera sobre la lista de búsqueda para obtener el primer directorio
char *SearchListFirst(ListaBusqueda *lista) {
    lista->actual = lista->cabeza;
    return (lista->actual != NULL) ? lista->actual->directorio : NULL;
}

// Itera sobre la lista de búsqueda para obtener el siguiente directorio
char *SearchListNext(ListaBusqueda *lista) {
    if (lista->actual != NULL) {
        lista->actual = lista->actual->siguiente;
        return (lista->actual != NULL) ? lista->actual->directorio : NULL;
    }
    return NULL;
}

// Importa los directorios del PATH a la lista de búsqueda
void importarPATH(ListaBusqueda *lista) {
    // Obtener la variable de entorno PATH
    char *path = getenv("PATH");
    if (path == NULL) { // Verificar si la variable PATH existe
        printf("No se pudo obtener la variable PATH\n");
        return;
    }

    // Dividir la cadena PATH en tokens usando ':' como delimitador
    char *token = strtok(path, ":");
    while (token != NULL) { 
        agregarDirectorio(lista, token); // Agregar cada directorio a la lista
        token = strtok(NULL, ":");       // Obtener el siguiente token (directorio)
    }
}
