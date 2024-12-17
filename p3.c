/*
 * TITLE: P3 SISTEMAS OPERATIVOS
 * SUBTITLE: Practica 1
 * AUTHOR 1: ISMAEL BREA ARIAS          LOGIN 1: ismael.brea
 * AUTHOR 2: BORJA CASTELEIRO GOTI      LOGIN 2: borja.casteleiro
 * GROUP: 4.1
 */

#include <stdio.h>  // Librería estándar de C para operaciones de E/S
#include "utils.h"  //Añadimos el archivo con las funciones auxiliares para usar en los comandos
#include <string.h> // Librería para manipulación de cadenas
#include <stdlib.h> // Librería estándar de C para funciones generales como gestión de memoria
#include <unistd.h> // Librería necesaria para funciones de obtención de PID, PPID o el CWD 
#include <time.h>   // Librería con funciones y tipos para manipular tiempos y fechas
#include <sys/utsname.h> // Librería que proporciona la interfaz para acceder a la función uname(), que permite obtener información del SO
#include "lista.h" // Añadimos la lista enlazada para hacer la función del histórico, en la que almacenaremos el historial de comandos
#include <ctype.h>  //Librería con funciones para clasificar y convertir caracteres
#include "lista_ficheros.h" //Añadimos la lista enlazada que contiene funciones para poder trabajar con ficheros para poder hacer las funciones open, close y dup
#include <fcntl.h> // FILE CONTROL: Para usar O_CREAT, O_RDONLY, O_WRONLY etc. Es una cabecera de la biblioteca estándar de C que proporciona funciones relacionadas con la manipulación de archivos y descriptores de archivos
                   /* usamos también esta librería para llamar a funciones como open,close, read*/
#include <sys/stat.h> // Librería que define la estructura de datos para manejar los atributos de archivos, como el tamaño, los permisos y el tipo de archivo. Se usa comúnmente con la función mkdir para crear directorios y definir sus permisos
#include <errno.h> //Librería para manejar errores
#include <limits.h> //Librería para obtener constantes como PATH_MAX
#include <dirent.h> //Librería para manipular directorios
#include <libgen.h> //Librería para manipular rutas de archivos
#include <stdint.h> //Librería para tipos de enteros con tamaño fijo
#include "lista_memoria.h" //Añadimos la lista enlazada que contiene funciones para poder trabajar con la memoria
#include <sys/mman.h>  // Librería para trabajar con memoria mapeada (mmap, munmap, PROT_READ, PROT_WRITE...)
#include "lista_busqueda.h" // Añadimos la lista enlazada que contiene funciones para manejar la búsqueda de directorios donde el shell busca archivos ejecutables
#include "lista_procesos_bg.h"  // Añadimos la lista enlazada que contiene funciones para manejar procesos en segundo plano
#include <pwd.h> // Librería para obtener información sobre usuarios
#include <sys/wait.h> // Librería para funciones relacionadas con la espera de procesos


#define MY_MAX_INPUT 1024 // Valor máximo de caracteres por comando
#define MAX_ARGS 64    // Número máximo de argumentos permitidos
#define TAMANO 2048

List historico; // Declaramos la lista que hemos creado para almacenar el histórico de comandos
ListFicheros ficheros; //Declaramos la lista para manejar ficheros abiertos
MemoryManager memoria;  // Declaramos la lista para manejar la memoria
ListaBusqueda listaBusqueda; // Declaramos la lista para manejar la búsqueda de directorios donde el shell busca archivos ejecutables
BackgroundProcess bgList; // Declaramos la lista para manejar los procesos en segundo plano

extern char **environ;  // Variables externas del sistema

// Estructura para los comandos
struct cmd {
    char *nombre;
    void (*pfuncion_arg)(char *arg);    // Para comandos que requieren un solo argumento
    void (*pfuncion_tr)(char *tr[]);     // Para comandos que requieren múltiples argumentos
    void (*pfuncion_v)(char *tr[], char *envp[]);      // Para comandos que requieren variables de entorno (tercer argumento del main)
};

//// DEFINICIÓN DE LAS FUNCIONES DE LOS COMANDOS ////
//FUNCIONES P0
void cmd_authors(char *arg);
void cmd_pid(char *arg);
void cmd_ppid(char *arg);
void cmd_cd(char *tr[]);
void cmd_date(char *arg);
void cmd_historic(char *arg);
void cmd_open(char *tr[]);
void cmd_close(char *tr[]);
void cmd_dup(char *tr[]);
void cmd_infosys(char *arg);
void cmd_help(char *arg);
void cmd_salir(char *arg);


//FUNCIONES P1 sobre ficheros
void cmd_makefile(char *arg);
void cmd_makedir(char *arg);
void cmd_listfile(char *tr[]);
void cmd_cwd(char *arg);
void cmd_listdir(char *tr[]);
void cmd_reclist(char *tr[]);
void cmd_revlist(char *tr[]);
void cmd_erase(char *arg);
void cmd_delrec(char *arg);

//Funciones auxiliares P1 para la recursividad
void reclist_recursive(const char *path, int long_format, int show_hidden, int show_access_time, int show_link);
void revlist_recursive(const char *path, int long_format, int show_hidden, int show_access_time, int show_link);
void delete_recursive(const char *path);


//FUNCIONE P2 sobre memoria
void cmd_allocate(char *tr[]);
void cmd_deallocate(char *tr[]);
void cmd_memfill(char *tr[]);
void cmd_memdump(char *tr[]);
void cmd_memory(char *tr[]);
void cmd_readfile(char *tr[]);
void cmd_writefile (char *tr[]);
void cmd_read(char *tr[]);
void cmd_write(char *tr[]);
void cmd_recurse(char *tr[]);

//FUNCIONES auxiliares P2 para la recursividad
void Recursiva(int n);


//FUNCIONES P3 sobre procesos
void cmd_getuid(char *arg);
void cmd_setuid(char *tr[]);
void cmd_showvar(char *tr[], char *envp[]);
void cmd_changevar(char *tr[], char *envp[]);
void cmd_subsvar(char *tr[], char *envp[]);
void cmd_environ(char *tr[], char *envp[]);
void cmd_fork(char *tr[]);
void cmd_search(char *tr[]);
void cmd_exec(char *tr[]);
void cmd_execpri(char *tr[]);
void cmd_fg(char *tr[]);
void cmd_fgpri(char *tr[]);
void cmd_back(char *tr[]);
void cmd_backpri(char *tr[]);
void cmd_listjobs(char *tr[]);
void cmd_deljobs(char *tr[]);
void cmd_execute_external_program(char *tr[], char *envp[]);

//FUNCIONES auxiliares P3
void update_background_processes();

struct cmd cmds[] = {
    {"authors", cmd_authors, NULL, NULL},
    {"pid", cmd_pid, NULL,NULL},
    {"ppid", cmd_ppid, NULL,NULL},
    {"cd", NULL, cmd_cd, NULL},
    {"date", cmd_date, NULL},
    {"historic", cmd_historic, NULL, NULL},
    {"open", NULL, cmd_open, NULL},
    {"close", NULL, cmd_close, NULL},
    {"dup", NULL, cmd_dup, NULL},
    {"infosys", cmd_infosys, NULL, NULL},
    {"help", cmd_help, NULL, NULL},
    {"quit", cmd_salir, NULL,NULL},
    {"exit", cmd_salir, NULL,NULL},
    {"bye", cmd_salir, NULL,NULL},
    {"makefile",cmd_makefile,NULL,NULL},
    {"makedir",cmd_makedir,NULL,NULL},
    {"listfile",NULL,cmd_listfile,NULL},
    {"cwd",cmd_cwd,NULL,NULL},
    {"listdir",NULL,cmd_listdir,NULL},
    {"reclist",NULL,cmd_reclist,NULL},
    {"revlist",NULL,cmd_revlist,NULL},
    {"erase",cmd_erase,NULL,NULL},
    {"delrec",cmd_delrec,NULL,NULL},
    {"allocate",NULL,cmd_allocate,NULL},
    {"deallocate",NULL,cmd_deallocate,NULL},
    {"memfill",NULL, cmd_memfill,NULL},
    {"memdump",NULL,cmd_memdump,NULL},
    {"memory",NULL,cmd_memory,NULL},
    {"readfile",NULL,cmd_readfile,NULL},
    {"writefile",NULL,cmd_writefile,NULL},
    {"read",NULL,cmd_read,NULL},
    {"write",NULL,cmd_write,NULL},
    {"recurse",NULL,cmd_recurse,NULL},
    {"getuid",cmd_getuid,NULL,NULL},
    {"setuid",NULL,cmd_setuid,NULL},
    {"showvar",NULL,NULL,cmd_showvar},
    {"changevar",NULL,NULL,cmd_changevar},
    {"subsvar",NULL,NULL,cmd_subsvar},
    {"environ",NULL,NULL,cmd_environ},
    {"fork",NULL,cmd_fork,NULL},
    {"search",NULL,cmd_search,NULL},
    {"exec",NULL,cmd_exec,NULL},
    {"execpri",NULL,cmd_execpri,NULL},
    {"fg",NULL,cmd_fg,NULL},
    {"fgpri",NULL,cmd_fgpri,NULL},
    {"back",NULL,cmd_back,NULL},
    {"backpri",NULL,cmd_backpri,NULL},
    {"listjobs",NULL,cmd_listjobs,NULL},
    {"deljobs",NULL,cmd_deljobs,NULL},
    {"execute",NULL,NULL,cmd_execute_external_program},
    {NULL, NULL, NULL}  // Fin del arreglo
};


// Funciones del bucle del shell
int TrocearCadena(char *cadena, char *trozos[]);

// Otras funciones
void imprimirPrompt();
void leerEntrada(char *buffer);
void procesarEntrada(char *input);



/**************************************************
 *               FUNCIÓN PRINCIPAL                *
 **************************************************/
int main(int argc, char *argv[], char *envp[]) {
    char input[MY_MAX_INPUT];            // Buffer para almacenar la entrada del usuario
    createEmptyList(&historico);         // Inicializamos la lista del historial de comandos
    createEmptyListFicheros(&ficheros);  //Inicializamos la lista del historial de comandos
    InitMemoryManager(&memoria);         // Inicializamos la lista de memoria   
    inicializarListaBusqueda(&listaBusqueda); // Inicializamos la lista de búsqueda de directorios
    Init_backgroundProcesses(&bgList);   // Inicializamos la lista de procesos en segundo plano
    printf("Bienvenido al Shell de Sistemas Operativos\n");

    // BUCLE del shell
    while (1) {
        imprimirPrompt();
        leerEntrada(input);
        procesarEntrada(input);
    }
    return 0;
}


// ====== IMPLEMENTACIÓN DE LAS FUNCIONES ======

// Función para imprimir un mensaje de entrada cada vez que se quiera ejecutar un comando
void imprimirPrompt() {
    char hostname[256];
    char cwd[PATH_MAX];
    struct passwd *pw;
    uid_t uid;

    // Obtener el nombre de host
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        strcpy(hostname, "unknown");
    }

    // Obtener el directorio actual
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        strcpy(cwd, "unknown");
    }

    // Obtener el nombre de usuario actual
    uid = geteuid();
    pw = getpwuid(uid);
    if (pw == NULL) {
        printf("%s@%s(%s) -> ", "unknown", hostname, cwd);
    } else {
        printf("%s@%s(%s) -> ", pw->pw_name, hostname, cwd);
    }

    fflush(stdout); // Asegura que el prompt se imprima inmediatamente
}

// Función que lee la entrada (comando introducida por el usuario)
void leerEntrada(char *buffer) {
    fgets(buffer, MY_MAX_INPUT, stdin);    // Lee la entrada del usuario hasta un máximo de 1024 caracteres
    int len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';  // Elimina el salto de línea
    }
}

// Función para procesar la entrada del usuario. Interacción entre el usuario y el sistema
void procesarEntrada(char *input) {
    char *args[MAX_ARGS];
    int num_args = TrocearCadena(input, args);

    if (num_args == 0) {
        return;
    }

    // Crear e inicializar el Item para el historial
    Item item;
    memset(&item, 0, sizeof(Item));
    item.posNum = isEmptyList(historico) ? 0 : last(historico)->item.posNum + 1;
    for (int i = 0; i < num_args && i < MAX_LENGTH; i++) {
        if (args[i] != NULL) {
            strncpy(item.data[i], args[i], MAX_LENGTH - 1);
            item.data[i][MAX_LENGTH - 1] = '\0';
        }
    }

    // Insertar el comando en el historial
    if (!insertToList(item, &historico)) {
        printf("Error al insertar el comando en el historial.\n");
    }

    // Buscar el comando en la lista de comandos internos
    for (int i = 0; cmds[i].nombre != NULL; i++) {
        if (strcmp(args[0], cmds[i].nombre) == 0) {
            if (cmds[i].pfuncion_arg) {
                cmds[i].pfuncion_arg(num_args > 1 ? args[1] : NULL);
            } else if (cmds[i].pfuncion_tr) {
                cmds[i].pfuncion_tr(args);
            } else if (cmds[i].pfuncion_v) {
                extern char **environ;
                cmds[i].pfuncion_v(args, environ);
            }
            return;
        }
    }

    // Si no es un comando interno, intentar ejecutar como comando externo
    cmd_execute_external_program(args, environ);
}


