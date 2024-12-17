/*
 * TITLE: P3 SISTEMAS OPERATIVOS
 * SUBTITLE: Practica 1
 * AUTHOR 1: ISMAEL BREA ARIAS          LOGIN 1: ismael.brea
 * AUTHOR 2: BORJA CASTELEIRO GOTI      LOGIN 2: borja.casteleiro
 * GROUP: 4.1
 */

#include "lista_memoria.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Inicaliza las listas de memoria
void InitMemoryManager(MemoryManager *manager) {
    manager->general.head = manager->general.tail = NULL;
    manager->mallocList.head = manager->mallocList.tail = NULL;
    manager->mmapList.head = manager->mmapList.tail = NULL;
    manager->sharedList.head = manager->sharedList.tail = NULL;
}

// Verifica si una dirección existe en la lista general
MemoryNode *FindMemoryBlock(MemoryManager *manager, void *addr) {
    MemoryNode *current = manager->general.head;
    while (current != NULL) {
        // Verificar si la dirección cae dentro del rango del bloque actual
        if (addr >= current->addr && addr < (current->addr + current->size)) {
            return current; // Retorna el nodo si encuentra un bloque que contiene la dirección
        }
        current = current->next;
    }
    return NULL; // Dirección no encontrada
}

// Inserta un nuevo nodo en la lista general
void InsertGeneralMemory(MemoryManager *manager, void *addr, size_t size, MemType type, int fd, int key) {
    MemoryNode *newNode = malloc(sizeof(MemoryNode));
    if (newNode == NULL) {
        fprintf(stderr, "Error: No se pudo asignar el nodo de memoria\n");
        return;
    }

    newNode->addr = addr;
    newNode->size = size;
    newNode->type = type;
    newNode->fd = fd; 
    newNode->key = key;
    time(&(newNode->timestamp));

    newNode->next = NULL;
    if (manager->general.tail == NULL) {
        manager->general.head = newNode;
    } else {
        manager->general.tail->next = newNode;
    }
    manager->general.tail = newNode;
}

// Inserta un nuevo nodo en la lista de memoria malloc
void InsertMallocMemory(MemoryManager *manager, void *addr, size_t size) {
    MemoryNode *newNode = malloc(sizeof(MemoryNode));
    if (newNode == NULL) {
        fprintf(stderr, "Error: No se pudo asignar el nodo de memoria para malloc\n");
        return;
    }

    newNode->addr = addr;
    newNode->size = size;
    newNode->type = MALLOC;
    time(&(newNode->timestamp)); // Guardamos la hora actual

    newNode->next = NULL;
    if (manager->mallocList.tail == NULL) {
        manager->mallocList.head = newNode;
    } else {
        manager->mallocList.tail->next = newNode;
    }
    manager->mallocList.tail = newNode;

    // También añadimos el bloque a la lista general
    InsertGeneralMemory(manager, addr, size, MALLOC,-1, -1);
}

// Inserta un nuevo nodo en la lista de memoria mmap
void InsertMmapMemory(MemoryManager *manager, void *addr, size_t size, int fd, const char *filename) {
    MemoryNode *newNode = malloc(sizeof(MemoryNode));
    if (newNode == NULL) {
        fprintf(stderr, "Error: No se pudo asignar el nodo de memoria para mmap\n");
        return;
    }

    newNode->addr = addr;
    newNode->size = size;
    newNode->type = MMAP;
    newNode->fd = fd;
    strncpy(newNode->filename, filename, MAX_FILENAME - 1);
    newNode->filename[MAX_FILENAME - 1] = '\0';  // Asegurar que la cadena esté terminada
    time(&(newNode->timestamp));

    newNode->next = NULL;
    if (manager->mmapList.tail == NULL) {
        manager->mmapList.head = newNode;
    } else {
        manager->mmapList.tail->next = newNode;
    }
    manager->mmapList.tail = newNode;

    // También añadimos el bloque a la lista general
    InsertGeneralMemory(manager, addr, size, MMAP,fd,-1);
}

