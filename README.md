# PRÁCTICAS DE SISTEMAS OPERATIVOS 2024

Este repositorio contiene las prácticas P0, P1, P2 y P3 de **Sistemas Operativos UDC**.

## Estructura de las prácticas + código

- **P0**: Inicio del shell con comandos básicos
- **P1**: Comandos sobre ficheros y directorios
- **P2**: Comandos sobre memoria
- **P3**: Comandos sobre procesos

## La distribución de los ficheros es esta:

### Iteraciones de la práctica desde la P0 a P3:
- **P3**: Funciones de los comandos

---

### 'P0 y P1':
- **utils**: Archivos para funciones auxiliares y funciones que nos proporcionan ellos.
- **lista**: Primera lista que empleamos. Lista para hacer la función del histórico, que almacena todos los comandos que vayamos escribiendo en nuestro shell.
- **lista_ficheros**: Lista de ficheros para ver los descriptores de texto abiertos en el shell.

---

### 'P2':
- **lista_memoria**: Cuatro listas en un archivo para trabajar con la memoria. Una lista general que incluye a las otras tres: una para los `malloc`, otra para `mmap` (ficheros mapeados) y otra para `shared` (para la memoria compartida).

---

### 'P3':
- **lista_busqueda**: Una lista para insertar y eliminar directorios que se utilizará para buscar ejecutables.
- **lista_procesos_bg**: Lista de procesos en segundo plano. Almacena todos los procesos que se ejecutan en segundo plano.