int TrocearCadena(char *cadena, char *trozos[]) {
    int i = 1;  // Inicializa un índice para acceder a los elementos del arreglo trozos

    // Divide la cadena en trozos
    trozos[0] = strtok(cadena, " \n\t");
    if (trozos[0] == NULL) {
        return 0;
    }
    // Bucle que se repite hasta que se hayan procesado todos los trozos de la cadena
    while ((trozos[i] = strtok(NULL, " \n\t")) != NULL) {
        i++; // Incrementa el índice para acceder al siguiente elemento del arreglo trozos
    }
    return i; // Devuelve el valor de i, que indica el número de trozos encontrados en la cadena
}



//// IMPLEMENTACIÓN DE LAS FUNCIONES DE LOS COMANDOS ////

/// ---- FUNCIONES P0 ----  ////
// Función que devuelve el nombre y los logins de los autores del shell
void cmd_authors(char *arg) {
    if (arg == NULL || strcmp(arg, "") == 0) { // Si no se le pasa ningún argumento
        printf("Ismael Brea Arias: ismael.brea\n");
        printf("Borja Casteleiro Goti: borja.casteleiro\n");
    } else if (strcmp(arg, "-l") == 0) { // Si se pasa "-l", imprimir solo logins
        printf("ismael.brea\n");
        printf("borja.casteleiro\n");
    } else if (strcmp(arg, "-n") == 0) {  // Si se pasa "-n", imprimir solo nombres
        printf("Ismael Brea Arias\n");
        printf("Borja Casteleiro Goti\n");
    } else {  // Si se le pasa un argumento no reconocido, salimos
     perror("Argumento inválido, usa authors, authors -l o authors -n\n");
        return;
    }
}


// Función que devuelve el identificador del Proceso que ejecuta el shell (Process ID del shell) 
void cmd_pid(char *arg) {
    if (arg == NULL || strcmp(arg, "\0") == 0) {
        // Si no se pasa ningún argumento, obtenemos el PID del shell
        int obtener_pid = getpid();
        printf("Pid del shell: %d\n", obtener_pid);
    } else if (strcmp(arg, "-p") == 0) {
        // Si el argumento es '-p', obtenemos el PPID (Parent PID)
        int obtener_ppid = getppid();
        printf("Pid del padre del shell: %d\n", obtener_ppid);
    } else {
        // Para cualquier otro argumento, mostramos un mensaje de error
        perror("Argumento inválido, usa ppid or pid -p\n");
    }
}


// Función que devuelve el Identificador del Proceso Padre (Parent Process ID)
void cmd_ppid(char *arg) {
    if (arg == NULL || strcmp((const char *) arg, "\0") == 0) {
        int obtener_ppid = getppid();
        printf("Pid del padre del shell: %d\n", obtener_ppid);
    } else {
        perror("Argumento inválido, usa ppid or pid -p\n");
    }
}


// Función que cambia el directorio de trabajo actual
void cmd_cd(char *tr[]) {
    char cwd[PATH_MAX];
    char *path; // Declara la variable path

    // Imprimir el directorio de trabajo actual antes de cambiar
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Directorio actual: %s\n", cwd);
    }

    // Si no se pasa argumento o se pasa solo "cd", vamos al directorio home
    if (tr[1] == NULL || strcmp(tr[1], "") == 0 || strcmp(tr[1], " ") == 0) {
        path = getenv("HOME"); // Obtiene el directorio home del usuario
    } else {
        path = tr[1]; // Usa el directorio proporcionado por el usuario
    }

    // Intentamos cambiar al directorio especificado
    if (chdir(path) != 0) {
        perror("Error al cambiar el directorio");
    } else {
        // Imprimir el nuevo directorio de trabajo después de cambiar
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Nuevo directorio actual: %s\n", cwd);
        }
    }
}


// Función que devuelve la fecha y hora actual
void cmd_date(char *arg) {
    // Obtener el tiempo actual
    time_t t = time(NULL);                // Obtener la hora actual en segundos desde el Epoch ( punto de referencia en el tiempo que se utiliza para calcular el tiempo en segundos)
    struct tm *tm_info = localtime(&t);   // Convertir el tiempo en una estructura de tiempo local ajustada a la zona horaria local

    char buffer[100];

    // Si no se pasa ningún argumento, imprimimos la hora y luego la fecha
    if (arg == NULL || strcmp(arg, "") == 0) {
        // Imprimir la hora
        strftime(buffer, sizeof(buffer), "%H:%M:%S", tm_info);    // buffer, cadena formateada, tamaño del buffer, estructura de tiempo
        printf("%s\n", buffer);
        // Imprimir la fecha
        strftime(buffer, sizeof(buffer), "%d/%m/%Y", tm_info);
        printf("%s\n", buffer);
    } 
    // Si se pasa la opción -d, imprimimos solo la fecha
    else if (strcmp(arg, "-d") == 0) {
        strftime(buffer, sizeof(buffer), "%d/%m/%Y", tm_info);
        printf("%s\n", buffer);
    } 
    // Si se pasa la opción -h, imprimimos solo la hora
    else if (strcmp(arg, "-t") == 0) {
        strftime(buffer, sizeof(buffer), "%H:%M:%S", tm_info);
        printf("%s\n", buffer);
    } else { // Si se pasa un argumento no reconocido
        printf("Argumento invalido, usa date [-t|-d]\n");
    }
}


// Función que muestra y maneja el histórico de comandos ejecutados (empieza desde 0)
void cmd_historic(char *arg) {
    if (isEmptyList(historico)) {
        printf("El historial está vacío\n");
        return;
    }

    // Si no se pasa argumento, imprimir todo el historial
    if (arg == NULL || *arg == '\0') {
        printList(first(historico), historico, 0);
        return;
    }

    // si el argumento es -c, borra el historial
    if (strcmp(arg, "-c") == 0) {
        deleteList(&historico);
        printf("Historial eliminado\n");
        return;
    }

    // Si el argumento es un número, ejecutar el comando correspondiente
    if (isNumber(arg)) {
        int num = atoi(arg);
        Position pos = findItem(num, historico);
        if (pos != LNULL) {
            Item commandItem = getItem(pos, historico);
            printf("Ejecutando hist (%d): %s", num, commandItem.data[0]);
            // Imprimir los argumentos del comando
            for (int i = 1; commandItem.data[i][0] != '\0'; i++) {
                printf(" %s", commandItem.data[i]);
            }
            printf("\n");
            // Reconstruir el comando completo con sus argumentos
            char fullCommand[MAX_LENGTH * MAX_LENGTH] = {0};
            strcat(fullCommand, commandItem.data[0]);
            for (int i = 1; commandItem.data[i][0] != '\0'; i++) {
                strcat(fullCommand, " ");
                strcat(fullCommand, commandItem.data[i]);
            }
            procesarEntrada(fullCommand);     //llama a esta función para ejecutar el comando como si hubiera sido ingresado directamente por el usuario
        } else {
            printf("No hay elemento %d en el histórico\n", num);
        }
        return;
    }

    // Si el argumento es -N, imprimir los últimos N comandos
    if (arg[0] == '-' && isNumber(arg + 1)) {
        int num = atoi(arg + 1);
        if (num > 0) {
            Position pos = last(historico);
            int totalCommands = 0;
            while (pos != LNULL) {
                totalCommands++;
                pos = pos->next;
            }

            int startIndex = (totalCommands > num) ? (totalCommands - num) : 0;
            pos = first(historico);
            for (int i = 0; i < startIndex; i++) {
                pos = pos->next;
            }

            printf("--- ULTIMOS %d COMANDOS: ---\n", num);
            int count = 0;
            while (pos != LNULL && count < num) {
                Item item = getItem(pos, historico);
                printf("%d: %s", item.posNum, item.data[0]);
                for (int i = 1; item.data[i][0] != '\0'; i++) {
                    printf(" %s", item.data[i]);
                }
                printf("\n");
                pos = pos->next;
                count++;
            }
        } else {
            printf("Numero invalido. Debe ser mayor que cero\n");
        }
        return;
    }

    printf("Usa: historic [-c|-N|N] (N = número positivo)\n");
}


//Función que abre un archivo y se encarga de añadirlo a una lista de archivos abiertos en el shell
void cmd_open(char *tr[]) {
    int i, df, mode = 0;
    
    //si el primer argumento que debe ser el nombre del archivo es nulo, mostramos la lista con los descripotres abiertos y salimos de la función
    if (tr[1] == NULL) {
        listarDescriptores();
        return;
    }

    // Bucle para recorrer los argumentos y establecer el modo de apertura
    for (i = 2; tr[i] != NULL; i++) {
        if (!strcmp(tr[i], "cr")) mode |= O_CREAT;
        else if (!strcmp(tr[i], "ex")) mode |= O_EXCL;
        else if (!strcmp(tr[i], "ro")) mode |= O_RDONLY;
        else if (!strcmp(tr[i], "wo")) mode |= O_WRONLY;
        else if (!strcmp(tr[i], "rw")) mode |= O_RDWR;
        else if (!strcmp(tr[i], "ap")) mode |= O_APPEND;
        else if (!strcmp(tr[i], "tr")) mode |= O_TRUNC;
        else break;
    }

    if (mode == 0) {
        mode = O_RDONLY;
    }

    if ((df = open(tr[1], mode, 0666)) == -1) {
        perror("Imposible abrir fichero");
    } else {
        if (anadirAFicherosAbiertos(df, mode, tr[1], &ficheros)) {
            printf("Anadida entrada %d a la tabla de ficheros abiertos: %s\n", df, tr[1]);
        } else {
            printf("Error al anadir el archivo a la lista de ficheros abiertos\n");
            close(df);
        }
    }
}


//Función que cierra un descriptor de archivo abierto y elimina el archivo correspondiente de la lista de archivos abiertos
void cmd_close(char *tr[]) { 
    int df;
    
    if (tr[1] == NULL) {
        listarDescriptores();
        return;
    }

    df = atoi(tr[1]);
    if (df < 0) {
        printf("Descriptor de archivo inválido\n");
        return;
    }

    if (close(df) == -1) {
        perror("Imposible cerrar descriptor");
    } else {
        if (eliminarDeFicherosAbiertos(df, &ficheros)) {
            printf("Descriptor %d cerrado y eliminado de la lista\n", df);
        } else {
            printf("El descriptor %d no estaba en la lista de ficheros abiertos\n", df);
        }
    }
}


//Función que duplica un descriptor de archivo y agrega la nueva entrada a la lista de archivos abiertos
void cmd_dup(char *tr[]) { 
    int df, duplicado;
    char aux[MAXNAME];
    const char* p;

    // Si no se proporciona argumento, listar descriptores
    if (tr[1] == NULL) {
        listarDescriptores();
        return;
    }

    // Validar que se haya pasado un descriptor válido
    if ((df = atoi(tr[1])) < 0) {
        listarFicherosAbiertos(ficheros); // Lista archivos abiertos si el descriptor no es válido
        return;
    }

    // Duplicar el descriptor
    duplicado = dup(df);
    if (duplicado == -1) {
        perror("Error al duplicar descriptor");
        return;
    }

    // Obtener el nombre del archivo asociado al descriptor original
    p = nombreFicheroDescriptor(df, ficheros);
    if (p == NULL) {
        // Si no se encuentra en la lista, usar un nombre genérico
        p = (df <= 2) ? 
            (df == 0 ? "entrada estandar" : (df == 1 ? "salida estandar" : "error estandar")) : 
            "desconocido";
    }

    // Crear la cadena para el nuevo nombre
    snprintf(aux, MAXNAME, "dup %d (%s)", df, p);

    // Obtener el modo del descriptor original
    int modo = fcntl(df, F_GETFL);
    if (modo == -1) {
        perror("Error al obtener el modo del descriptor");
        close(duplicado);
        return;
    }

    // Añadir el nuevo descriptor a la lista de archivos abiertos
    if (anadirAFicherosAbiertos(duplicado, modo, aux, &ficheros)) {
        printf("Descriptor %d duplicado como %d (%s)\n", df, duplicado, aux);
    } else {
        printf("Error al anadir el descriptor duplicado a la lista\n");
        close(duplicado);
    }
}


//Función que devuelve información sobre la máquina que ejecuta el shell
void cmd_infosys(char *arg){
    struct utsname sys_info;
   
    if(arg == NULL || strcmp((const char *) arg, "\0")==0){
        // Llamada a uname (librería  <sys/utsname.h> ) para obtener información del sistema
    if (uname(&sys_info) == -1) {
        perror("Error al obtener la información del sistema");
        return;
    }

    // Formatear la salida como se espera
    printf("%s (%s), OS: %s-%s-%s\n", 
           sys_info.nodename,    // Nombre del host 
           sys_info.machine,     // Arquitectura 
           sys_info.sysname,     // Sistema operativo
           sys_info.release,     // Versión del kernel 
           sys_info.version      // Detalles adicionales de la versión 
    );
    }else{
        printf("Argumento invalido, usa infosys\n");
    }
}


