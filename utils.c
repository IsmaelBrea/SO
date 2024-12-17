/*
 * TITLE: P3 SISTEMAS OPERATIVOS
 * SUBTITLE: Practica 1
 * AUTHOR 1: ISMAEL BREA ARIAS          LOGIN 1: ismael.brea
 * AUTHOR 2: BORJA CASTELEIRO GOTI      LOGIN 2: borja.casteleiro
 * GROUP: 4.1
 */
#include "utils.h"

extern char **environ;  // Variables externa

// ====== AUXILIARES P0 ====== //
// Función para verificar si un string es un número
int isNumber(char *str) {
    int i = 0;
    while (str[i] != '\0') {
        if (!isdigit(str[i])) {
            return 0;
        }
        i++;
    }
    return 1;
}


// ====== AUXILIARES P1 ====== //
// Función para determinar el tipo de archivo (normal, directorio, etc.)
char LetraTF(mode_t m) {
    // & - AND bit a bit e indica solo la parte relevante para el tipo de archivo
    switch (m & S_IFMT) { // Verifica el tipo de archivo
        case S_IFSOCK: return 's'; // Socket
        case S_IFLNK: return 'l';  // Enlace simbólico
        case S_IFREG: return '-';   // Archivo normal
        case S_IFBLK: return 'b';   // Dispositivo de bloque
        case S_IFDIR: return 'd';   // Directorio
        case S_IFCHR: return 'c';   // Dispositivo de carácter
        case S_IFIFO: return 'p';    // FIFO (pipe)
        default: return '?';        // Tipo desconocido
    }
}

// Función para convertir el modo (permisos) a una cadena legible
char *ConvierteModo(mode_t m, char *permisos) {
    strcpy(permisos, "---------- "); // Inicializa la cadena de permisos
    permisos[0] = LetraTF(m); // Determina el tipo de archivo y lo asigna a la cadena

    // Verifica los permisos del propietario
    if (m & S_IRUSR) permisos[1] = 'r'; // Permiso de lectura
    if (m & S_IWUSR) permisos[2] = 'w'; // Permiso de escritura
    if (m & S_IXUSR) permisos[3] = 'x'; // Permiso de ejecución

    // Verifica los permisos del grupo
    if (m & S_IRGRP) permisos[4] = 'r'; // Permiso de lectura
    if (m & S_IWGRP) permisos[5] = 'w'; // Permiso de escritura
    if (m & S_IXGRP) permisos[6] = 'x'; // Permiso de ejecución

    // Verifica los permisos para otros
    if (m & S_IROTH) permisos[7] = 'r'; // Permiso de lectura
    if (m & S_IWOTH) permisos[8] = 'w'; // Permiso de escritura
    if (m & S_IXOTH) permisos[9] = 'x'; // Permiso de ejecución

    // Verifica los bits especiales
    if (m & S_ISUID) permisos[3] = 's'; // Setuid
    if (m & S_ISGID) permisos[6] = 's'; // Setgid
    if (m & S_ISVTX) permisos[9] = 't'; // Sticky bit

    return permisos; // Retorna la cadena de permisos
}

// Función para imprimir información de archivos y directorios
void printFileInfo(char *filename, int longFormat, int linkInfo, int accessTime) {
    struct stat file_info;
    char time_buff[256];
    struct tm *time_info;
    char *basename;

    // Obtener el nombre base del archivo
    basename = strrchr(filename, '/');
    basename = basename ? basename + 1 : filename;

    if (lstat(filename, &file_info) == -1) {
        printf("Error al obtener información de %s: %s\n", basename, strerror(errno));
        return;
    }

    // Manejo de enlaces simbólicos (-link)
    if (S_ISLNK(file_info.st_mode) && linkInfo) {
        char link_buff[1024];
        ssize_t link_size = readlink(filename, link_buff, sizeof(link_buff) - 1);
        if (link_size != -1) {
            link_buff[link_size] = '\0';
            printf("%s -> %s\n", basename, link_buff);
            printf("%8ld %s\n", (long)file_info.st_size, basename);
        } else {
            printf("Error al leer el enlace simbólico %s: %s\n", basename, strerror(errno));
        }
        return;
    }

    // Formato largo de directorio o archivo regular (-long)
    if (longFormat) {
        struct passwd *pw = getpwuid(file_info.st_uid);
        struct group *gr = getgrgid(file_info.st_gid);
        time_info = accessTime ? localtime(&file_info.st_atime) : localtime(&file_info.st_mtime);
        strftime(time_buff, sizeof(time_buff), "%Y/%m/%d-%H:%M", time_info);
        
        char permisos[12];
        ConvierteModo(file_info.st_mode, permisos);

        printf("%s %3ld (%8ld) %8s %8s %s %8ld %s\n",
               time_buff,
               file_info.st_nlink,
               (long)file_info.st_ino,
               pw ? pw->pw_name : "unknown",
               gr ? gr->gr_name : "unknown",
               permisos,
               (long)file_info.st_size,
               basename);
    } else {
        // Formato corto (-acc)
        if (accessTime) {
            time_info = localtime(&file_info.st_atime);
            strftime(time_buff, sizeof(time_buff), "%Y/%m/%d-%H:%M", time_info);
            printf("%s %8ld %s\n", time_buff, (long)file_info.st_size, basename);
        } else {
            printf("%8ld %s\n", (long)file_info.st_size, basename);
        }
    }
}


