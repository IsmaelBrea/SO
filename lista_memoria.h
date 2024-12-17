/*
 * TITLE: P3 SISTEMAS OPERATIVOS
 * SUBTITLE: Practica 1
 * AUTHOR 1: ISMAEL BREA ARIAS          LOGIN 1: ismael.brea
 * AUTHOR 2: BORJA CASTELEIRO GOTI      LOGIN 2: borja.casteleiro
 * GROUP: 4.1
 */

#ifndef LISTA_MEMORIA_H
#define LISTA_MEMORIA_H

#include <stddef.h>    
#include <time.h>
#include <sys/mman.h>   // Para usar mmap
#include <sys/types.h>  // Para tipos de datos como key_t
#include <sys/shm.h>    // Para funciones de memoria compartida

#define MAX_FILENAME 256

////////DEFINICIÓN DE LOS TIPOS DE DATOS A USAR DE UNA LISTA SIMPLEMENTE ENLAZADA PARA USAR EN LAS FUNCIONES DE MEMORIA(P2)////////

typedef enum {
    MALLOC,
    MMAP,
    SHARED
} MemType;

// Estructura del nodo de la lista
typedef struct MemoryNode {
    void *addr;           // Dirección de memoria
    size_t size;          // Tamaño de la asignación
    time_t timestamp;     // Marca de tiempo de la asignación
    int fd;               // Descriptor de archivo (solo para mmap)
    int key;              // Clave de memoria compartida (solo para shared)
    MemType type;         // Tipo de asignación (malloc, mmap, shared)
    char filename[MAX_FILENAME]; // Nombre del archivo (solo para mmap)
    struct MemoryNode *next; // Siguiente nodo
} MemoryNode;

// Estructura de la lista de memoria
typedef struct {
    MemoryNode *head;
    MemoryNode *tail;
} MemoryList;

// Estructura que contiene las listas de memoria
typedef struct {
    MemoryList general;
    MemoryList mallocList;
    MemoryList mmapList;
    MemoryList sharedList;
} MemoryManager;

// Funciones para gestionar la lista
void InitMemoryManager(MemoryManager *);
MemoryNode *FindMemoryBlock(MemoryManager *, void *);
void InsertGeneralMemory(MemoryManager *, void *, size_t, MemType, int , int);
void InsertMallocMemory(MemoryManager *, void *, size_t);
void InsertMmapMemory(MemoryManager *, void *, size_t, int, const char *);
void InsertSharedMemory(MemoryManager *, void *, size_t, int, const char *);
void DeleteMemoryBlockFromGeneral(MemoryManager *, void *);
void DeleteMallocMemory(MemoryManager *, size_t);
void DeleteMmapMemory(MemoryManager *, const char *);
void DeleteMmapMemorybyAddr(MemoryManager *, void *);
void DeleteSharedMemory(MemoryManager *, key_t);
void DeleteMemSharedByAddr(MemoryManager *, void *);
void DeleteSharedMemoryByKey(MemoryManager *, key_t);
void DeleteMemoryList(MemoryManager *);
void DeleteMemorySublist(MemoryList *);
void LimpiarRecursosMemoriaCompartida(MemoryManager *);

// Funciones de impresión
void ImprimirListaMemoria(MemoryList *);
void ImprimirListaMalloc(MemoryList *);
void ImprimirListaMmap(MemoryList *);
void ImprimirListaShared(MemoryList *);

#endif // LISTA_MEMORIA_H