//Función que muestra una ayuda con los comandos disponibles
void cmd_help(char *arg) {
    if (arg == NULL || strcmp(arg, "") == 0) { // Si no se pasa ningún argumento, mostrar lista general de comandos
        printf("'help [cmd|-lt|-T topic]' ayuda sobre comandos\n"
       "\t\tComandos disponibles:\n"
       "authors pid ppid cd date historic open close dup infosys help quit exit bye\n "
       "makefile makedir listfile cwd listdir reclist revlist erase delrec\n"
       "allocate deallocate memfill memdump memory readfile writefile read write recurse\n"
       "getuid setuid showvar changevar subsvar environ fork search exec execpri fg fgpri back backpri listjobs deljobs\n");
    }
    else
    {
        if (strcmp(arg, "authors") == 0 || strcmp(arg, "autores") == 0) {
            printf("authors [-n][-l] Muestra los nombres y logins de los autores\n");
        } else if (strcmp(arg, "pid") == 0) {
            printf("pid [-p] Muestra el pid del shell o de su proceso padre\n");
        } else if(strcmp(arg, "ppid") == 0) {
            printf("ppid Muestra el pid del proceso padre del shell\n");
        } else if (strcmp(arg, "cd") == 0 || strcmp(arg, "carpeta") == 0) {
            printf("cd[dir] Cambia (o muestra) el directorio actual del shell\n");
        } else if (strcmp(arg, "date") == 0 || strcmp(arg, "time") == 0) {
            printf("date [-d][-t] Muestra la fecha y/o la hora actual\n");
        } else if (strcmp(arg, "historic") == 0) {
            printf("historic [-c|-N|N]\tMuestra (o borra) el historico de comandos\n");
            printf("\t-N: muestra los N primeros\n");
            printf("\t-c: borra el historico\n");
            printf("\tN: repite el comando N\n");
        }else if(strcmp(arg, "open") == 0 ){
            printf("open fich m1 m2...\tAbre el fichero fich\n");
            printf("lo anade a la lista de ficheros abiertos en el shell\n");
            printf("m1,m2..es el modo de apertura (or bit a bit de los siguientes)\n");
            printf("cr: O_CREAT\tap: O_APPEND\n");
            printf("ex: O_EXCL\tro: O_RDONLY\n");
            printf("rw: O_RWDR\two: O_WRONLY\n");
            printf("tr: O_TRUNC\n");
        } else if(strcmp(arg,"close") == 0){
            printf("close df\tCierra el descriptor df  y elimina el correspondiente fichero de la lista de ficheros abiertos\n");
        }else if(strcmp(arg,"dup") == 0){
            printf("dup df\tDuplica el descriptor de fichero df y anade una nueva entrada a la lista ficheros abiertos\n");
        }else if(strcmp(arg,"infosys") == 0){
            printf("infosys\tMuestra información de la maquina donde se corre el shell\n");
        }else if(strcmp(arg, "quit") == 0) {
            printf("quit\tTermina la ejecución del shell\n");
        }else if(strcmp(arg, "exit") == 0) {
            printf("exit\tTermina la ejecución del shell\n");
        }else if(strcmp(arg, "bye") == 0) {
            printf("bye\tTermina la ejecución del shell\n");
        } else if (strcmp(arg, "help") == 0 || strcmp(arg, "ayuda") == 0) {
            printf("help/ayuda [cmd|-lt|-T topic] Muestra ayuda sobre los comandos\n"
                   "\t-lt: lista topics de ayuda\n"
                   "\t-T topic: lista comandos sobre ese topic\n"
                   "\tcmd: info sobre el comando cmd\n");
        } else if(strcmp(arg, "makefile") == 0){
            printf("makefile [name] Crea un fichero de nombre name\n");
        } else if(strcmp(arg, "makedir") == 0){
            printf("makedir [name] Crea un directorio de nombre name\n");
        } else if(strcmp(arg, "listfile") == 0) {
            printf("listfile [-long][-link][-acc] name1 name2 ..   lista ficheros;\n");
            printf("          -long: listado largo\n");
            printf("          -acc: accestime\n");
            printf("          -link: si es enlace simbolico, el path contenido\n");
        } else if (strcmp(arg,"cwd") == 0){
            printf("cwd\tMuestra el directorio actual del shell\n");
        } else if (strcmp(arg,"listdir") == 0){
            printf("listdir [-hid] [-long] [-link] [-acc]  n1 n2 ..\tlista contenido de directorios\n");
            printf("         -long: listado largo\n");
            printf("         -hid: incluye los ficheros ocultos\n");
            printf("         -acc: accestime\n");
            printf("         -link: si es enlace simbolico, el path contenido\n");
        } else if(strcmp(arg,"reclist") == 0){
             printf("reclist [-hid] [-long] [-link] [-acc]  n1 n2 ..\tlista recursivamente contenidos de directorios (subdirs despues)\n");
            printf("         -hid: incluye los ficheros ocultos\n");
            printf("         -long: listado largo\n");
            printf("         -acc: accestime\n");
            printf("         -link: si es enlace simbolico, el path contenido\n");
        } else if(strcmp(arg,"revlist") == 0){
             printf("revlist [-hid] [-long] [-link] [-acc]  n1 n2 ..\tlista recursivamente contenidos de directorios (subdirs antes)\n");
            printf("         -hid: incluye los ficheros ocultos\n");
            printf("         -long: listado largo\n");
            printf("         -acc: accestime\n");
            printf("         -link: si es enlace simbolico, el path contenido\n");
        } else if(strcmp(arg, "erase") == 0){
            printf("erase [name1 name2..]  Borra ficheros o directorios vacios\n");
        } else if(strcmp(arg, "delrec") == 0){
            printf("delrec [name1 name2..]  Borra ficheros o directorios recursivamente\n");
        } else if(strcmp(arg, "allocate") == 0){
            printf("allocate [-malloc|-shared|-createshared|-mmap]... Asigna un bloque de memoria\n");
            printf("          -malloc tam: asigna un bloque malloc de tamano tam\n");
            printf("          -createshared cl tam: asigna (creando) el bloque de memoria compartida de clave cl y tamano tam\n");
            printf("          -shared cl: asigna el bloque de memoria compartida (ya existente) de clave cl\n");
            printf("          -mmap fich perm: mapea el fichero fich, perm son los permisos\n");
        } else if(strcmp(arg, "deallocate") == 0){
            printf("deallocate [-malloc|-shared|-delkey|-mmap|addr].. Deasigna un bloque de memoria\n");
            printf("          -malloc addr: desasigna el bloque malloc de tamano tam\n");
            printf("          -shared cl: deasigna (desmapea) el bloque de memoria compartida de clave cl\n");
            printf("          -delkey cl: elimina del sistema (sin desmapear) la clave de memoria cl\n");
            printf("          -mmap fich: desmapea el fichero mapeado fich\n");
            printf("          addr: desasigna el bloque de memoria en la direccion addr\n");
        } else if (strcmp(arg, "memdump") == 0){
            printf("memdump addr cont\tVuelca en pantallas los contenidos (cont bytes) de la posicion de memoria addr\n");
        } else if(strcmp(arg,"memfill") == 0){
            printf("memfill addr cont byte\tLlena la memoria a partir de addr con byte\n");
        } else if(strcmp(arg, "memory") == 0){
            printf("memory [-blocks|-funcs|-vars|-all|-pmap] ..\tMuestra muestra detalles de la memoria del proceso\n");
            printf("        -blocks: los bloques de memoria asignados\n");
            printf("        -funcs: las direcciones de las funciones\n");
            printf("        -vars: las direcciones de las variables\n");
            printf("        -all: todo\n");
            printf("        -pmap: muestra la salida del comando pmap(o similar)\n");
        } else if(strcmp(arg, "readfile") == 0){
            printf("readfile fiche addr cont\nLee cont bytes desde fich a la direccion addr\n");
        } else if(strcmp(arg, "writefile") == 0){
            printf("writefile [-o] fiche addr cont\tEscribe cont bytes desde la direccion addr a fich (-o sobreescribe\n)");
        } else if (strcmp(arg, "read") == 0){
            printf("read df addr cont\tTransfiere cont bytes del fichero descrito por df a la dirección addr\n");
        } else if(strcmp(arg, "write") == 0){
            printf("write df addr cont\tTransfiere cont bytes desde la dirección addr al fichero descrito por df\n");
        } else if (strcmp(arg, "getuid") == 0){
            printf("getuid\tMuestra las credenciales del proceso que ejecuta el shell\n");
        }
        else if (strcmp(arg, "setuid") == 0){
            printf("setuid [-l] id\tCambia las credenciales del proceso que ejecuta el shell\n");
            printf("        id: establece la credencial al valor numerico id\n");
            printf("        -l: establece la credencial al login id\n");
        }else if (strcmp(arg, "showvar") == 0){
            printf("showvar var\tMuestra el valor y las direcciones de la variable de entorno var\n");
        }
        else if (strcmp(arg, "changevar") == 0){
            printf("changevar [-a|-e|-p] var valor\tCambia el valor de una variable de entorno\n");
            printf("     -a: accede por el tercer arg del main\n");
            printf("     -e: accede mediante environ\n");
            printf("     -p: accede mediante putenv\n");
        }else if (strcmp(arg, "subsvar") == 0){
            printf("subsvar [-a|-e] var1 var2 valor	Sustituye la variable de entorno var1 con var2=valor\n");
            printf("     -a: accede por el tercer arg del main\n");
            printf("     -e: accede mediante environ\n");
        } else if(strcmp(arg,"environ") == 0){
            printf("environ [-environ|-addr]\t\tMuestra el entorno del proceso\n");
            printf("        -environ: accede usando environ (en lugar del tercer arg del main)\n");
            printf("        -addr: muestra el valor y donde se almacenan environ y el 3er arg main\n");
        } else if (strcmp(arg,"fork") == 0){
            printf("fork\tEl shell hace fork y queda en espera a que su hijo termine\n");
        } else if(strcmp(arg,"search") == 0){
            printf("search [-add|-del|-clear|-path]..\t\tManipula o muestra la ruta de busqueda del shell(path)\n");
            printf("        -add dir: aniade dir a la ruta de busqueda(equiv +dir)\n");
            printf("        -del dir: elimina dir de la ruta de busqueda(equiv -dir)\n");
            printf("        -clear: vacia la ruta de busqueda\n");
            printf("        -path: importa el PATH en la ruta de busqueda\n");
        } else if(strcmp(arg,"exec") == 0){
            printf("exec VAR1 VAR2 ..prog args....[@pri]\tEjecuta, sin crear proceso, prog con argumentos en un entorno que contiene solo las variables VAR1, VAR2...\n");
        } else if(strcmp(arg,"execpri") == 0){
            printf("execpri prio prog args....\tEjecuta, sin crear proceso, prog con argumentos con la prioridad cambiada a prio\n");
        } else if(strcmp(arg,"fg") == 0){
            printf("fg prog args...\tCrea un proceso que ejecuta en en primer plano prog con argumentos\n");
        } else if(strcmp(arg,"fgpri") == 0){
            printf("fgpri prio prog args...\tCrea un proceso que ejecuta en en primer plano prog con argumentos con la prioridad cambiada a prio\n");
        } else if(strcmp(arg,"back") == 0){
            printf("back prog args...\tCrea un proceso que ejecuta en en segundo plano prog con argumentos\n");
        } else if(strcmp(arg,"backpri") == 0){
            printf("backpri prio prog args...\tCrea un proceso que ejecuta en en segundo plano prog con argumentos con la prioridad cambiada a prio\n");
        } else if(strcmp(arg,"listjobs") == 0){
            printf("listjobs\tLista los procesos en segundo plano\n");
        } else if(strcmp(arg,"deljobs") == 0){
            printf("deljobs [-term][-sig]\tElimina los procesos de la lista procesos en segundo plano\n");
            printf("        -term: los terminados\n");
            printf("        -sig: los terminados por senal\n");
       } else{
            printf("%s no encontrado\n", arg);
        }
    }
}


//Función que finaliza el shell
void cmd_salir(char *arg) {
    // Comprobamos si hay un argumento adicional
    if (arg != NULL) {
        return; // No se hace nada si hay un argumento adicional
    }
    //Limpiar todos los recursos de memoria compartida
    //LimpiarRecursosMemoriaCompartida(&memoria);

    deleteList(&historico);         // Eliminamos el historial de comandos  
    deleteListFicheros(&ficheros);  // Eliminamos la lista de ficheros abiertos  
    DeleteMemoryList(&memoria);   // Eliminamos la lista de memoria
    limpiarListaBusqueda(&listaBusqueda); // Eliminamos la lista de búsqueda
    Delete_backgroundProcesses(&bgList); // Eliminamos la lista de procesos en segundo plano

    // Finalizamos el shell sin imprimir ningún mensaje
    exit(0);
}



/// ---- FUNCIONES P1 (ficheros) ----  ////
// Función que imprime el directorio de trabajo actual
void cmd_cwd(char *arg){
    // Creamos un buffer para almacenar el directorio actual
    char buffer[1024];
    size_t size = sizeof(buffer);

    // Comprobamos si el argumento es nulo o está vacío
    if (arg == NULL || *arg == '\0')
{
        // Si es así, imprimimos el directorio actual
        if (getcwd(buffer, size) != NULL)
        {
            printf("%s\n", buffer); // Utilizamos getcwd para obtener el directorio actual
        }
        else
        {
            perror("Error obteniendo el directorio actual\n"); // Manejo de errores si getcwd falla
        }
        return;
    }
    else{
        printf("Argumento invalido, usa cwd\n");
    }
}


//Función que crea un fichero con el nombre que le pasemos como argumento
void cmd_makefile(char *arg) {
    // Verificar si se pasó un argumento (nombre del archivo)
    if (arg == NULL || *arg == '\0') {
        cmd_cwd(NULL);    // Si no se pasa un argumento, mostrar el directorio actual   
        return;
    }

    // Intentar crear el archivo con O_CREAT y O_EXCL para evitar sobreescribir si ya existe
    int fd = open(arg, O_CREAT | O_WRONLY | O_EXCL, 0644);  // Crear con permisos de escritura, solo si no existe
    if (fd == -1) {
        if (errno == EEXIST) {
            // Si el archivo ya existe, mostrar el mensaje de error
            printf("Imposible crear %s: File exists\n", arg);
        } else {
            // Mostrar cualquier otro error de apertura de archivo
            perror("Error al crear el archivo");
        }
        return;
    }

    printf("Archivo %s creado correctamente\n", arg);
    
    // Cerrar el archivo
    close(fd);
}


