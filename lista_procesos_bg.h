/*
 * TITLE: P3 SISTEMAS OPERATIVOS
 * SUBTITLE: Practica 1
 * AUTHOR 1: ISMAEL BREA ARIAS          LOGIN 1: ismael.brea
 * AUTHOR 2: BORJA CASTELEIRO GOTI      LOGIN 2: borja.casteleiro
 * GROUP: 4.1
 */

#ifndef LISTA_PROCESOS_BG_H
#define LISTA_PROCESOS_BG_H

#include <time.h>
#include <sys/types.h>
#include <pwd.h>

////////DEFINICIÓN DE LOS TIPOS DE DATOS A USAR DE UNA LISTA SIMPLEMENTE ENLAZADA PARA USAR EN LAS FUNCIONES DE PROCESOS EN SEGUNDO PLANO(P3)////////

typedef enum {     
    FINISHED,
    STOPPED,
    SIGNALED,
    ACTIVE
} ProcessStatus;

// Estructura para representar un proceso en segundo plano
typedef struct BackgroundProcess {
    pid_t pid;               // PID del proceso
    time_t launch_time;      // Tiempo de lanzamiento (hora de inicio)
    char user[50];           // Usuario que lanzó el proceso
    ProcessStatus status;    // Estado del proceso
    int return_value;        // Valor de retorno o señal
    char* command_line;      // Línea de comando ejecutada
    int priority;            // Prioridad del proceso
    struct BackgroundProcess* next;     // Puntero al siguiente proceso
} BackgroundProcess;

// Funciones para manejar la lista de procesos en segundo plano
void Init_backgroundProcesses(BackgroundProcess*);  // Inicializa la lista
void Add_backgroundProcess(BackgroundProcess*, pid_t, const char*,int); // Añade un proceso
void Update_processStatus(BackgroundProcess*, pid_t, ProcessStatus, int); // Actualiza el estado de un proceso
void Remove_backgroundProcess(BackgroundProcess*, pid_t); // Elimina un proceso de la lista
void Imprimir_backgroundProcesses(BackgroundProcess*);  // Imprime los procesos en segundo plano
void Delete_backgroundProcesses(BackgroundProcess*);  // Elimina todos los procesos de la lista

#endif 