// Inserta un nuevo nodo en la lista de memoria shared
void InsertSharedMemory(MemoryManager *manager, void *addr, size_t size, int key, const char *filename) {
    // Validación de entrada
    if (!manager || !addr || size == 0 || key < 0) {
        fprintf(stderr, "Error: Parámetros inválidos para InsertSharedMemory\n");
        return;
    }

    // Crear nuevo nodo
    MemoryNode *newNode = (MemoryNode *)malloc(sizeof(MemoryNode));
    if (!newNode) {
        perror("Error: No se pudo asignar el nodo de memoria para shared");
        return;
    }

    // Inicializar campos del nodo
    newNode->addr = addr;
    newNode->size = size;
    newNode->type = SHARED;
    newNode->key = key;
    strncpy(newNode->filename, filename ? filename : "", MAX_FILENAME - 1);
    newNode->filename[MAX_FILENAME - 1] = '\0';  // Asegurar terminación
    time(&(newNode->timestamp));
    newNode->next = NULL;

    // Insertar en la lista de memoria compartida
    if (!manager->sharedList.tail) {
        manager->sharedList.head = newNode;
    } else {
        manager->sharedList.tail->next = newNode;
    }
    manager->sharedList.tail = newNode;

    // También añadir a la lista general
    InsertGeneralMemory(manager, addr, size, SHARED, -1,key);
}

// Elimina un nodo de la lista general
void DeleteMemoryBlockFromGeneral(MemoryManager *manager, void *addr) {
    MemoryNode *current = manager->general.head;
    MemoryNode *prev = NULL;

    while (current != NULL) {
        if (current->addr == addr) {
            // Se encontró el bloque, eliminarlo de la lista
            if (prev == NULL) {
                manager->general.head = current->next;  // Eliminar el primer nodo
                if (manager->general.head == NULL) {
                    manager->general.tail = NULL;  // Si la lista queda vacía, actualizar tail
                }
            } else {
                prev->next = current->next;  // Eliminar el nodo intermedio
                if (current == manager->general.tail) {
                    manager->general.tail = prev;  // Si es el último nodo, actualizar tail
                }
            }
            free(current);  // Liberar el nodo de memoria
            return;
        }
        prev = current;
        current = current->next;
    }
}

// Elimina un bloque de memoria malloc
void DeleteMallocMemory(MemoryManager *manager, size_t size) {
    MemoryNode *current = manager->mallocList.head;
    MemoryNode *prev = NULL;

    while (current != NULL) {
        if (current->type == MALLOC && current->size == size) {
            void *addr = current->addr;  // Guardamos la dirección correcta

            if (prev == NULL) {
                manager->mallocList.head = current->next;
            } else {
                prev->next = current->next;
            }
            if (current == manager->mallocList.tail) {
                manager->mallocList.tail = prev;
            }
            
            // Eliminar de la lista general
            DeleteMemoryBlockFromGeneral(manager, addr);
            
            printf("Memoria de %zu bytes en %p liberada y eliminada de la lista de bloques malloc\n", size, addr);
            
            free(addr);  // Liberamos la memoria asignada
            free(current);  // Liberamos el nodo de la lista
            return;
        }
        prev = current;
        current = current->next;
    }
    
    printf("No se encontró un bloque malloc de tamaño %zu\n", size);
}

// Elimina un bloque de memoria mmap
void DeleteMmapMemory(MemoryManager *manager, const char *filename) {
    MemoryNode *current = manager->mmapList.head;
    MemoryNode *prev = NULL;

    while (current != NULL) {
        // Si es el nombre del archivo que estamos buscando
        if (strcmp(current->filename, filename) == 0) {
            if (prev == NULL) {
                manager->mmapList.head = current->next;
            } else {
                prev->next = current->next;
            }

            if (current == manager->mmapList.tail) {
                manager->mmapList.tail = prev;
            }

            // Eliminar de la lista general
            DeleteMemoryBlockFromGeneral(manager, current->addr);

            // Liberar la memoria mapeada
            if (munmap(current->addr, current->size) == -1) {
                perror("Error al liberar la memoria mapeada");
            } else {
                printf("Memoria mapeada en %p, de tamaño %zu, liberada eliminada de la lista de bloques mmap\n", current->addr, current->size);
            }

            // Liberar el nodo
            free(current);
            return;
        }
        current = current->next;
    }

    printf("No se encontró un bloque mmap con el archivo %s\n", filename);
}