//Función que crea un fichero con el nombre que le pasemos como argumento
void cmd_makedir(char *arg) {
     // Verificar si se pasó un argumento (nombre del archivo)
    if (arg == NULL || *arg == '\0') {
        cmd_cwd(NULL);    // Si no se pasa un argumento, mostrar el directorio actual
        return;
    }

    // Verificar si el directorio ya existe
    struct stat st;   //stat se utiliza para obtener información sobre un archivo o directorio en el sistema de archivos
    if (stat(arg, &st) == 0 && S_ISDIR(st.st_mode)) {  //verifica si el archivo o directorio indicado por arg existe (usando stat)y luego comprueba si lo que encontró es un directorio
        printf("Imposible crear %s: File exists\n", arg);
        return;  //si ya existe el directorio, avisa que el directorio ya está creado y no hace nada más
    }

    // Intentar crear el directorio con permisos 0777 (permite lectura, escritura y ejecución para todos)
    int result = mkdir(arg, 0777);
    
    // Comprobar si hubo un error al crear el directorio
    if (result == -1) {
        perror("Error al crear el directorio");
    } else {
        printf("Directorio %s creado correctamente\n", arg);
    }
}


// Función que proporciona información sobre un archivo o un directorio 
void cmd_listfile(char *tr[]) {
    int longFormat = 0, accessTime = 0, linkInfo = 0;
    int i = 1;

    // Procesar opciones
    while (tr[i] != NULL && tr[i][0] == '-') {
        if (strcmp(tr[i], "-long") == 0) longFormat = 1;     // -long muestra un listado con el inodo del archivo o directorio
        else if (strcmp(tr[i], "-acc") == 0) accessTime = 1;   // -acc muestra el último tiempo de acceso al archivo o directorio
        else if (strcmp(tr[i], "-link") == 0) linkInfo = 1;    // -link muestra el path en caso de que sea un enlace simbólico
        else {
            fprintf(stderr, "Opción no reconocida: %s\n", tr[i]);
            return;
        }
        i++;
    }

    // Si no se proporcionan directorios o archivos, usar el directorio actual llamando a cmd_cwd
    if (tr[i] == NULL) {
        // Llamamos a la función cmd_cwd para imprimir el directorio actual
        cmd_cwd(NULL);
        return;
    }

    // Procesar cada archivo
    while (tr[i] != NULL) {
        if (longFormat) {
            // Llamar a printFileInfo pasando las banderas adecuadas
            printFileInfo(tr[i], longFormat, linkInfo, accessTime);
        } else {
            // Mostrar el tiempo de acceso o modificación si se especificó -acc o -long
            struct stat file_info;
            if (lstat(tr[i], &file_info) == -1) {
                perror(tr[i]);
                i++;
                continue;
            }

            char time_str[20];
            struct tm *time_info;
            time_t time_to_show = accessTime ? file_info.st_atime : file_info.st_mtime;
            time_info = localtime(&time_to_show);
            strftime(time_str, sizeof(time_str), "%Y/%m/%d-%H:%M", time_info);

            if (accessTime) {
                printf("%s ", time_str);
            }

            printf("%8ld  %s", (long)file_info.st_size, tr[i]);

            if (linkInfo && S_ISLNK(file_info.st_mode)) {
                char linkpath[1024];
                ssize_t len = readlink(tr[i], linkpath, sizeof(linkpath) - 1);
                if (len != -1) {
                    linkpath[len] = '\0';
                    printf(" -> %s", linkpath);
                }
            }

            printf("\n");
        }
        i++;
    }
}


// Función que lista el contenido de archivos
void cmd_listdir(char *tr[]) {
    int long_format = 0, show_hidden = 0, show_access_time = 0, show_link = 0;
    int i, start_index = 1;

    // Procesar opciones
    for (i = 1; tr[i] != NULL && tr[i][0] == '-'; i++) {
        if (strcmp(tr[i], "-long") == 0) long_format = 1;
        else if (strcmp(tr[i], "-hid") == 0) show_hidden = 1;
        else if (strcmp(tr[i], "-acc") == 0) show_access_time = 1;
        else if (strcmp(tr[i], "-link") == 0) show_link = 1;
        else {
            printf("Opción no reconocida: %s\n", tr[i]);
            return;
        }
    }
    start_index = i;

    // Si no se proporcionan directorios, usar el directorio actual
    if (tr[start_index] == NULL) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd() error");
            return;
        }
    }

    // Listar cada directorio proporcionado
    for (i = start_index; tr[i] != NULL; i++) {
        struct stat file_info;
        char real_path[PATH_MAX];

        // Obtener el path real (resuelve enlaces simbólicos)
        if (realpath(tr[i], real_path) == NULL) {
            printf("****error al acceder a %s: %s\n", tr[i], strerror(errno));
            continue;
        }

        // Verificamos si podemos acceder al directorio o archivo
        if (lstat(tr[i], &file_info) == -1) {
            printf("****error al acceder a %s: %s\n", tr[i], strerror(errno));
            continue;
        }

        // Manejar enlaces simbólicos
        if (S_ISLNK(file_info.st_mode)) {
            char linkpath[PATH_MAX] = {0};
            ssize_t len = readlink(tr[i], linkpath, sizeof(linkpath) - 1);
            if (len != -1) {
                linkpath[len] = '\0';
                printf("%8ld %s -> %s\n", (long)file_info.st_size, tr[i], linkpath); // Tamaño del enlace simbólico
            } else {
                printf("****error al leer el enlace simbólico %s: %s\n", tr[i], strerror(errno));
            }

            // Actualizar file_info con la información del archivo al que apunta el enlace
            if (stat(real_path, &file_info) == -1) {
                printf("****error al acceder al destino del enlace %s: %s\n", tr[i], strerror(errno));
                continue;
            }
        }
        
        // Verificar si es un archivo y no un directorio
        if (S_ISREG(file_info.st_mode)) {
            // Manejar la salida para archivos, similar a como se hace con el contenido de directorios
            printFileInfo(tr[i], long_format, show_link, show_access_time);
            continue;
        }

        // Si es un directorio, proceder a listar su contenido
        if (S_ISDIR(file_info.st_mode)) {
            printf("************%s\n", tr[i]);
            DIR *dir = opendir(real_path);
            if (dir == NULL) {
                printf("****error al abrir el directorio %s: %s\n", real_path, strerror(errno));
                continue;
            }
            struct dirent *entry;

            // Leer cada entrada en el directorio
            while ((entry = readdir(dir)) != NULL) {
                // Ocultar archivos que no son visibles si no se ha solicitado mostrar ocultos
                if (!show_hidden && entry->d_name[0] == '.') {
                    continue;
                }

                char full_path[PATH_MAX];

                // Verificar si la longitud de las cadenas excede el tamaño del buffer
                if (snprintf(full_path, sizeof(full_path), "%s/%s", real_path, entry->d_name) >= sizeof(full_path)) {
                    printf("Ruta demasiado larga: %s/%s\n", real_path, entry->d_name);
                    continue;
                }

                // Obtener información del archivo
                if (lstat(full_path, &file_info) == -1) {
                    perror("Error al obtener información del archivo");
                    continue;
                }

                // Obtener tiempo para mostrar (modificación o acceso)
                char time_buf[80];
                struct tm *tm_info;

                // Seleccionar el tiempo de modificación o acceso según la opción
                time_t time_to_show = show_access_time ? file_info.st_atime : file_info.st_mtime;
                tm_info = localtime(&time_to_show);
                strftime(time_buf, sizeof(time_buf), "%Y/%m/%d-%H:%M", tm_info);

                // Imprimir en formato largo con alineación mejorada
                if (long_format) {
                    char permisos[11];
                    ConvierteModo(file_info.st_mode, permisos); // Convertir permisos a cadena

                    printf("%-19s %3ld (%8ld) %8s %8s %s %8ld %s",
                           time_buf,
                           (long)file_info.st_nlink,
                           (long)file_info.st_ino,
                           getpwuid(file_info.st_uid)->pw_name,
                           getgrgid(file_info.st_gid)->gr_name,
                           permisos,
                           (long)file_info.st_size,
                           entry->d_name);
                } else {
                    if (show_access_time) {   // Si se pide -acc mostramos el último tiempo de acceso
                         printf("%-19s %8ld %s", time_buf, (long)file_info.st_size, entry->d_name);
                    } else {
                        printf("%8ld %s", (long)file_info.st_size, entry->d_name);
                    }
                }

                // Mostrar información de enlace simbólico si se solicita
                if (show_link && S_ISLNK(file_info.st_mode)) {
                    char linkpath[PATH_MAX] = {0};
                    ssize_t len = readlink(full_path, linkpath, sizeof(linkpath) - 1);
                    if (len != -1) {
                        linkpath[len] = '\0';
                        printf(" -> %s\n", linkpath);
                    } else {
                        printf(" -> [Error: %s]", strerror(errno));
                    }
                }

                printf("\n");
            }
            closedir(dir);
        } else {
            printf("****%s no es un directorio\n", tr[i]);
        }
    }
}


// Función que lista directorios de manera recursiva (subdirectorios después)
void cmd_reclist(char *tr[]) {
    int long_format = 0, show_hidden = 0, show_access_time = 0, show_link = 0;
    int i, start_index = 1;

    // Procesar opciones
    for (i = 1; tr[i] != NULL && tr[i][0] == '-'; i++) {
        if (strcmp(tr[i], "-long") == 0) long_format = 1;
        else if (strcmp(tr[i], "-hid") == 0) show_hidden = 1;
        else if (strcmp(tr[i], "-acc") == 0) show_access_time = 1;
        else if (strcmp(tr[i], "-link") == 0) show_link = 1;
        else {
            printf("Opción no reconocida: %s\n", tr[i]);
            return;
        }
    }
    start_index = i;
    // Si no se proporcionan directorios o archivos, usar el directorio actual llamando a cmd_cwd
    if (tr[i] == NULL) {
        // Llamamos a la función cmd_cwd para imprimir el directorio actual
        cmd_cwd(NULL);
        return;
    }

    // Procesar cada argumento proporcionado
    for (i = start_index; tr[i] != NULL; i++) {
        struct stat file_info;
        if (lstat(tr[i], &file_info) == -1) {
            printf("Error al obtener información de %s: %s\n", tr[i], strerror(errno));
            continue;
        }

        if (S_ISDIR(file_info.st_mode)) {
            // Si es un directorio, procesarlo recursivamente
            reclist_recursive(tr[i], long_format, show_hidden, show_access_time, show_link);
        } else {
            // Si es un archivo, mostrar su información
            printFileInfo(tr[i], long_format, show_link, show_access_time);
        }
    }
}

// Definición de la función recursiva para listar directorios (subdirectorios después)
void reclist_recursive(const char *path, int long_format, int show_hidden, int show_access_time, int show_link) {
    DIR *dir;
    struct dirent *entry;
    char full_path[PATH_MAX];

    dir = opendir(path);
    if (dir == NULL) {
        printf("****error al abrir el directorio %s: %s\n", path, strerror(errno));
        return;
    }

    printf("************%s\n", path);

    // Mostrar todos los archivos y directorios (incluidos los ocultos si se indica)
    while ((entry = readdir(dir)) != NULL) {
        // Construir la ruta completa
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        struct stat file_info;
        if (lstat(full_path, &file_info) == -1) {
            printf("Error al obtener información de %s: %s\n", entry->d_name, strerror(errno));
            continue;
        }

        // Mostrar el archivo/directorio si:
        // 1. Se deben mostrar archivos ocultos (show_hidden es verdadero)
        // 2. No es un archivo oculto (no comienza con '.')
        // 3. Es '.' o '..' y se deben mostrar archivos ocultos
        if (show_hidden || entry->d_name[0] != '.' || (show_hidden && (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0))) {
            // Imprimir información del archivo o directorio
            printFileInfo(full_path, long_format, show_link, show_access_time);
        }
    }
    
    // Reiniciar el directorio para buscar subdirectorios
    rewinddir(dir);
    while ((entry = readdir(dir)) != NULL) {
        // Omitir los directorios especiales "." y ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        struct stat file_info;
        if (lstat(full_path, &file_info) == -1) {
            continue;
        }

        if (S_ISDIR(file_info.st_mode)) {
            // Llamada recursiva para el directorio si:
            // 1. No es un directorio oculto
            // 2. Es un directorio oculto pero se deben mostrar archivos ocultos
            if (entry->d_name[0] != '.' || show_hidden) {
                reclist_recursive(full_path, long_format, show_hidden, show_access_time, show_link);
            }
        }
    }

    closedir(dir);
}


// Función que lista archivos de manera inversa (subdirectorios antes)
void cmd_revlist(char *tr[]) {
    int long_format = 0, show_hidden = 0, show_access_time = 0, show_link = 0;
    int i, start_index = 1;

    // Procesar opciones
    for (i = 1; tr[i] != NULL && tr[i][0] == '-'; i++) {
        if (strcmp(tr[i], "-long") == 0) long_format = 1;
        else if (strcmp(tr[i], "-hid") == 0) show_hidden = 1;
        else if (strcmp(tr[i], "-acc") == 0) show_access_time = 1;
        else if (strcmp(tr[i], "-link") == 0) show_link = 1;
        else {
            printf("Opción no reconocida: %s\n", tr[i]);
            return;
        }
    }
    start_index = i;

    // Si no se proporcionan directorios o archivos, usar el directorio actual llamando a cmd_cwd
    if (tr[i] == NULL) {
        // Llamamos a la función cmd_cwd para imprimir el directorio actual
        cmd_cwd(NULL);
        return;
    }

    // Procesar cada argumento proporcionado
    for (i = start_index; tr[i] != NULL; i++) {
        struct stat file_info;
        if (lstat(tr[i], &file_info) == -1) {
            printf("Error al obtener información de %s: %s\n", tr[i], strerror(errno));
            continue;
        }

        if (S_ISDIR(file_info.st_mode)) {
            // Si es un directorio, procesarlo recursivamente con subdirectorios antes
            revlist_recursive(tr[i], long_format, show_hidden, show_access_time, show_link);
        } else {
            // Si es un archivo, mostrar su información
            printFileInfo(tr[i], long_format, show_link, show_access_time);
        }
    }
}

