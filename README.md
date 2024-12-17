# PŔACTICAS DE SISTEMAS OPERATIVOS 2024
Este repositorio contiene las prácticas P0,P1,P2 y P3 de Sistemas Operativos UDC.

## Estructura de las prácticas + código
p0: inicio del shell con comandos básicos
p1: comandos sobre ficheros y directorios
p2: comandos sobre memoria
p3: comandos sobre procesos

## La distribución de los ficheros es esta:
- Iteraciones de la práctica desde la p0 a p3:
p3: funciones de los comandos

- 'P0 y P1':
utils: archivos para funciones auxialiares y funciones que nos proporcionan ellos
lista: primera lista que empleamos. Lista para hacer la función del histórico, que almacena
       todos los comandos que vayamos escribiendo en nuestro shell
lista_ficheros: lista de ficheros para ver los descriptores de texto abiertos en el shell

- 'P2':
lista_memoria: 4 listas en un archivo para trabajar con la memoria. Una lista general que 
              incluye a las otras 3: una para los malloc, otra para mmap (ficheros mapeados)
              y otra para shared (para la memoria compartida)

- 'P3':
lista_busqueda: una lista para insertar y eliminar directorios que se utilizará para buscar ejecutables
lista_procesos_bg: lista de procesos background. Almacena todos los procesos en segundo plano