// Elimina un bloque de memoria a partir de una dirección de memoria que pertenece a la lista mmap
void DeleteMmapMemorybyAddr(MemoryManager *manager, void *addr) {
    MemoryNode *current = manager->mmapList.head;
    MemoryNode *prev = NULL;

    // Buscar por dirección de memoria
    while (current != NULL) {
        if (current->addr == addr) {
            // Si se encuentra el bloque con la dirección, eliminamos de la lista
            if (prev == NULL) {
                manager->mmapList.head = current->next;  // Eliminar el primer nodo
            } else {
                prev->next = current->next;  // Eliminar el nodo intermedio
            }
            if (current == manager->mmapList.tail) {
                manager->mmapList.tail = prev;  // Actualizar tail si es el último nodo
            }

            // Eliminar de la lista general también
            DeleteMemoryBlockFromGeneral(manager, current->addr);

            // Intentar liberar la memoria mapeada
            if (munmap(current->addr, current->size) == -1) {
                perror("Error al liberar la memoria mapeada");
            } else {
                printf("Memoria mapeada en %p, de tamaño %zu, liberada y eliminada de la lista de bloques mmap\n", current->addr, current->size);
            }

            free(current);  // Liberar el nodo de memoria
            return;
        }
        prev = current;
        current = current->next;
    }

    // Si no se encuentra la dirección
    printf("No se encontró un bloque mmap en la dirección %p\n", addr);
}

// Desacopla y elimina un bloque de memoria compartida de la lista shared y general, pero la memoria sigue existiendo en el sistema 
void DeleteSharedMemory(MemoryManager *manager, key_t key) {
   MemoryNode *current = manager->sharedList.head;
    MemoryNode *prev = NULL;

    while (current != NULL) {
        if (current->key == key) {
            // Desacoplar la memoria compartida
            if (shmdt(current->addr) == -1) {
                perror("Error al desacoplar la memoria compartida");
            } else {
                printf("Memoria compartida en %p (clave %d) desacoplada y eliminada de la lista de bloques shared\n", current->addr, key);
            }

            // Eliminar de la lista de memoria compartida
            if (prev == NULL) {
                manager->sharedList.head = current->next;
            } else {
                prev->next = current->next;
            }
            if (current == manager->sharedList.tail) {
                manager->sharedList.tail = prev;
            }

            // Eliminar de la lista general
            DeleteMemoryBlockFromGeneral(manager, current->addr);

            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }

    printf("No se encontró un bloque de memoria compartida con la clave %d\n", key);
}

// Elimina la clave de memoria compartida del sistema (sin afectar a los procesos que ya tiene memoria adjunta)
void DeleteSharedMemoryByKey(MemoryManager *manager, key_t key) {
    int shmid = shmget(key, 0, 0);
    if (shmid == -1) {
        perror("Error al obtener el identificador de memoria compartida");
        return;
    }
    // Eliminar la clave de memoria compartida del sistema pero no afectará a sus procesos asociados
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Error al eliminar la clave de memoria compartida");
    } else {
        printf("Clave de memoria compartida %d eliminada del sistema\n", key);
    }
}