// Definición de la función recursiva para listar directorios de manera inversa 
void revlist_recursive(const char *path, int long_format, int show_hidden, int show_access_time, int show_link) {
    DIR *dir;
    struct dirent *entry;
    char full_path[PATH_MAX];
    struct stat file_info;

    dir = opendir(path);
    if (dir == NULL) {
        printf("****error al abrir el directorio %s: %s\n", path, strerror(errno));
        return;
    }

    // Almacenar los subdirectorios
    char *subdirs[1024];
    int subdir_count = 0;

    // Primera pasada: almacenar los subdirectorios
    while ((entry = readdir(dir)) != NULL) {
        if (!show_hidden && entry->d_name[0] == '.') {
            continue; // Ignorar archivos ocultos
        }

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; // Ignorar '.' y '..'
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        if (lstat(full_path, &file_info) == -1) {
            printf("Error al obtener información de %s: %s\n", entry->d_name, strerror(errno));
            continue;
        }

        // Si es un directorio, lo almacenamos en la lista
        if (S_ISDIR(file_info.st_mode)) {
            subdirs[subdir_count] = strdup(full_path); // Almacenar el path del subdirectorio
            subdir_count++;
        }
    }

    // Segunda pasada: procesar primero los subdirectorios
    for (int i = 0; i < subdir_count; i++) {
        revlist_recursive(subdirs[i], long_format, show_hidden, show_access_time, show_link);
    }

    // Imprimir el nombre del directorio actual después de haber listado su contenido
    printf("************%s\n", path);

    // Tercera pasada: imprimir los archivos en el directorio actual
    rewinddir(dir); // Regresar al inicio del directorio para leer los archivos
    while ((entry = readdir(dir)) != NULL) {
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        if (lstat(full_path, &file_info) == -1) {
            printf("Error al obtener información de %s: %s\n", entry->d_name, strerror(errno));
            continue;
        }

        // Si es un archivo, imprimir su información
        if (S_ISREG(file_info.st_mode)) {
            printFileInfo(full_path, long_format, show_link, show_access_time); // Imprimir el archivo
        }
    }

    // Imprimir los directorios ocultos '.' y '..' si se solicita
    if (show_hidden) {
        printf("    %8ld  .\n", file_info.st_size); // Tamaño de '.' (directorio actual)
        printf("    %8ld  ..\n", file_info.st_size); // Tamaño de '..' (directorio padre)
    }

    closedir(dir);
}


// Función que elimina archivos y/o directorios vacíos
void cmd_erase(char *arg) {
    // Verificar si se pasó un argumento (nombre del archivo)
    if (arg == NULL || *arg == '\0') {
        // Si no se pasa un argumento, llamamos a cmd_cwd para imprimir el directorio de trabajo actual
        cmd_cwd(NULL);
        return;
    }
    struct stat st;

    // Verificar si el archivo o directorio existe
    if (stat(arg, &st) != 0) {
        perror("Error al acceder al archivo o directorio");
        return;
    }

    // Verificar si es un archivo regular
    if (S_ISREG(st.st_mode)) {
        // Intentar eliminar el archivo directamente, sin importar si está vacío o no
        if (remove(arg) == 0) {
            printf("Archivo %s eliminado correctamente\n", arg);
        } else {
            perror("Error al eliminar el archivo");
        }
    } 
    // Verificar si es un directorio
    else if (S_ISDIR(st.st_mode)) {
        DIR *dir = opendir(arg);
        struct dirent *entry;
        int isEmpty = 1; // Asumimos que el directorio está vacío

        if (dir == NULL) {
            perror("Error al abrir el directorio");
            return;
        }

        // Leer el contenido del directorio
        while ((entry = readdir(dir)) != NULL) {
            // Ignorar los directorios especiales '.' y '..'
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                isEmpty = 0; // El directorio no está vacío
                break;
            }
        }
        closedir(dir);

        // Si está vacío, intentamos eliminarlo
        if (isEmpty) {
            if (rmdir(arg) == 0) {
                printf("Directorio %s eliminado correctamente\n", arg);
            } else {
                perror("Error al eliminar el directorio");
            }
        } else {
            // Si no está vacío, informamos que no se puede eliminar
            printf("Imposible borrar %s: Directorio not empty\n", arg);
        }
    } else {
        // Si no es un archivo ni un directorio válido, se informa
        printf("El path no es un archivo ni un directorio válido: '%s'\n", arg);
    }
}


// Función que elimina archivos y/o directorios no vacíos de manera recursiva
void cmd_delrec(char *arg) {
    struct stat st;

    // Verificar si el archivo o directorio existe
    if (stat(arg, &st) != 0) {
        perror("Error al acceder al archivo o directorio");
        return;
    }

    // Si es un archivo regular, eliminarlo sin importar su tamaño
    if (S_ISREG(st.st_mode)) {
        if (remove(arg) == 0) {
            printf("Archivo %s eliminado correctamente\n", arg);
        } else {
            perror("Error al eliminar el archivo");
        }
        return;
    }

    // Si es un directorio
    if (S_ISDIR(st.st_mode)) {
        DIR *dir = opendir(arg);
        struct dirent *entry;
        int isEmpty = 1; // Asumimos que el directorio está vacío inicialmente

        if (dir == NULL) {
            perror("Error al abrir el directorio");
            return;
        }

        // Verificar si el directorio está vacío
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                isEmpty = 0; // El directorio no está vacío
                break;
            }
        }
        closedir(dir);

        // Si el directorio está vacío, no se puede borrar
        if (isEmpty) {
            printf("Imposible borrar %s: Directorio vacío\n", arg);
        } else {
            // Si no está vacío, llamar a la función de eliminación recursiva
            delete_recursive(arg);
        }
    } else {
        printf("El path no es un archivo ni un directorio válido: %s\n", arg);
    }
}


// Función auxiliar que elimina recursivamente el contenido de un directorio
void delete_recursive(const char *path) {
    DIR *dir;
    struct dirent *entry;
    char file_path[1024];

    // Abrir el directorio
    dir = opendir(path);
    if (dir == NULL) {
        perror("Error al abrir el directorio");
        return;
    }

    // Recorrer todas las entradas del directorio
    while ((entry = readdir(dir)) != NULL) {
        // Ignorar los directorios especiales "." y ".."
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            // Construir la ruta completa del archivo o subdirectorio
            snprintf(file_path, sizeof(file_path), "%s/%s", path, entry->d_name);
            
            struct stat st;
            if (stat(file_path, &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    // Si es un directorio, llamar recursivamente a delete_recursive
                    delete_recursive(file_path);
                } else {
                    // Si es un archivo, eliminarlo
                    if (remove(file_path) == 0) {
                        printf("Archivo %s eliminado correctamente\n", file_path);
                    } else {
                        perror("Error al eliminar el archivo");
                    }
                }
            }
        }
    }
    closedir(dir);
    
    // Eliminar el directorio después de haber eliminado todo su contenido
    if (rmdir(path) == 0) {
        printf("Directorio %s eliminado correctamente\n", path);
    } else {
        perror("Error al eliminar el directorio");
    }
}



/// ---- FUNCIONES P2 (memoria) ----  ////
// Función que asigna bloques de memoria de diferentes tipos
void cmd_allocate(char *tr[]) {
    if (tr[1] == NULL) {
        // Mostrar la lista general de bloques asignados
        printf("******Lista de bloques asignados para el proceso %d\n", getpid());
       if (memoria.general.head == NULL) {
                printf("No hay bloques de memoria asignados\n");
            } else {
                ImprimirListaMemoria(&memoria.general);
            }
            return;
    }

    // Argumento -malloc: Asignar memoria 
    if (strcmp(tr[1], "-malloc") == 0) {       //allocate -malloc n
        if (tr[2] == NULL) {    // tr[2] es el tamaño
            // Si no se pasa un tamaño, mostrar solo la lista de bloques asignados con malloc
            printf("******Lista de bloques asignados malloc para el proceso %d\n", getpid());
            if (memoria.mallocList.head == NULL) {
                printf("No hay bloques de memoria malloc asignados\n");
            } else {
                ImprimirListaMalloc(&memoria.mallocList);
            }
            return;
        }

        // Verificar si el tamaño proporcionado es válido
        size_t size = (size_t)atoi(tr[2]);
        if (size <= 0) {
            // Si el tamaño es inválido, imprimir el mensaje de error
            printf("Error: el tamaño debe ser un número positivo\n");
            return;
        }

        // Intentar asignar memoria del tamaño especificado
        void *ptr = malloc(size);
        if (ptr == NULL) {
            printf("Error: no se pudo asignar memoria\n");
            return;
        }

        // Imprimir información sobre la asignación realizada
        printf("Asignados %zu bytes en %p\n", size, ptr);

        // Agregar el bloque de memoria a la lista malloc y a la lista general
        InsertMallocMemory(&memoria, ptr, size);
        return;
    }

    // Argumento -mmap: Mapear un archivo
    if (strcmp(tr[1], "-mmap") == 0) {          // allocate -mmap archivo permisos
        if (tr[2] == NULL) {   // tr[2] es el nombre del archivo
             printf("******Lista de bloques asignados mmap para el proceso %d\n", getpid());
            if (memoria.mmapList.head == NULL) {
                printf("No hay bloques de memoria mmap asignados\n");
            } else {
                ImprimirListaMmap(&memoria.mmapList);
            }
            return;
        }

        // Mapear el archivo con los permisos especificados (El + 2 desplaza el puntero tr dos posiciones hacia adelante para pasar solo los argumentos relevantes para do_AllocateMmap)
        do_AllocateMmap(tr + 2, &memoria); // Pasar solo los argumentos necesarios para mapear (tr+2 porque do_AllocateMmap necesita recibir los argumentos a partir de tr[2])
                                           // solo quiere recibir el nombre de archivo y sus permisos
        return; 
    }

    // Argumento -createshared: Crear y asignar memoria compartida
    if (strcmp(tr[1], "-createshared") == 0) {
        if (tr[2] == NULL || tr[3] == NULL) {    // tr[2] es la clave y tr[3] es el tamaño
            printf("******Lista de bloques asignados shared para el proceso %d\n", getpid());
            if (memoria.sharedList.head == NULL) {
                printf("No hay bloques de memoria shared asignados\n");
            } else {
                ImprimirListaShared(&memoria.sharedList);
            }
            return;
        }
        do_AllocateCreateshared(&tr[2]);
        return;
    }

    // Argumento -shared: Asignar memoria compartida existente
    if (strcmp(tr[1], "-shared") == 0) {
         if (tr[2] == NULL) {   // tr[2] es la clave
            printf("******Lista de bloques asignados shared para el proceso %d\n", getpid());
            if (memoria.sharedList.head == NULL) {
                printf("No hay bloques de memoria shared asignados\n");
            } else {
                ImprimirListaShared(&memoria.sharedList);
            }
            return;
        }
        do_AllocateShared(&tr[2]);
        return;
    }

    // Si el primer argumento no es válido, mostramos el uso adecuado del comando
    printf("Uso: allocate [-malloc|-shared|-createshared|-mmap] ....\n");
}