// ====== AUXILIARES P2 ====== //
// Mapea un archivo en memoria con los permisos especificados
void * MapearFichero(char *fichero, int protection, int *fd) {
    int map = MAP_PRIVATE, modo = O_RDONLY; // Configuración inicial: mapeo privado y modo de apertura de solo lectura
    struct stat s;  // Estructura para almacenar información sobre el archivo
    void *p;  // Puntero para almacenar la dirección de la memoria mapeada

    // Si se solicita permiso de escritura, el archivo debe abrirse en modo lectura/escritura.
    if (protection & PROT_WRITE)
        modo = O_RDWR;

    // Obtener información del archivo con `stat` y abrir el archivo en el modo correspondiente
    if (stat(fichero, &s) == -1 || (*fd = open(fichero, modo)) == -1) {
        perror("Error al abrir el archivo"); // Mensaje de error detallado en caso de fallo
        return NULL;  // Retorna `NULL` si no puede obtener la información o abrir el archivo
    }

    // Se usa mmap para mapear el archivo en memoria. Esto asignará una región de memoria que contendrá los datos del archivo
    if ((p = mmap(NULL, s.st_size, protection, map, *fd, 0)) == MAP_FAILED) {
        perror("Error en mmap");
        close(*fd);
        return NULL;
    }

    return p;
}

// Función que maneja la asignación de memoria mapeada, incluyendo la configuración de permisos
void do_AllocateMmap(char *arg[], MemoryManager *manager) {  
    char *perm;
    void *p;
    int protection = 0;
    int fd;

    if (arg[0] == NULL) {
        ImprimirListaMmap(&manager->mmapList);  // Mostrar la lista de mmap
        return;
    }

    // Analizar permisos
    if ((perm = arg[1]) != NULL && strlen(perm) < 4) {
        if (strchr(perm, 'r') != NULL) protection |= PROT_READ;
        if (strchr(perm, 'w') != NULL) protection |= PROT_WRITE;
        if (strchr(perm, 'x') != NULL) protection |= PROT_EXEC;
    }

    // Mapear el archivo
    p = MapearFichero(arg[0], protection, &fd);

    // Verificar si hubo error en el mapeo
    if (p == NULL) {
        perror("Imposible mapear fichero");
    } else {
        printf("fichero %s mapeado en %p\n", arg[0], p);

        // Añadir el bloque a la lista de memoria
        InsertMmapMemory(manager, p, lseek(fd, 0, SEEK_END), fd, arg[0]);
        // Asegurarse de que el offset se establezca a 0 después de mapear. El cursor que lee y escribe se sitúa al inicio del archivo
        lseek(fd, 0, SEEK_SET);

        // Añadir a la lista de ficheros abiertos
        int file_perm = O_RDWR;  // Por defecto, asumimos lectura y escritura
        char nombre[256];
        snprintf(nombre, sizeof(nombre), "Mapeo de %s", arg[0]);  // se añade esto en el texto de la lista de descriptores abiertos
          // Añadir a la lista de ficheros
        anadirAFicherosAbiertos(fd, file_perm, nombre, &ficheros);
    }
}