// Elimina un bloque de memoria compartida utilizando una dirección de memoria que pertenece a la lista shared
void DeleteMemSharedByAddr(MemoryManager *manager, void *addr) {
    MemoryNode *current = manager->sharedList.head;
    MemoryNode *prev = NULL;

    // Buscar el bloque de memoria compartida por dirección
    while (current != NULL) {
        if (current->addr == addr) {
            // Se encontró el bloque, eliminarlo de la lista de memoria compartida
            if (prev == NULL) {
                manager->sharedList.head = current->next;  // Eliminar el primer nodo
            } else {
                prev->next = current->next;  // Eliminar el nodo intermedio
            }
            if (current == manager->sharedList.tail) {
                manager->sharedList.tail = prev;  // Si es el último nodo, actualizar tail
            }

            // Eliminar de la lista general también
            DeleteMemoryBlockFromGeneral(manager, addr);

            // Desacoplar la memoria compartida
            if (shmdt(current->addr) == -1) {
                perror("Error al desacoplar la memoria compartida");
            } else {
                printf("Memoria compartida en %p desacoplada y eliminada de la lista de bloques shared\n", current->addr);
            }

            // Liberar el nodo de memoria
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }

    // Si no se encuentra el bloque de memoria
    printf("No se encontró un bloque de memoria compartida en la dirección %p\n", addr);
}


// Libera una lista de memoria específica
void DeleteMemorySublist(MemoryList *list) {
    MemoryNode *current = list->head;
    while (current != NULL) {
        MemoryNode *toDelete = current;
        current = current->next;
        free(toDelete);  // Libera cada nodo de la lista
    }
    list->head = NULL;
    list->tail = NULL;
}

// Liberar todas las listas de memoria
void DeleteMemoryList(MemoryManager *manager) {
    // Eliminar cada sublista usando el nombre correcto de la función
    DeleteMemorySublist(&manager->general);
    DeleteMemorySublist(&manager->mallocList);
    DeleteMemorySublist(&manager->mmapList);
    DeleteMemorySublist(&manager->sharedList);
}

// Limpiar los recursos de memoria compartida una vez que el programa termina
void LimpiarRecursosMemoriaCompartida(MemoryManager *manager) {
  MemoryNode *current = manager->sharedList.head;
  MemoryNode *next;

  while (current != NULL) {
      next = current->next;

      // Desacoplar la memoria compartida
      if (shmdt(current->addr) == -1) {        // shmdt desacopla la memoria compartida
          perror("Error al desacoplar la memoria compartida");
      }

      // Eliminar el segmento de memoria compartida del sistema
      int shmid = shmget(current->key, 0, 0);
      if (shmid != -1) {
          if (shmctl(shmid, IPC_RMID, NULL) == -1) {
              perror("Error al eliminar el segmento de memoria compartida");
          } else {
              printf("Segmento de memoria compartida con clave %d eliminado del sistema\n", current->key);
          }
      }

      current = next;
  }

  // Limpiar la lista de memoria compartida
  manager->sharedList.head = NULL;
  manager->sharedList.tail = NULL;
}

void printTime(char *buffer, time_t timestamp) {
    struct tm *tm_info = localtime(&timestamp);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
}

void ImprimirListaMemoria(MemoryList *list) {
    MemoryNode *current = list->head;
    while (current) {
        char time_buffer[26];
        printTime(time_buffer, current->timestamp);
        
        if (current->type == MALLOC) {
            printf("  %p             %zu %s malloc\n", current->addr, current->size, time_buffer);
        } else if (current->type == MMAP) {
            printf("  %p             %zu %s %s  (descriptor %d) mmap\n", 
                   current->addr, current->size, time_buffer, current->filename, current->fd);
        } else if (current->type == SHARED) {
            printf("  %p             %zu %s  (key %d) shared\n", 
                   current->addr, current->size, time_buffer, current->key);
        }
        current = current->next;
    }
}

void ImprimirListaMalloc(MemoryList *list) {
    MemoryNode *current = list->head;
    while (current && current->type == MALLOC) {
        char time_buffer[26];
        printTime(time_buffer, current->timestamp);
        printf("  %p             %zu %s malloc\n", current->addr, current->size, time_buffer);
        current = current->next;
    }
}

void ImprimirListaMmap(MemoryList *list) {
    MemoryNode *current = list->head;
    while (current) {
        if (current->type == MMAP) {
            char time_buffer[26];
            printTime(time_buffer, current->timestamp);
            printf("  %p             %zu %s %s  (descriptor %d) mmap\n", 
                   current->addr, current->size, time_buffer, current->filename, current->fd);
        }
        current = current->next;
    }
}

void ImprimirListaShared(MemoryList *list) {
    MemoryNode *current = list->head;
    while (current && current->type == SHARED) {
        char time_buffer[26];
        printTime(time_buffer, current->timestamp);
        printf("  %p             %zu %s shared (key %d)\n", current->addr, current->size, time_buffer, current->key);
        current = current->next;
    }
}