// Función que libera bloques de memoria
void cmd_deallocate(char *tr[]) {
    // Si no se pasa ningún argumento, imprime la lista general de bloques de memoria
    if (tr[1] == NULL) {
        printf("******Lista de bloques asignados para el proceso %d\n", getpid());
        if (memoria.general.head == NULL) {
            printf("No hay bloques de memoria asignados\n");
        } else {
            ImprimirListaMemoria(&memoria.general);
        }
        return;
    }

    // Argumento -malloc: Deasigna un bloque de memoria 
    if (strcmp(tr[1], "-malloc") == 0) {
        if (tr[2] == NULL) {
            printf("******Lista de bloques asignados malloc para el proceso %d\n", getpid());
            ImprimirListaMalloc(&memoria.mallocList);
            return;
        }

        // Convertir el tamaño a size_t y verificar que sea válido
        size_t size = (size_t)atoi(tr[2]);
        if (size <= 0) {
            printf("Error: el tamaño debe ser un número positivo\n");
            return;
        }

        // Eliminar memoria malloc del proceso
        DeleteMallocMemory(&memoria, size);
        return;
    }

    // Argumento -mmap: Deasigna un bloque de memoria mapeado
    if (strcmp(tr[1], "-mmap") == 0) {
        if (tr[2] == NULL) {
            printf("******Lista de bloques asignados mmap para el proceso %d\n", getpid());
            ImprimirListaMmap(&memoria.mmapList);
            return;
        }

        // Asumimos que es un nombre de archivo
        DeleteMmapMemory(&memoria, tr[2]);
        return;
    }

    // Argumento -malloc: Desacopla un bloque de memoria compartida del proceso actual. Esto no afecta a otros procesos que puedan estar utilizando la memoria compartida.
    if (strcmp(tr[1], "-shared") == 0) {
        if (tr[2] == NULL) {
            printf("******Lista de bloques asignados shared para el proceso %d\n", getpid());
            ImprimirListaShared(&memoria.sharedList);
            return;
        }

        // Convertir la clave de memoria compartida a key_t
        key_t key = (key_t)strtoul(tr[2], NULL, 10);   // usamos 10 indicando que el número de la cadena (de la clave) se interpreta en base10
        DeleteSharedMemory(&memoria, key);
        return;
    }

       // Argumento -delkey: Elimina la clave de memoria compartida del sistema
    if (strcmp(tr[1], "-delkey") == 0) {
        if (tr[2] == NULL) {
            printf("\tdelkey: necesita clave valida\n");
            return;
        }

        key_t key = (key_t)strtoul(tr[2], NULL, 10);
        DeleteSharedMemoryByKey(&memoria, key);  //elimina la clave del sistema sin desacoplar la memoria ni actualizar la lista loca
        }
     

    // Argumento addr: Si no es ninguno de los anteriores argumentos, se asume que es una dirección de memoria
    void *addr = (void *)strtoull(tr[1], NULL, 16); // Convertir la dirección a un puntero. Usamos 16 para indicar que la cadena es hexadecimal
    if (addr == NULL) {
        printf("Error: Dirección no válida\n");
        return;
    }

    // Buscar el bloque de memoria en la lista general
    MemoryNode *block = FindMemoryBlock(&memoria, addr);
    if (block == NULL) {
        printf("Error: Dirección no encontrada\n");
        return;
    }

    // Determinar el tipo de memoria y eliminarla de forma centralizada
    switch (block->type) {
        case MALLOC:
            DeleteMallocMemory(&memoria, block->size);
            break;
        case MMAP:
            DeleteMmapMemorybyAddr(&memoria, addr); // Eliminamos mmap por dirección al final
            break;
        case SHARED:
            DeleteMemSharedByAddr(&memoria, addr);
            break;
        default:
            printf("Error: Tipo de memoria no manejado para esta dirección\n");
    }
}

// Función que escribe en un archivo cont bytes comenzando en la dirección de memoria addr
void cmd_memfill(char *tr[]) {
    void *addr;          // Dirección donde se llenará la memoria
    size_t cont;         // Número de bytes a llenar
    unsigned char ch;    // Carácter con el que se llenará (en decimal en ASCII)

    // Validar argumentos
    if (tr[1] == NULL || tr[2] == NULL || tr[3] == NULL) {
        printf("Uso: memfill addr cont ch\n");
        return;
    }

    // Convertir el primer argumento (addr) a un puntero de memoria
    addr = (void *)strtoull(tr[1], NULL, 16);

    // Verificar que la dirección existe en la lista
    MemoryNode *block = FindMemoryBlock(&memoria, addr);
    if (block == NULL) {
        printf("Error: La dirección %p no está gestionada por el programa\n", addr);
        return;
    }

    // Convertir el segundo argumento (cont) a un número de bytes
    cont = (size_t)strtoul(tr[2], NULL, 10);
    if (cont == 0) {
        printf("Error: la cantidad de bytes tiene que ser mayor que 0\n");
        return;
    }

    // Verificar que el tamaño no exceda los límites del bloque
    if ((char*)addr + cont > (char*)block->addr + block->size) {
        printf("Error: El rango [%p, %p) excede los límites del bloque gestionado [%p, %p)\n",
               addr, (char*)addr + cont, block->addr, (char*)block->addr + block->size);
        return;
    }

    // Convertir el tercer argumento (ch) a un carácter
    ch = (unsigned char)strtoul(tr[3], NULL, 10);

    // Llenar la memoria usando la función LlenarMemoria
    LlenarMemoria(addr, cont, ch);

    // Confirmar la operación
    printf("Llenados %zu bytes en la dirección %p con el carácter '%c' (0x%02x)\n", 
           cont, addr, ch, ch);
}


// Función que vuelca el contenido de cont bytes de memoria en la dirección addr a la pantalla
void cmd_memdump(char *tr[]) {
    if (!tr[1] || !tr[2]) {
        printf("Uso: memdump addr cont\n");
        return;
    }

    // Convertir argumentos a dirección y tamaño
    void *addr = (void *)strtoull(tr[1], NULL, 16); // Dirección en hexadecimal
    size_t size = strtoull(tr[2], NULL, 10);   // Tamaño en bytes

    if (addr == NULL || size == 0) {
        printf("Error: Dirección o tamaño inválidos.\n");
        return;
    }

    // Buscar si la dirección pertenece a un bloque válido en la lista general
    MemoryNode *node = FindMemoryBlock(&memoria, addr);
    if (!node) {
        printf("Error: La dirección %p no pertenece a un bloque válido.\n", addr);
        return;
    }

    // Validar si el rango del volcado está dentro del tamaño del bloque
    if ((char *)addr + size > (char *)node->addr + node->size) {
        printf("Error: El rango excede el tamaño del bloque de memoria.\n");
        return;
    }

    // Realizar el volcado de memoria
    unsigned char *mem = (unsigned char *)addr;
    printf("Volcando %zu bytes desde %p:\n", size, addr);

    for (size_t i = 0; i < size; i++) {
        // Dirección base de cada línea
        if (i % 16 == 0) {
            printf("%p: ", (void *)(mem + i));
        }

        // Imprimir byte en hexadecimal
        int byte_value;
        if (readByte(mem + i, &byte_value) == 0) {
            printf("%02x ", byte_value);
        } else {
            printf("?? ");
        }

        // Imprimir caracteres ASCII al final de cada línea o al final del volcado
        if ((i + 1) % 16 == 0 || i == size - 1) {
            // Rellenar con espacios si la última línea está incompleta
            for (size_t j = i % 16; j < 15; j++) {
                printf("   ");
            }
            printf("| ");
            for (size_t j = i - (i % 16); j <= i; j++) {
                int ch;
                if (readByte(mem + j, &ch) == 0) {
                    printf("%c", isprint(ch) ? ch : '.');
                } else {
                    printf("?");
                }
            }
            printf("\n");
        }
    }
}


// Función que muestra información sobre la memoria del programa
void cmd_memory(char *tr[]) {
    int i = 1;
    int show_vars = 0, show_funcs = 0, show_blocks = 0, show_all = 0, show_pmap = 0;

    if (tr[1] == NULL) {
        // Si no hay argumentos, mostrar todo
        show_all = 1;
    } else {
        // Procesar argumentos
        while (tr[i] != NULL) {
            if (strcmp(tr[i], "-vars") == 0) show_vars = 1;
            else if (strcmp(tr[i], "-funcs") == 0) show_funcs = 1;
            else if (strcmp(tr[i], "-blocks") == 0) show_blocks = 1;
            else if (strcmp(tr[i], "-all") == 0) show_all = 1;
            else if (strcmp(tr[i], "-pmap") == 0) show_pmap = 1;
            else {
                printf("Opcion %s no contemplada\n", tr[i]);
                printf("Uso: memory [-vars|-funcs|-blocks|-all|-pmap]\n");
                return;
            }
            i++;
        }
    }

    // Caso 1: -all y/o -funcs: muestra direcciones de funciones del programa y de la biblioteca
    if (show_all || show_funcs) {
        printf("Funciones programa      %p,   %p,   %p\n", &cmd_authors, &cmd_pid, &cmd_cwd);  // Direcciones de funciones definidas en este programa
        printf("Funciones libreria      %p,   %p,   %p\n", &printf, &strcmp, &malloc);         // Direcciones de funciones de bibliotecas estándar
    }

    // Caso 2: -all y/o -vars: muestra direcciones de variables locales, globales y estáticas
    if (show_all || show_vars) {
        static int static_var1, static_var2, static_var3;  // Variables estáticas no inicializadas
        static int static_init_var1 = 1, static_init_var2 = 2, static_init_var3 = 3;  // Variables estáticas inicializadas
        int auto_var1, auto_var2, auto_var3;  // Variables automáticas (locales)

        // Variables locales: en la pila (stack), de duración limitada a la función
        printf("Variables locales       %p,   %p,   %p\n", &auto_var1, &auto_var2, &auto_var3);  

        // Variables globales inicializadas: en el segmento de datos inicializados (data)
        printf("Variables globales      %p,   %p,   %p\n", &historico, &ficheros, &memoria);     

        // Variables globales no inicializadas: en el segmento de datos inicializados (data) 
        printf("Var (N.I.)globales      %p,   %p,   %p\n", &environ, &stdout, &stdin);

        // Variables estáticas inicializadas: en el segmento BSS
        printf("Variables estaticas     %p,   %p,   %p\n", &static_init_var1, &static_init_var2, &static_init_var3);

        // Variables estáticas no inicializadas: en el segmento BSS, que se inicializa a 0 automáticamente 
        printf("Var (N.I.)estaticas     %p,   %p,   %p\n", &static_var1, &static_var2, &static_var3);
    }

    // Caso 3: -all y/o -blocks: muestra la lista de bloques de memoria asignados
    if (show_all || show_blocks) {
        printf("******Lista de bloques asignados para el proceso %d\n", getpid());
        ImprimirListaMemoria(&memoria.general);
    }

    // Caso 4: -pmap: muestra la información de la memoria del proceso usando herramientas externas
    if (show_pmap) {
        Do_pmap();
    }
}


// Función que lee cont bytes de un archivo en una dirección de memoria 
void cmd_readfile(char *tr[]) {
    void *p;           // Dirección de memoria donde se leerá
    size_t cont = -1;  // Cantidad de bytes a leer
    ssize_t n;         // Número de bytes leídos

    // Validación de argumentos
    if (tr[1] == NULL || tr[2] == NULL) {   // tr[1] es el nombre del archivo (fiche) y tr[2] es la dirección de memoria (addr)
        printf("faltan parametros\n");
        printf("Uso: readfile fiche addr cont\n");
        return;
    }

    // Validar acceso al archivo (access() devuelve 0 si se puede leer)
    if (access(tr[1], R_OK) == -1) {
        perror("Error al acceder al archivo");
        return;
    }

    // Convertir dirección de memoria
    p = cadtop(tr[2]);
    if (p == NULL) {
        printf("Dirección de memoria inválida: %s\n", tr[2]);
        return;
    }

    // Determinar la cantidad de bytes
    if (tr[3] != NULL) {
        cont = (size_t) atoll(tr[3]);
        if (cont == 0) {
            printf("La cantidad de bytes a leer debe ser mayor que cero\n");
            return;
        }
    }

    // Leer el archivo
    n = LeerFichero(tr[1], p, cont);
    if (n == -1) {
        perror("Imposible leer fichero");
    } else {
        printf("Leídos %zd bytes de %s en %p\n", n, tr[1], p);
    }
}


// Función que escribe en un archivo cont bytes comenzando en la dirección de memoria addr los sobreescribe usando -o si ya existe
void cmd_writefile(char *tr[]) {
    void *addr;        // Dirección de memoria desde donde se escribirá
    size_t cont;       // Número de bytes a escribir
    int overwrite = 0; // Bandera para el modo de sobreescritura
    int fd;            // Descriptor de archivo
    ssize_t written;   // Bytes realmente escritos

    // Validación de argumentos mínimos
    if (tr[1] == NULL || (strcmp(tr[1], "-o") == 0 && (tr[2] == NULL || tr[3] == NULL || tr[4] == NULL)) ||
        (strcmp(tr[1], "-o") != 0 && (tr[2] == NULL || tr[3] == NULL))) {
            printf("faltan parametros\n");
        printf("Uso: writefile [-o] fich addr cont\n");
        return;
    }

    // Comprobar si se pasó la opción "-o" para sobrescribir
    if (strcmp(tr[1], "-o") == 0) {
        overwrite = 1;
        tr++; // Avanzar los argumentos para ignorar "-o"
    }

    // Abrir el archivo en el modo correspondiente
    if (overwrite) {
        fd = open(tr[1], O_WRONLY | O_CREAT | O_TRUNC, 0644); // Sobrescribir archivo
    } else {
        fd = open(tr[1], O_WRONLY | O_CREAT | O_EXCL, 0644); // Crear nuevo archivo (error si ya existe)
    }

    if (fd == -1) {
        if (errno == EEXIST && !overwrite) {
            printf("Error: el archivo %s ya existe. Use -o para sobrescribir.\n", tr[1]);
        } else {
            perror("Imposible abrir fichero");
        }
        return;
    }

    // Convertir dirección y cantidad de bytes desde las cadenas
    addr = cadtop(tr[2]);           // Convertir cadena a puntero
    if (addr == NULL) {
        printf("Dirección de memoria inválida: %s\n", tr[2]);
        close(fd);
        return;
    }

    // Verificar que tr[3] no sea NULL antes de usar atoll
    if (tr[3] == NULL) {
        printf("Error: Falta especificar la cantidad de bytes a escribir\n");
        close(fd);
        return;
    }
    cont = (size_t)atoll(tr[3]);    // Número de bytes a escribir

    // Validar que la cantidad de bytes sea mayor que cero
    if (cont == 0) {
        printf("Error: La cantidad de bytes a escribir debe ser mayor que cero\n");
        close(fd);
        return;
    }

    // Intentar escribir en el archivo
    written = write(fd, addr, cont);

    if (written == -1) {
        perror("Imposible escribir fichero");
    } else {
        printf("Escritos %zd bytes en %s desde %p\n", written, tr[1], addr);
    }

    // Cerrar el archivo
    close(fd);
}