// Función que crea o accede a un segmento de memoria compartida (shmget) con la clave dada (allocate -shared)
void * ObtenerMemoriaShmget (key_t clave, size_t tam) {
    void * p;
    int aux, id, flags = 0777;
    struct shmid_ds s;

    // Si el tamaño es diferente de 0, establecer las banderas para crear el segmento
    if (tam)     /*tam distinto de 0 indica crear */
        flags = flags | IPC_CREAT | IPC_EXCL;

    // Si la clave es IPC_PRIVATE, no se permite su uso
    if (clave == IPC_PRIVATE)  /*no nos vale, si la clave es PRIVATE devolvemos un error*/   
        {errno = EINVAL; return NULL;}

    // Intentar obtener el segmento de memoria con shmget
    if ((id = shmget(clave, tam, flags)) == -1)
        return NULL;

    // Intentar adjuntar el segmento de memoria al proceso
    if ((p = shmat(id, NULL, 0)) == (void*) -1) {
        aux = errno;
        if (tam)
             shmctl(id, IPC_RMID, NULL);  // Eliminar el segmento si se creó y hubo error
        errno = aux;
        return NULL;
    }

    // Obtener información sobre el segmento de memoria (para validación)
    if (shmctl(id, IPC_STAT, &s) == -1) {
        aux = errno;
        shmdt(p);  // Desmontar el segmento de memoria si hay un error
        if (tam)
            shmctl(id, IPC_RMID, NULL);  // Eliminar el segmento si se creó y hubo error
        errno = aux;
        return NULL;
    }
    
    // Guardar el segmento de memoria compartida en la lista
    InsertSharedMemory(&memoria, p, s.shm_segsz, clave, NULL);
    return p;
}

// Función para crear y asignar un segmento de memoria compartida con clave y tamaño especificados (allocate -createshared)
void do_AllocateCreateshared (char *tr[]){
   key_t cl;
   size_t tam;
   void *p;

   if (tr[0] == NULL || tr[1] == NULL) {    // tr[0] es la clave y tr[1] es el tamaño
        ImprimirListaShared(&memoria.sharedList);
        return;
   }
  
   cl = (key_t) strtoul(tr[0], NULL, 10);
   tam = (size_t) strtoul(tr[1], NULL, 10);
   if (tam == 0) {
        printf("No se asignan bloques de 0 bytes\n");
        return;
   }
   if ((p = ObtenerMemoriaShmget(cl, tam)) != NULL)
        printf("Asignados %lu bytes en %p\n", (unsigned long) tam, p);
   else
        printf("Imposible asignar memoria compartida clave %lu: %s\n", (unsigned long) cl, strerror(errno));
}

// Función que asigna un segmento de memoria compartida existente utilizando una clave específica  (allocate -shared)
void do_AllocateShared (char *tr[]){
   key_t cl;
   void *p;

   if (tr[0] == NULL) {
        ImprimirListaShared(&memoria.sharedList);
        return;
   }
  
   cl = (key_t) strtoul(tr[0], NULL, 10);

   if ((p = ObtenerMemoriaShmget(cl, 0)) != NULL)
        printf("Asignada memoria compartida de clave %lu en %p\n", (unsigned long) cl, p);
   else
        printf("Imposible asignar memoria compartida clave %lu: %s\n", (unsigned long) cl, strerror(errno));
}
 
// Función para convertir una cadena a un puntero
void *cadtop(char *cadena) {
    return (void *)strtoul(cadena, NULL, 16);  // Asumiendo que convierte una cadena hexadecimal a un puntero con strtoul
}

// Función que usamos en memfill para llenar un bloque de memoria con un byte específico
void LlenarMemoria (void *p, size_t cont, unsigned char byte) {
  unsigned char *arr=(unsigned char *) p;
  size_t i;

  for (i=0; i<cont;i++)
		arr[i]=byte;
}

// Función que usamos en memdump para leer un byte de forma segura
int readByte(void *addr, int *value) {
    if (addr == NULL || value == NULL) {
        return -1;
    }
    
    errno = 0;
    *value = *(volatile unsigned char *)addr;
    return (errno == 0) ? 0 : -1;
}

