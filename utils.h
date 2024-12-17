/*
 * TITLE: P3 SISTEMAS OPERATIVOS
 * SUBTITLE: Practica 1
 * AUTHOR 1: ISMAEL BREA ARIAS          LOGIN 1: ismael.brea
 * AUTHOR 2: BORJA CASTELEIRO GOTI      LOGIN 2: borja.casteleiro
 * GROUP: 4.1
 */

#ifndef UTILS_H
#define UTILS_h

#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h> // Para mode_t
#include <sys/stat.h>  // Para la estructura stat
#include <time.h>      // Para manejar fechas y horas
#include <unistd.h>    // Para funciones de POSIX como stat
#include <string.h>    // Para funciones de manipulación de cadenas
#include <stdlib.h>    // Para funciones de utilidad general
#include <errno.h>     // Para manejar errores
#include <pwd.h>       // Para obtener información del propietario del archivo
#include <grp.h>       // Para obtener información del grupo del archivo
#include <limits.h>    // Para obtener constantes como PATH_MAX
#include <dirent.h>    // Para manipular directorios (uso de DIR)
#include "lista_memoria.h" 
#include <fcntl.h>     // Para funciones de control de archivos (open, close, etc.)
#include <sys/mman.h>  // Para usar PROT_READ, PROT_WRITE, PROT_EXEC
#include "lista_ficheros.h" // Para trabajar con la lista de ficheros abiertos
#include <sys/wait.h>     // Para usar waitpid
#include <sys/ipc.h>     // Para claves IPC
#include <sys/shm.h>    // Para usar shmget, shmat, shmdt, shmctl
#include <unistd.h>     // Para usar fork, execvp, getpid, getppid, environ
#include "lista_busqueda.h"
#include <sys/resource.h> // Para usar getpriority
#include <signal.h>      // Para manejar señales

#define MAXVAR 256

extern MemoryManager memoria; // Declaramos la lista para manejar la memoria

////////DEFINICIÓN DE LAS FUNCIONES AUXILIARES Y DE AYUDA QUE UTILIZAN LOS COMANDOS////////

// ====== AUXILIARES P0 ====== //
// Función que se utilizará en la función historic para leer el número de comandos que se van a imprimir cuando se pasa como argumento -num
int isNumber(char *);


// ====== AUXILIARES P1 ====== //
char LetraTF(mode_t);
char *ConvierteModo(mode_t, char *);
void printFileInfo(char *, int, int, int);


// ====== AUXILIARES P2 ====== //
void *MapearFichero(char *, int, int *);
void do_AllocateMmap(char *[], MemoryManager *);
void *ObtenerMemoriaShmget(key_t, size_t);
void do_AllocateCreateshared(char *[]);
void do_AllocateShared(char *[]);
void *cadtop(char *);
void LlenarMemoria(void *, size_t, unsigned char);
int readByte(void *, int *);
void Do_pmap(void);
ssize_t LeerFichero(char *, void *, size_t);


// ====== AUXILIARES P3 ====== //
int BuscarVariable(char *, char *[]);
int CambiarVariable(char *, char *, char *[]);
char *Ejecutable(ListaBusqueda *, char *);
int Execpve(ListaBusqueda *, char *[], char **, int *);
int ValorSenal(char *);
char *NombreSenal(int);

#endif 