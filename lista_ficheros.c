/*
 * TITLE: P3 SISTEMAS OPERATIVOS
 * SUBTITLE: Practica 1
 * AUTHOR 1: ISMAEL BREA ARIAS          LOGIN 1: ismael.brea
 * AUTHOR 2: BORJA CASTELEIRO GOTI      LOGIN 2: borja.casteleiro
 * GROUP: 4.1
 */

#include "lista_ficheros.h"

//Función que inicaliza una lista vacía de ficheros abiertos
void createEmptyListFicheros(ListFicheros *list) {
    *list = NULL; // Inicializa la lista vacía
}

//Función que elimina una lista de ficheros abiertos
void deleteListFicheros(ListFicheros *list) {
    PositionFichero actual = *list;
    PositionFichero next;
    // Liberar cada nodo en la lista
    while (actual != NULL) {
        next = actual->next;
        free(actual);
        actual = next;
    }
    *list = NULL; // Asigna NULL a la lista
}

//Función que comprueba si una lista de ficheros abiertos está vacía
bool isEmptyListFicheros(ListFicheros list) {
    return list == NULL; // Verifica si la lista es NULL
}

//Función que muestra de manera enumerada los ficheros que están abiertos en el shell
void listarFicherosAbiertos(ListFicheros list) {
    if (isEmptyListFicheros(list)) {
        printf("No hay ficheros abiertos.\n");
        return;
    }

    printf("--- Archivos Abiertos ---\n");
    PositionFichero actual = list;
    while (actual != NULL) {
        printf("Descriptor: %d, Nombre: %s, Modo: %d\n", 
            actual->item.descriptor, 
            actual->item.nombre, 
            actual->item.modo);
        actual = actual->next; // Avanza al siguiente nodo
    }
}

//Función que añade ficheros a la lista de ficheros
bool anadirAFicherosAbiertos(int df, int modo, const char *nombre, ListFicheros *list) {
    PositionFichero nuevoNodo = malloc(sizeof(struct NodeFichero));
    if (nuevoNodo == NULL) return false; // Verifica si se pudo asignar memoria

    // Inicializa el nuevo nodo
    nuevoNodo->item.descriptor = df;
    strncpy(nuevoNodo->item.nombre, nombre, MAXNAME - 1);
    nuevoNodo->item.nombre[MAXNAME - 1] = '\0'; // Asegura que la cadena termine en '\0'
    nuevoNodo->item.modo = modo;
    nuevoNodo->next = *list; // Enlaza el nuevo nodo al inicio de la lista
    *list = nuevoNodo; // Asigna el nuevo nodo como el inicio de la lista

    return true; // Inserción exitosa
}


//Función que elimina un archivo de la lista de archivos abiertos
bool eliminarDeFicherosAbiertos(int df, ListFicheros *list) {
    PositionFichero actual = *list;
    PositionFichero anterior = NULL;

    // Busca el nodo a eliminar
    while (actual != NULL) {
        if (actual->item.descriptor == df) {
            if (anterior == NULL) {
                // Es el primer nodo
                *list = actual->next;
            } else {
                anterior->next = actual->next; // Desvincula el nodo de la lista
            }
            free(actual); // Libera la memoria del nodo
            return true; // Eliminación exitosa
        }
        anterior = actual;
        actual = actual->next;
    }
    return false; // No se encontró el descriptor
}

//Función que obtiene el nombre del archivo asociado a un descriptor de archivo
const char* nombreFicheroDescriptor(int df, ListFicheros list) {
    PositionFichero actual = list;

    while (actual != NULL) {
        if (actual->item.descriptor == df) {
            return actual->item.nombre; // Devuelve el nombre del archivo
        }
        actual = actual->next;
    }
    return NULL; // No se encontró el descriptor
}

// Función que lista los descriptores de archivo que están en uso en el sistema, junto con su tipo y modo de operación
void listarDescriptores() {
    for (int i = 0; i < 20; i++) {
        int flags = fcntl(i, F_GETFL);
        if (flags != -1) {
            const char* nombre = nombreFicheroDescriptor(i, ficheros);
            char tipo[50] = "";
            char modo[10] = "";
            char offset[10] = "  ";

            if (i <= 2) {
                // Descriptores estándar
                switch(i) {
                    case 0: strcpy(tipo, "entrada estandar"); break;
                    case 1: strcpy(tipo, "salida estandar"); break;
                    case 2: strcpy(tipo, "error estandar"); break;
                }
                strcpy(modo, "O_RDWR");
            } else if (nombre != NULL) {
                // Descriptores de archivos abiertos o mapeados
                strncpy(tipo, nombre, sizeof(tipo) - 1);
                tipo[sizeof(tipo) - 1] = '\0';

                off_t current_offset = lseek(i, 0, SEEK_CUR);
                if (current_offset != -1) {
                    snprintf(offset, sizeof(offset), "(%lld)", (long long)current_offset);
                } else {
                    strcpy(offset, "(0)");  // Default to 0 if lseek fails
                }

                if (flags & O_RDWR) strcpy(modo, "O_RDWR");
                else if (flags & O_RDONLY) strcpy(modo, "O_RDONLY");
                else if (flags & O_WRONLY) strcpy(modo, "O_WRONLY");
            } else {
                // Descriptor no usado
                strcpy(tipo, "no usado");
            }

            printf("descriptor: %d, offset: (%s)-> %s %s\n", i, offset, tipo, modo);
        } else {
            // Descriptor no disponible
            printf("descriptor: %d, offset: (  )-> no usado\n", i);
        }
    }
}