// Función que muestra el mapa de memoria del proceso actual usando herramientas del sistema 
void Do_pmap (void){   /*sin argumentos*/
   pid_t pid;       /*hace el pmap (o equivalente) del proceso actual*/
   char elpid[32];
   char *argv[4]={"pmap",elpid,NULL};    //pmap mostrará el mapa de memoria del proceso actual
   
   // Crea un proceso hijo con fork
   sprintf (elpid,"%d", (int) getpid());
   if ((pid=fork())==-1){
      perror ("Imposible crear proceso");
      return;
      }

    // Ejecución de comandos específicos en el proceso hijo para diferentes sistemas operativos
   if (pid==0){
      if (execvp(argv[0],argv)==-1)
         perror("cannot execute pmap (linux, solaris)");
         
      argv[0]="procstat"; argv[1]="vm"; argv[2]=elpid; argv[3]=NULL;   
      if (execvp(argv[0],argv)==-1)/*No hay pmap, probamos procstat FreeBSD */
         perror("cannot execute procstat (FreeBSD)");
         
      argv[0]="procmap",argv[1]=elpid;argv[2]=NULL;    
            if (execvp(argv[0],argv)==-1)  /*probamos procmap OpenBSD*/
         perror("cannot execute procmap (OpenBSD)");
         
      argv[0]="vmmap"; argv[1]="-interleave"; argv[2]=elpid;argv[3]=NULL;
      if (execvp(argv[0],argv)==-1) /*probamos vmmap Mac-OS*/
         perror("cannot execute vmmap (Mac-OS)");      
      exit(1);
  }
  waitpid (pid,NULL,0);
}

// Función que usamos en readfile para leer un archivo y almacenarlo en una dirección de memoria proporcionada
ssize_t LeerFichero(char *f, void *p, size_t cont) {
    struct stat s;      // Estructura para almacenar información sobre el archivo (como su tamaño)
    ssize_t n;          // Variable para almacenar el número de bytes leídos
    int df, aux;        // Descriptor de archivo y variable auxiliar para manejar errores

    // Comprueba si el archivo existe (stat) y lo abre en modo de solo lectura (open)
    if (stat(f, &s) == -1 || (df = open(f, O_RDONLY)) == -1) {
        return -1;
    }

    if (cont == -1) {
        cont = s.st_size; // Si no se pasa un tamaño, se lee el archivo completo
    }

    // Lee hasta `cont` bytes desde el archivo abierto al buffer apuntado por `p`
    n = read(df, p, cont); // Intentamos leer el archivo a la dirección 'p' 

    // Si ocurre un error durante la lectura
    if (n == -1) {
        aux = errno;    // Guarda el valor actual de `errno` para no perder el código de error
        close(df);      // Cierra el archivo antes de salir
        errno = aux;    // Restaura el código de error original
        return -1;      // Devuelve -1 indicando error
    }

    close(df); // Cierra el archivo después de leer con éxito
    return n;
}


// ====== AUXILIARES P3 ====== //
// Función que busca una variable en el entorno que se le pasa como parámetro. Devuelve la posición de la variable en el entorno, -1 si no existe
int BuscarVariable (char * var, char *e[]) {   
  int pos=0;
  char aux[MAXVAR];
  
  strcpy (aux,var);
  strcat (aux,"=");
  
  while (e[pos]!=NULL)
    if (!strncmp(e[pos],aux,strlen(aux)))
      return (pos);
    else 
      pos++;
  errno=ENOENT;   /*no hay tal variable*/
  return(-1);
}

// Función que cambia una variable en el entorno que se le pasa como parámetro (lo hace directamente, no usa putenv)
int CambiarVariable(char * var, char * valor, char *e[]) {                                                        
  int pos;
  char *aux;
   
  if ((pos=BuscarVariable(var,e))==-1)
    return(-1);
 
  if ((aux=(char *)malloc(strlen(var)+strlen(valor)+2))==NULL)
	return -1;
  strcpy(aux,var);
  strcat(aux,"=");
  strcat(aux,valor);
  e[pos]=aux;
  return (pos);
}

// Función que busca un ejecutable en los directorios de la lista
char *Ejecutable(ListaBusqueda *lista, char *s) {
    static char path[MAXNAME];
    struct stat st;
    char *p;

     // Si no hay nombre de archivo o lista vacía, devuelve el nombre original
    if (s == NULL || (p = SearchListFirst(lista)) == NULL)
        return s;

    // Si es un pathname absoluto, devuelve el nombre tal cual
    if (s[0] == '/' || !strncmp(s, "./", 2) || !strncmp(s, "../", 3))
        return s;  // Es un pathname absoluto

    // Busca el ejecutable en la lista de directorios
    do {
        snprintf(path, MAXNAME, "%s/%s", p, s);
        if (lstat(path, &st) != -1)
            return path;
    } while ((p = SearchListNext(lista)) != NULL);

    // Si no se encuentra, devuelve el nombre original
    return s;
}