// Función que lee cont bytes de un archivo en una dirección de memoria si tiene un descriptor de archivo abierto (fd)
void cmd_read(char *tr[]) {
    // Validación de parámetros
    if (tr[1] == NULL || tr[2] == NULL || tr[3] == NULL) {
        printf("Faltan parámetros\n");
        printf("Uso: read df addr cont\n");
        return;
    }

    // Conversión de parámetros
    int df = atoi(tr[1]); // Convertir el descriptor de archivo de cadena a entero
    void *addr = cadtop(tr[2]);  // Convertir la dirección de memoria de cadena a puntero
    size_t cont = atoll(tr[3]);  // Convertir el número de bytes a leer de cadena a tamaño

    // Verificación del descriptor de archivo
    if (df < 0 || df > 19) {
        printf("Descriptor de archivo inválido\n");
        return;
    }

    // Verificar si el descriptor está en la lista de ficheros abiertos
    const char *nombreArchivo = nombreFicheroDescriptor(df, ficheros);
    if (nombreArchivo == NULL) {
        printf("El descriptor %d no está en la lista de ficheros abiertos\n", df);
        return;
    }

    // Verificación de la dirección de memoria
    if (addr == NULL) {
        printf("Dirección de memoria inválida: %s\n", tr[2]);
        return;
    }

    // Verificación de que la cantidad de bytes sea válida
    if (cont == 0) {
        printf("La cantidad de bytes a leer debe ser mayor que cero\n");
        return;
    }

    // Leer los datos desde el archivo al buffer en addr
    ssize_t bytes_read = read(df, addr, cont);
    if (bytes_read == -1) {
        printf("Error al leer del descriptor %d (%s): %s\n", df, nombreArchivo, strerror(errno));
    } else {
        printf("Leídos %zd bytes del descriptor %d (%s) en la dirección %p\n", bytes_read, df, nombreArchivo, addr);
    }
}


// Función que escribe en un archivo cont bytes comenzando en la dirección de memoria addr si tiene un descriptor de archivo abierto (fd)
void cmd_write(char *tr[]) {
    // Validación de parámetros
    if (tr[1] == NULL || tr[2] == NULL || tr[3] == NULL) {
        printf("Faltan parámetros\n");
        printf("Uso: write df addr cont\n");
        return;
    }

    // Conversión de parámetros
    int df = atoi(tr[1]); // Convertir el descriptor de archivo de cadena a entero
    void *addr = cadtop(tr[2]);  // Convertir la dirección de memoria de cadena a puntero
    size_t cont = atoll(tr[3]);  // Convertir el número de bytes a escribir de cadena a tamaño

    // Verificación del descriptor de archivo
    if (df < 0 || df > 19) {
        printf("Descriptor de archivo inválido\n");
        return;
    }

    // Verificar si el descriptor está en la lista de ficheros abiertos
    const char *nombreArchivo = nombreFicheroDescriptor(df, ficheros);
    if (nombreArchivo == NULL) {
        printf("El descriptor %d no está en la lista de ficheros abiertos\n", df);
        return;
    }

    // Verificación de la dirección de memoria
    if (addr == NULL) {
        printf("Dirección de memoria inválida: %s\\n", tr[2]);
        return;
    }

    // Verificación de que la cantidad de bytes sea válida
    if (cont == 0) {
        printf("La cantidad de bytes a escribir debe ser mayor que cero\n");
        return;
    }

    // Escribir los datos desde el buffer en addr al archivo
    ssize_t bytes_written = write(df, addr, cont);
    if (bytes_written == -1) {
        printf("Error al escribir en el descriptor %d (%s) desde la dirección %p: %s\n", df, nombreArchivo, addr, strerror(errno));
    } else {
        printf("Escritos %zd bytes en el descriptor %d (%s) desde la dirección %p\n", bytes_written, df, nombreArchivo, addr);
    }
}


// Función auxiliar que muestra como se comportan las variables locales estáticas (en el segmento de datos estático) y automáticas (en el stack)
void Recursiva (int n){
  char automatico[TAMANO];
  static char estatico[TAMANO];   //está inicializado a 0, por lo que se guardará en el segmento de datos

  printf ("parametro:%3d(%p) array %p, arr estatico %p\n",n,&n,automatico, estatico);

  if (n>0)
    Recursiva(n-1);
}


// Función que llama a recursiva con el valor de n proporcionado
void cmd_recurse(char *tr[]) {
    if (tr[1] == NULL) {
        printf("Uso: recurse n\n");
        return;
    }

    int n = atoi(tr[1]);
    if (n <= 0) {
        printf("El número de recursiones debe ser positivo\\n");
        return;
    }

    Recursiva(n);
}


/// ---- FUNCIONES P3 (procesos) ----  ////
// Función que muestra el UID del proceso que ejecuta el shell
void cmd_getuid(char *arg) {
    // Obtener el ID de usuario real y efectivo del proceso
    uid_t real_uid = getuid();
    uid_t effective_uid = geteuid();

    // Obtener los nombres de usuario a partir de los UID
    struct passwd *real_user = getpwuid(real_uid);
    struct passwd *effective_user = getpwuid(effective_uid);

    // Imprimir las credenciales
    if (real_user && effective_user) {
        printf("Credencial real: %d, (%s)\n", real_uid, real_user->pw_name);
        printf("Credencial efectiva: %d, (%s)\n", effective_uid, effective_user->pw_name);
    } else {
        printf("Error al obtener los nombres de usuario.\n");
    }
}


// Función que cambia la credencial efectiva del proceso
void cmd_setuid(char *tr[]) {
    uid_t new_uid;
    struct passwd *pwd;
    char *endptr;

    // Verificar si tenemos al menos un argumento
    if (tr[1] == NULL) {
        fprintf(stderr, "Uso: setuid [-l] <uid|username>\n");
        return;
    }

    // Si el primer argumento es "-l", buscar el nombre de usuario
    if (strcmp(tr[1], "-l") == 0) {
        if (tr[2] == NULL) {
            fprintf(stderr, "Uso: setuid -l <username>\n");
            return;
        }
        // Buscar el nombre de usuario en la base de datos
        pwd = getpwnam(tr[2]);
        if (pwd == NULL) {
            fprintf(stderr, "setuid: Usuario '%s' no encontrado\n", tr[2]);
            return;
        }
        new_uid = pwd->pw_uid;  // UID asociado al nombre de usuario
    
    // Si no es "-l", tratar el argumento como un UID numérico
    } else {
        new_uid = (uid_t)strtol(tr[1], &endptr, 10);

        // Verificar que la conversión fue exitosa y el argumento es un UID válido (no puede ser menor que 0)
        if (*endptr != '\0' || new_uid < 0) {
            fprintf(stderr, "setuid: UID inválido '%s'\n", tr[1]);
            return;
        }
    }

    // Intentar cambiar el UID efectivo
    if (seteuid(new_uid) == -1) {
        perror("setuid: Error al cambiar el UID");
        return;
    }

    // Verificar el cambio y mostrar el nuevo UID efectivo
    pwd = getpwuid(geteuid());  // Obtener el nombre del usuario del UID efectivo
    if (pwd != NULL) {
        printf("UID efectivo cambiado a: %d (%s)\n", geteuid(), pwd->pw_name);
    } else {
        printf("UID efectivo cambiado a: %d\n", geteuid());
    }
}


// Función que muestra el valor y la dirección de las variables de entorno
void cmd_showvar(char *tr[], char *envp[]) {
    if (tr[1] == NULL) {
        // Si no se pasan argumentos, se muestran todas las variables de entorno
        for (int i = 0; envp[i] != NULL; i++) {
            printf("%p->main arg3[%d]=(%p) %s\n", (void*)&envp[i], i, (void*)envp[i], envp[i]);
        }
    } else {
        // Para cada argumento proporcionado, buscar en los tres casos: arg3, environ y getenv
        int multiple_vars = 0; // Variable para verificar si hay más de un argumento
        for (int i = 1; tr[i] != NULL; i++) {
            if (i > 1) {
                multiple_vars = 1; // Se ha pasado más de un argumento
            }

            // Imprimir espacio si es necesario
            if (multiple_vars) {
                printf("\n");
            }

            // Caso 1: Buscar en arg3[] (variables pasadas al programa)
            for (int j = 0; envp[j] != NULL; j++) {
                if (strncmp(envp[j], tr[i], strlen(tr[i])) == 0 && envp[j][strlen(tr[i])] == '=') {
                    char *value = strchr(envp[j], '=') + 1;
                    printf("Con arg3 main %s=%s(%p) @%p\n", tr[i], value, (void*)value, (void*)&envp[j]);
                    break;
                }
            }

            // Caso 2: Buscar en environ[] (variable global de entorno)
            for (int j = 0; environ[j] != NULL; j++) {
                if (strncmp(environ[j], tr[i], strlen(tr[i])) == 0 && environ[j][strlen(tr[i])] == '=') {
                    char *value = strchr(environ[j], '=') + 1;
                    printf("  Con environ %s=%s(%p) @%p\n", tr[i], value, (void*)value, (void*)&environ[j]);
                    break;
                }
            }

            // Caso 3: Buscar con getenv()
            char *env_value = getenv(tr[i]);
            if (env_value != NULL) {
                printf("   Con getenv %s=%s(%p)\n", tr[i], env_value, (void*)env_value);
            } else {
                printf("   Con getenv: Variable no encontrada\n");
            }
        }
    }
}


// Función que cambia el valor de una variable de entorno
void cmd_changevar(char *tr[], char *envp[]) {
    // Verifica que los argumentos sean válidos
    if (tr[1] == NULL || tr[2] == NULL || tr[3] == NULL) {
        printf("Uso: changevar [-a|-e|-p] var valor\n");
        return;
    }

    char *modo = tr[1];  // El primer argumento después del comando es el modo
    char *var = tr[2];   // El segundo argumento es el nombre de la variable
    char *valor = tr[3]; // El tercer argumento es el valor para la variable

    // Modo -a: Modifica o muestra un error si la variable no existe en envp
    if (strcmp(modo, "-a") == 0) {
        int found = 0;
        for (int i = 0; envp[i] != NULL; i++) {
            if (strncmp(envp[i], var, strlen(var)) == 0 && envp[i][strlen(var)] == '=') {
                // Modificar la variable en envp
                snprintf(envp[i], strlen(envp[i]) + 1, "%s=%s", var, valor);
                printf("Variable '%s' cambiada a '%s' con envp\n", var, valor);
                found = 1;
                break;
            }
        }
        if (!found) {
            printf("Error: La variable '%s' no existe en envp y no se puede crear con -a\n", var);
        }
    } 
    // Modo -e: Modifica o muestra un error si la variable no existe en environ
    else if (strcmp(modo, "-e") == 0) {
        int found = 0;
        for (int i = 0; environ[i] != NULL; i++) {
            if (strncmp(environ[i], var, strlen(var)) == 0 && environ[i][strlen(var)] == '=') {
                // Modificar la variable en environ
                snprintf(environ[i], strlen(environ[i]) + 1, "%s=%s", var, valor);
                printf("Variable '%s' cambiada a '%s' con environ\n", var, valor);
                found = 1;
                break;
            }
        }
        if (!found) {
            printf("Error: La variable '%s' no existe en environ y no se puede crear con -e\n", var);
        }
    } 
    // Modo -p: Permite crear o modificar la variable en el entorno global usando putenv
    else if (strcmp(modo, "-p") == 0) {
        char *aux = malloc(strlen(var) + strlen(valor) + 2); // Reserva memoria para crear una nueva variable con su valor: "var=valor"
        if (aux == NULL) {
            perror("malloc");
            return;
        }
        sprintf(aux, "%s=%s", var, valor);
        if (putenv(aux) != 0) {
            perror("putenv");
            free(aux);  // Libera memoria si putenv falla
            return;
        }
        // No liberar aux porque putenv toma posesión de la memoria
        printf("Variable '%s' creada o cambiada a '%s' mediante putenv\n", var, valor);
    } 
    // Si el modo no es válido, muestra un mensaje de error
    else {
        printf("Opción no válida. Usa: -a, -e o -p\n");
    }
}


// Función que sustituye una variable de entorno por otra con un nuevo valor
void cmd_subsvar(char *tr[], char *envp[]) {
    // Verifica si faltan parámetros
    if (tr[1] == NULL || tr[2] == NULL || tr[3] == NULL || tr[4] == NULL) {  //t
        printf("Uso: subsvar [-a|-e] var1 var2 valor\n");
        return;
    }

    char *modo = tr[1], *var1 = tr[2], *var2 = tr[3], *valor = tr[4];
    char **env = NULL;

    // Determina el entorno a usar (local(-a) o global(-e)) -> -a: envp, -e: environ
    if (strcmp(modo, "-a") == 0) {
        env = envp;
    } else if (strcmp(modo, "-e") == 0) {
        env = environ;
    } else {
        printf("Modo no válido. Usa -a (local) o -e (global).\n");
        return;
    }

    // Busca la variable var1 en el entorno
    int pos1 = BuscarVariable(var1, env);
    if (pos1 == -1) {
        printf("No existe la variable %s\n", var1);
        return;
    }

    // Verifica si var2 ya existe en el entorno
    int pos2 = BuscarVariable(var2, env);
    if (pos2 != -1) {
        printf("La variable %s ya existe\n", var2);
        return;
    }

    // Crea la nueva variable "var2=valor"
    char *nueva_var = malloc(strlen(var2) + strlen(valor) + 2);  // +2 para '=' y '\0'
    if (nueva_var == NULL) {
        perror("Error al asignar memoria");
        return;
    }

    // Asigna la nueva variable sin liberar la memoria previamente asignada a var1
    sprintf(nueva_var, "%s=%s", var2, valor);

    // Modifica el valor en el entorno (sin usar 'free' directamente sobre 'env')
    env[pos1] = nueva_var;

    // Indica qué entorno se utilizó para realizar el cambio
    const char *entorno_usado = (env == envp) ? "local (envp)" : "global (environ)";

    // Muestra el cambio realizado con el entorno utilizado
    printf("Variable '%s' sustituida por '%s=%s' con %s\n", var1, var2, valor, entorno_usado);
}


