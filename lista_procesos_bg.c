/*
 * TITLE: P3 SISTEMAS OPERATIVOS
 * SUBTITLE: Practica 1
 * AUTHOR 1: ISMAEL BREA ARIAS          LOGIN 1: ismael.brea
 * AUTHOR 2: BORJA CASTELEIRO GOTI      LOGIN 2: borja.casteleiro
 * GROUP: 4.1
 */

#include "lista_procesos_bg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>

// Inicializa la lista de procesos en segundo plano (ahora usa el primer nodo, no puntero a puntero)
void Init_backgroundProcesses(BackgroundProcess* list) {
    list->next = NULL;  // Establece el primer nodo como vacío (inicio de la lista)
}

// Añade un proceso a la lista
void Add_backgroundProcess(BackgroundProcess* list, pid_t pid, const char* command_line, int priority) {
    BackgroundProcess* new_process = malloc(sizeof(BackgroundProcess));
    if (new_process == NULL) {
        perror("Error al asignar memoria para el nuevo proceso");
        return;
    }

    new_process->pid = pid;
    new_process->launch_time = time(NULL);
    new_process->status = ACTIVE;
    new_process->return_value = 0;
    new_process->command_line = strdup(command_line);
    new_process->priority = priority;
    new_process->next = NULL;

    // Obtener el nombre de usuario
    struct passwd *pw = getpwuid(getuid());
    if (pw != NULL) {
        strncpy(new_process->user, pw->pw_name, sizeof(new_process->user) - 1);
        new_process->user[sizeof(new_process->user) - 1] = '\0';
    } else {
        strcpy(new_process->user, "unknown");
    }

    // Insertar al final de la lista
    if (list->next == NULL) {
        list->next = new_process;
    } else {
        BackgroundProcess* current = list->next;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_process;
    }
}

// Actualiza el estado de un proceso en la lista
void Update_processStatus(BackgroundProcess* list, pid_t pid, ProcessStatus status, int return_value) {
    BackgroundProcess* current = list->next;  // Empezar desde el primer proceso real
    while (current != NULL) {
        if (current->pid == pid) {
            current->status = status;
            current->return_value = return_value;
            return;
        }
        current = current->next;
    }
}

// Elimina un proceso de la lista
void Remove_backgroundProcess(BackgroundProcess* list, pid_t pid) {
    BackgroundProcess* current = list;

    while (current->next != NULL) {
        if (current->next->pid == pid) {
            BackgroundProcess* to_remove = current->next;
            current->next = to_remove->next;
            free(to_remove->command_line);
            free(to_remove);
            return;
        }
        current = current->next;
    }
}

// Lista los procesos en segundo plano de manera detallada
void Imprimir_backgroundProcesses(BackgroundProcess* list) {
    BackgroundProcess* current = list->next;  // Comenzar desde el primer proceso real
    char time_str[20];
    struct passwd *pw;

    // Imprimimos los encabezados de las columnas
    printf("%-6s %-8s %-17s %-15s %-15s %-9s\n", 
           "PID", "Usuario", "Fecha", "Estado", "Comando", "Prioridad");
    printf("------------------------------------------------------------"
           "--------------------\n");

    // Recorremos la lista de procesos en segundo plano
    while (current != NULL) {
        // Obtener el nombre de usuario
        pw = getpwuid(getuid());
        char *username = pw ? pw->pw_name : "unknown";

        // Convertimos el tiempo de lanzamiento a formato legible
        strftime(time_str, sizeof(time_str), "%Y/%m/%d %H:%M", localtime(&current->launch_time));
        
        // Preparamos el estado del proceso
        char status_str[16];
        switch (current->status) {
            case FINISHED:
                snprintf(status_str, sizeof(status_str), "TERMINADO (%03d)", current->return_value);
                break;
            case STOPPED:
                snprintf(status_str, sizeof(status_str), "DETENIDO");
                break;
            case SIGNALED:
                snprintf(status_str, sizeof(status_str), "SIGNALED (%d)", current->return_value);
                break;
            case ACTIVE:
                snprintf(status_str, sizeof(status_str), "ACTIVO");
                break;
        }
        
        // Imprimimos la información de cada proceso
        printf("%-6d %-8.8s %-17s %-15.15s %-15.15s p=%-9d\n", 
               current->pid, 
               username, 
               time_str, 
               status_str, 
               current->command_line, 
               current->priority);

        // Avanzamos al siguiente proceso
        current = current->next;
    }

    if (list->next == NULL) {
        printf("No hay procesos en segundo plano\n");
    }
}

// Elimina todos los procesos de la lista
void Delete_backgroundProcesses(BackgroundProcess* list) {
    BackgroundProcess* current = list->next;  // Comienza con el segundo nodo (no el primero si es estático)
    BackgroundProcess* temp;

    while (current != NULL) {
        temp = current;
        current = current->next;

        // Libera solo las partes dinámicas de los nodos
        if (temp->command_line != NULL) {
            free(temp->command_line);  // Liberar la memoria de la cadena que fue asignada dinámicamente
        }
        free(temp);  // Liberar el nodo si fue creado dinámicamente
    }
}
