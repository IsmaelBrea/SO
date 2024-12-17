/*
 * TITLE: P3 SISTEMAS OPERATIVOS
 * SUBTITLE: Practica 1
 * AUTHOR 1: ISMAEL BREA ARIAS          LOGIN 1: ismael.brea
 * AUTHOR 2: BORJA CASTELEIRO GOTI      LOGIN 2: borja.casteleiro
 * GROUP: 4.1
 */

#include "lista.h"

////////IMPLEMENTACIÓN DE LAS FUNCIONES DE LA LISTA SIMPLEMENTE ENLAZADA////////

//Función que crea un nuevo nodo para la lista enlazada
bool createNode(Position *tNode) {
    *tNode = malloc(sizeof(struct Node));  //Se asigna memoria suficiente para un nodo y se guarda la dirección en tNode
    return *tNode!= LNULL;  //Devuelve true si la creación fue exitosa; de lo contrario, devuelve false
}

//Función que inicializa una lista vacía
void createEmptyList(List *list) {
    // Inicializa una lista vacía estableciendo el puntero de la lista a NULL.
    *list = LNULL;
}

//Función que comprueba si un historial está vacío
bool isEmptyList(List list) {
    return list == LNULL; 
}

//Función que elimina el historial de comandos introducido
void deleteList(List *list) {
    Position actual = *list; //Posición actual de la lista que se está recorriendo
    Position next; //Puntero para almacenar la siguiente posición a liberar
    
    // Bucle que recorre la lista liberando la memoria de cada nodo.
    while (actual != NULL) {
        next = actual->next; //Guarda la siguiente posición antes de liberar la actual
        free(actual); //Libera la memoria del nodo actual
        actual = next; //Avanza al siguiente nodo
    }
    //Al finalizar, se establece el puntero de la lista a NULL
    *list = NULL;
}

//Función que devuelve la primera posición del primer comando del historial
Position first(List list) {
    return list;  //Devuelve la posición del primer nodo de la lista si no está vacía
}

//Función que devuelve la posición del último comando del historial.
Position last(List list) {
    if (isEmptyList(list)) { 
        return LNULL;  // Devuelve NULL si la lista está vacía
    }

    Position Pos; //Puntero para recorrer la lista
    // Recorremos la lista hasta encontrar el último nodo.
    for (Pos = list; Pos->next != LNULL; Pos = Pos->next);
    return Pos; // Devuelve la posición del último nodo encontrado
}

//Función que inserta un comando al historial en una posición deseada
bool insertToList(Item command, List *list) {
    Position newNode;

    // Creamos un nuevo nodo
    if (!createNode(&newNode)) return false;

    // Inicializamos la memoria del nuevo nodo con ceros
    memset(&(newNode->item), 0, sizeof(Item));

    // Copiar el contenido de command a newNode
    newNode->item.posNum = command.posNum;
    
    // Copiamos los datos de manera segura
    for (int i = 0; i < MAX_LENGTH; i++) {
        if (command.data[i][0] != '\0') {  // Solo si hay datos que copiar
            strncpy(newNode->item.data[i], command.data[i], MAX_LENGTH - 1);
            newNode->item.data[i][MAX_LENGTH - 1] = '\0';  // Aseguramos terminación null
        } else {
            newNode->item.data[i][0] = '\0';  // Inicializamos como cadena vacía
        }
    }
    
    newNode->next = LNULL;

    // Si la lista está vacía, el nuevo nodo se convierte en el primer nodo
    if (*list == LNULL) {
        *list = newNode;
    } else {
        // Buscar el último nodo de la lista
        Position lastNode = *list;
        while (lastNode->next != LNULL) {
            lastNode = lastNode->next;
        }
        lastNode->next = newNode;
    }

    return true;
}

//Función que devuelve la posición de un comando del historial en función de su posición
Position findItem(PositionNumber command, List list) {
    Position Pos; // Puntero para recorrer la lista.
    
    // Recorre la lista hasta encontrar el comando que coincide con el número proporcionado
    for (Pos = list; (Pos != LNULL) && (command != Pos->item.posNum); Pos = Pos->next);
    
    // Si se encuentra, retorna la posición del comando; de lo contrario, retorna NULL
    return (Pos != LNULL && command == Pos->item.posNum) ? Pos : LNULL;
}

// Función que devuelve el Item en la posición dada de la lista
Item getItem(Position pos, List list) {
    if (pos == LNULL) {
        // Manejar el caso de posición no válida
        Item emptyItem;
        memset(&emptyItem, 0, sizeof(Item)); // Llenar con ceros o inicializar de alguna forma
        return emptyItem;
    }

    // Recorremos la lista hasta llegar a la posición deseada
    Position current = list;
    while (current != LNULL && current != pos) {
        current = current->next;
    }

    // Devolvemos el item en la posición deseada
    return current->item;
}


/////FUNCIONES AUXILIARES////
//Función que imprime el historial hasta una posición indicada
void printList(Position Pos, List list, int code) {
    Position current = Pos; // Posición actual de la lista
    int index = 0; // Inicializa un índice para acceder a los elementos

    if (code == 0) {
        printf("--- COMMAND HISTORY: ---\n");
    }

    // Recorremos la lista hasta el final
    while (current != LNULL) {
        printf("%d: %s", index, current->item.data[0]); // Imprime el comando

        // Imprimir argumentos si los hay
        for (int i = 1; i < MAX_LENGTH; i++) {
            if (current->item.data[i][0] != '\0') {
                printf(" %s", current->item.data[i]); // Imprime los argumentos
            }
        }
        
        printf("\n"); // Nueva línea al final de cada comando
        current = current->next; // Avanza al siguiente nodo
        index++; // Incrementa el índice
    }
}