// Función que muestra el entorno del proceso
void cmd_environ(char *tr[], char *envp[]) {
    extern char **environ;

    // Caso general: Sin argumentos, usa envp (tercer argumento del main) igual que showvar
    if (tr[1] == NULL) {
        for (int i = 0; envp[i] != NULL; i++) {
            printf("0x%p->main arg3[%d]=(%p) %s\n", 
                   (void*)&envp[i], i, (void*)envp[i], envp[i]);
        }

    // Modo: -environ, usa la variable global environ para mostrar el entorno
    } else if (strcmp(tr[1], "-environ") == 0) {
        for (int i = 0; environ[i] != NULL; i++) {
            printf("0x%p->environ[%d]=(%p) %s\n", 
                   (void*)&environ[i], i, (void*)environ[i], environ[i]);
        }

    // Modo:  -addr, muestra las direcciones de los punteros de envp y environ
    } else if (strcmp(tr[1], "-addr") == 0) {
        printf("environ:   %p (almacenado en %p)\n", (void*)environ, (void*)&environ);
        printf("main arg3: %p (almacenado en %p)\n", (void*)envp, (void*)&envp);
    } else {
        printf("Uso: environ [-environ|-addr]\n");
    }
}


// Función que realiza una llamada al sistema fork y muestra el PID del proceso hijo
void cmd_fork (char *tr[]){
	pid_t pid;
	
    // Caso en el que estamos en el proceso hijo
	if ((pid=fork())==0){     // Realizar la llamada al sistema fork para crear un proceso hijo
        Remove_backgroundProcess(&bgList, getpid());	// eliminar el proceso hijo de la lista de procesos en segundo plano	
		printf ("Ejecutando proceso %d\n", getpid());   
	}

    // Caso en el que estamos en el proceso padre
	else if (pid!=-1)
		waitpid (pid,NULL,0);   // el proceso padre espera a que el proceso hijo termine
}


// Función que muestra o modifica la lista de búsqueda (la lista de directorios donde se encuentra el shell busca ejecutables)
void cmd_search(char *tr[]) {
    // Si no se pasan argumentos, mostramos la lista de búsqueda
    if(tr[1] == NULL){ 
        mostrarListaBusqueda(&listaBusqueda);
        return;
    }

    // Argumento -add dir: añadir un directorio a la lista de búsqueda
    if (strcmp(tr[1], "-add") == 0) {
        if(tr[2] == NULL){
            printf("Uso: search -add dir\n");
            return;
        }
        agregarDirectorio(&listaBusqueda, tr[2]);       

    // Argumento -del dir: eliminar un directorio de la lista de búsqueda
    } else if (strcmp(tr[1], "-del") == 0) {
        if (tr[2] == NULL) {
            printf("Uso: search -del <directorio>\n");
            return;
        }
        if (eliminarDirectorio(&listaBusqueda, tr[2])) {
            printf("Directorio '%s' eliminado de la lista de búsqueda\n", tr[2]);
        } else {
            printf("El directorio '%s' no se encontró en la lista de búsqueda\n", tr[2]);
        }

    // Argumento -clear: limpiar la lista de búsqueda
    } else if (strcmp(tr[1], "-clear") == 0) {
        limpiarListaBusqueda(&listaBusqueda);
        printf("Lista de búsqueda limpiada\n");
    
    // Argumento -path: importar los directorios del PATH a la lista de búsqueda
    } else if (strcmp(tr[1], "-path") == 0) {
        importarPATH(&listaBusqueda);
        printf("Directorios del PATH importados a la lista de búsqueda\n");

    } else {
        printf("Uso: search [-add|-del|-clear|-path]\n");
    }
}


// Función que ejecuta, sin crear un nuevo proceso, el programa descrito por progspec
void cmd_exec(char *tr[]) {
    char *env_vars[100];        // Array para almacenar las variables de entorno
    int env_count = 0;          // Contador de variables de entorno
    char **args;                // Argumentos del programa
    int i;

    // Procesar variables de entorno
    for (i = 1; tr[i] != NULL && strchr(tr[i], '=') != NULL; i++) {
        env_vars[env_count++] = tr[i];  // Variables en formato "VAR=valor"
    }
    args = &tr[i]; // El resto son los argumentos del programa

    if (args[0] == NULL) {
        fprintf(stderr, "Error: No se especificó un programa para ejecutar\n");
        return;
    }

    // Crear un nuevo entorno si se especificaron variables de entorno
    char **new_environ = NULL;
    if (env_count > 0) {
        new_environ = malloc((env_count + 1) * sizeof(char *));
        if (new_environ == NULL) {
            perror("Error al asignar memoria para el nuevo entorno");
            return;
        }
        // Copiar las variables de entorno a la nueva lista
        for (i = 0; i < env_count; i++) {
            new_environ[i] = getenv(env_vars[i]);
        }
        new_environ[env_count] = NULL;  // Terminar la lista de entorno
    }

    // Intentar ejecutar el programa con el nuevo entorno, si se proporcionó uno
    if (Execpve(&listaBusqueda, args, new_environ, NULL) == -1) {
        // Si el error es "No such file or directory", mostramos el error adecuado
        if (errno == ENOENT) {
            fprintf(stderr, "Error: No such file or directory: '%s'\n", args[0]);
        } else {
            // Si hay otro tipo de error, lo mostramos como antes
            fprintf(stderr, "Error al ejecutar '%s': %s\n", args[0], strerror(errno));
        }
    }

    free(new_environ); // Liberar memoria asignada para el entorno
}


// Función que ejecuta un programa con una prioridad específica, sin crear un nuevo proceso
void cmd_execpri(char *tr[]) {
    if (tr[1] == NULL || tr[2] == NULL) {
        fprintf(stderr, "Uso: execpri prioridad programa [argumentos...]\n");
        return;
    }

    int priority = atoi(tr[1]); // Convertir prioridad

    // Intentar ejecutar el programa con la prioridad
    if (Execpve(&listaBusqueda, &tr[2], NULL, &priority) == -1) {
        // Si el error es "No such file or directory", mostramos el error adecuado
        if (errno == ENOENT) {
            fprintf(stderr, "Error: No such file or directory: '%s'\n", tr[2]);
        } else {
            // Si hay otro tipo de error, lo mostramos como antes
            fprintf(stderr, "Error al ejecutar '%s' con prioridad %d: %s\n", tr[2], priority, strerror(errno));
        }
    }
}


// Función que crea un proceso que ejecuta en primer plano el programa descrito por progspec
void cmd_fg(char *tr[]) {
    if (tr[1] == NULL) {
        fprintf(stderr, "Uso: fg programa [argumentos...]\n");
        return;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        // Proceso hijo: Ejecutar el programa
        if (Execpve(&listaBusqueda, &tr[1], NULL, NULL) == -1) {
            fprintf(stderr, "Error al ejecutar '%s': %s\n", tr[1], strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else if (pid > 0) {
        // Proceso padre: Mostrar PID y esperar al hijo
        printf("Ejecutando proceso %d en primer plano\n", pid);
        int status;
        waitpid(pid, &status, 0);

        // Manejo de señales
        if (WIFSIGNALED(status)) {
            printf("Proceso %d terminado por señal: %s\n", pid, NombreSenal(WTERMSIG(status)));
        } else if (WIFEXITED(status)) {
            printf("Proceso %d terminado con código: %d\n", pid, WEXITSTATUS(status));
        }
    } else {
        perror("Error en fork");
    }
}


// Función que crea un proceso que ejecuta en primer plano el programa descrito por progspec con una prioridad específica
void cmd_fgpri(char *tr[]) {
    if (tr[1] == NULL || tr[2] == NULL) {
        fprintf(stderr, "Uso: fgpri prioridad programa [argumentos...]\n");
        return;
    }

    int priority = atoi(tr[1]); // Convertir prioridad
    pid_t pid = fork();
    if (pid == 0) {
        // Proceso hijo: Ejecutar el programa con prioridad
        if (Execpve(&listaBusqueda, &tr[2], NULL, &priority) == -1) {
            fprintf(stderr, "Error al ejecutar '%s' con prioridad %d: %s\n", tr[2], priority, strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else if (pid > 0) {
        // Proceso padre: Mostrar PID y prioridad y esperar al hijo
        printf("Ejecutando proceso %d con prioridad: %d en primer plano\n", pid,priority);
        int status;
        waitpid(pid, &status, 0);

        // Manejo de señales
        if (WIFSIGNALED(status)) {
            printf("Proceso terminado por señal: %s\n", NombreSenal(WTERMSIG(status)));
        } else if (WIFEXITED(status)) {
            printf("Proceso terminado con código: %d\n", WEXITSTATUS(status));
        }
    } else {
        perror("Error en fork");
    }
}


// Función que crea un proceso que ejecuta en segundo plano el programa descrito por progspec
void cmd_back(char *tr[]) {
    if (tr[1] == NULL) {
        fprintf(stderr, "Uso: back programa [argumentos...]\n");
        return;
    }

    char *executable = Ejecutable(&listaBusqueda, tr[1]);
    if (executable == NULL) {
        fprintf(stderr, "No se encontró el comando: %s\n", tr[1]);
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Proceso hijo. El padre no espera por él (no hay waitpid)
        setpgid(0, 0);  // Establece un nuevo grupo de procesos para que sea independiente del shell
        if (Execpve(&listaBusqueda, &tr[1], NULL, NULL) == -1) {
            perror("Error al ejecutar");
            exit(EXIT_FAILURE);
        }
    } else if (pid > 0) {
        // Proceso padre
        int default_priority = getpriority(PRIO_PROCESS, pid);
        printf("[%d] Ejecutando %s en segundo plano\n", pid, tr[1]);
        Add_backgroundProcess(&bgList, pid, tr[1], default_priority);
    } else {
        perror("Error en fork");
    }
}


// Función que crea un proceso que ejecuta en segundo plano el programa descrito por progspec con una prioridad específica
void cmd_backpri(char *tr[]) {
    if (tr[1] == NULL || tr[2] == NULL) {
        fprintf(stderr, "Uso: backpri prioridad programa [argumentos...]\n");
        return;
    }

    int priority = atoi(tr[1]); // Convertir prioridad
    pid_t pid = fork();
    if (pid == 0) {
        //  Proceso hijo. El padre no espera por él (no hay waitpid)
        setpgid(0, 0);  // Establece un nuevo grupo de procesos para que sea independiente del shell
        if (Execpve(&listaBusqueda, &tr[2], NULL, &priority) == -1) {
            fprintf(stderr, "Error al ejecutar '%s' en segundo plano con prioridad %d: %s\n", tr[2], priority, strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else if (pid > 0) {
        // Proceso padre
        printf("[%d] Ejecutando %s en segundo plano con prioridad: %d\n", pid, tr[2], priority);
        Add_backgroundProcess(&bgList, pid, tr[2], priority);
    } else {
        perror("Error en fork");
    }
}


// Función que lista los procesos en segundo plano
void cmd_listjobs(char *tr[]) {
    // Actualizar el estado de los procesos en segundo plano
    update_background_processes();
    if (bgList.next == NULL) {
        printf("No hay procesos en segundo plano\n");
    } else {
        Imprimir_backgroundProcesses(&bgList);
    }
}


// Función que elimina procesos en segundo plano de la lista
void cmd_deljobs(char *tr[]) {
    if (tr[1] == NULL) {
        fprintf(stderr, "Uso: deljobs [-term|-sig]\n");
        return;
    }

    // Actualizar el estado de los procesos en segundo plano
    update_background_processes();

    BackgroundProcess *current = &bgList;
    BackgroundProcess *next;
    int removed = 0;

    while (current->next != NULL) {
        next = current->next;
        if ((strcmp(tr[1], "-term") == 0 && next->status == FINISHED) ||
            (strcmp(tr[1], "-sig") == 0 && next->status == SIGNALED)) {
            Remove_backgroundProcess(&bgList, next->pid);
            removed++;
        } else {
            current = next;
        }
    }

    printf("Procesos eliminados: %d\n", removed);
}


// Función auxiliar para actualizar el estado de los procesos en segundo plano
void update_background_processes() {
    BackgroundProcess *current = bgList.next;
    int status;
    pid_t pid;

    while (current != NULL) {
        // Usamos WNOHANG para no bloquear el shell si un proceso no ha terminado y WUNTRACED para procesos detenidos
        pid = waitpid(current->pid, &status, WNOHANG | WUNTRACED);   
        if (pid > 0) {
            if (WIFEXITED(status)) {
                current->status = FINISHED;
                current->return_value = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                current->status = SIGNALED;
                current->return_value = WTERMSIG(status);
            } else if (WIFSTOPPED(status)) {
                current->status = STOPPED;
                current->return_value = WSTOPSIG(status);
            }
        } else if (pid == 0) {
            // El proceso sigue en ejecución
            current->status = ACTIVE;
        }
        current = current->next;
    }
}


// Función para ejecutar comandos externos
void cmd_execute_external_program(char *tr[], char *envp[]) {
    char *executable = Ejecutable(&listaBusqueda, tr[0]);
    if (executable == NULL) {
        fprintf(stderr, "No se encontró el comando: %s\n", tr[0]);
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Proceso hijo
        execv(executable, tr);
        perror("No ejecutado:");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Proceso padre
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork");
    }
}