// Función que ejecuta un comando con execv o execve, permitiendo cambiar el entorno y la prioridad
int Execpve(ListaBusqueda *lista, char *tr[], char **NewEnv, int *pprio) {
    char *p;

    // Si el ejecutable no es válido o no se encuentra, devuelve un error
    if (tr[0] == NULL || (p = Ejecutable(lista, tr[0])) == NULL) {
        errno = EFAULT;
        return -1;
    }

    // Si se especifica una nueva prioridad, intenta cambiarla
    if (pprio != NULL && setpriority(PRIO_PROCESS, getpid(), *pprio) == -1) {
        printf("Imposible cambiar prioridad: %s\n", strerror(errno));
        return -1;
    }

    // Ejecuta el programa con el entorno modificado si se especifica uno
    if (NewEnv == NULL) {
        return execv(p, tr);            // Usando el entorno predeterminado
    } else {
        return execve(p, tr, NewEnv);   // Usando el entorno modificado
    }
}

// Estructura para manejar señales
struct SEN {
    char *nombre;
    int senal;
};

static struct SEN sigstrnum[]={   
	{"HUP", SIGHUP},
	{"INT", SIGINT},
	{"QUIT", SIGQUIT},
	{"ILL", SIGILL}, 
	{"TRAP", SIGTRAP},
	{"ABRT", SIGABRT},
	{"IOT", SIGIOT},
	{"BUS", SIGBUS},
	{"FPE", SIGFPE},
	{"KILL", SIGKILL},
	{"USR1", SIGUSR1},
	{"SEGV", SIGSEGV},
	{"USR2", SIGUSR2}, 
	{"PIPE", SIGPIPE},
	{"ALRM", SIGALRM},
	{"TERM", SIGTERM},
	{"CHLD", SIGCHLD},
	{"CONT", SIGCONT},
	{"STOP", SIGSTOP},
	{"TSTP", SIGTSTP}, 
	{"TTIN", SIGTTIN},
	{"TTOU", SIGTTOU},
	{"URG", SIGURG},
	{"XCPU", SIGXCPU},
	{"XFSZ", SIGXFSZ},
	{"VTALRM", SIGVTALRM},
	{"PROF", SIGPROF},
	{"WINCH", SIGWINCH}, 
	{"IO", SIGIO},
	{"SYS", SIGSYS},
/*senales que no hay en todas partes*/
#ifdef SIGPOLL
	{"POLL", SIGPOLL},
#endif
#ifdef SIGPWR
	{"PWR", SIGPWR},
#endif
#ifdef SIGEMT
	{"EMT", SIGEMT},
#endif
#ifdef SIGINFO
	{"INFO", SIGINFO},
#endif
#ifdef SIGSTKFLT
	{"STKFLT", SIGSTKFLT},
#endif
#ifdef SIGCLD
	{"CLD", SIGCLD},
#endif
#ifdef SIGLOST
	{"LOST", SIGLOST},
#endif
#ifdef SIGCANCEL
	{"CANCEL", SIGCANCEL},
#endif
#ifdef SIGTHAW
	{"THAW", SIGTHAW},
#endif
#ifdef SIGFREEZE
	{"FREEZE", SIGFREEZE},
#endif
#ifdef SIGLWP
	{"LWP", SIGLWP},
#endif
#ifdef SIGWAITING
	{"WAITING", SIGWAITING},
#endif
 	{NULL,-1},
	};    /*fin array sigstrnum */

    
// Función que devuelve el número de señal a partir del nombre
int ValorSenal(char * sen){
  int i;
  for (i=0; sigstrnum[i].nombre!=NULL; i++)
  	if (!strcmp(sen, sigstrnum[i].nombre))
		return sigstrnum[i].senal;
  return -1;
}

// Función que devuelve el nombre de la señal a partir de la señal
char *NombreSenal(int sen) {
 int i;
  for (i=0; sigstrnum[i].nombre!=NULL; i++)
  	if (sen==sigstrnum[i].senal)
		return sigstrnum[i].nombre;
 return ("SIGUNKNOWN");
}

