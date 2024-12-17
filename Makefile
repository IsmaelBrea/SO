# Variables
CC = gcc                    # Compilador a usar
CFLAGS = -g -Wall -m64      # Opciones de compilación (g para depuración, Wall para mostrar todas las advertencias) y -m64 le indica al compilador que genere código para una arquitectura de 64 bits
OBJ = p3.o lista.o lista_ficheros.o utils.o lista_memoria.o lista_busqueda.o lista_procesos_bg.o # Archivos objeto a crear
TARGET = p3                # Nombre del ejecutable

# Regla por defecto que compila y ejecuta
all: $(TARGET)              # Compila el ejecutable
	./$(TARGET)              # Ejecuta el programa

# Regla para crear el ejecutable
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@       # Enlaza los archivos objeto para crear el ejecutable

# Regla para compilar p1.c
p1.o: p3.c lista.h lista_ficheros.h utils.h lista_memoria.h
	$(CC) $(CFLAGS) -c $<    # Compila p1.c a p1.o

# Regla para compilar lista.c
lista.o: lista.c lista.h
	$(CC) $(CFLAGS) -c $<    # Compila lista.c a lista.o

# Regla para compilar lista_ficheros.c
lista_ficheros.o: lista_ficheros.c lista_ficheros.h
	$(CC) $(CFLAGS) -c $<    # Compila lista_ficheros.c a lista_ficheros.o

# Regla para compilar utils.c
utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c $<    # Compila utils.c a utils.o

# Regla para compilar lista_memoria.c
lista_memoria.o: lista_memoria.c lista_memoria.h
	$(CC) $(CFLAGS) -c $<    # Compila lista_memoria.c a lista_memoria.o

# Regla para compilar lista_busqueda.c
lista_busqueda.o: lista_busqueda.c lista_busqueda.h
	$(CC) $(CFLAGS) -c $<    # Compila lista_busqueda.c a lista_busqueda.o

lista_procesos_bg.o: lista_procesos_bg.c lista_procesos_bg.h
	$(CC) $(CFLAGS) -c $<    # Compila lista_procesos_bg.c a lista_procesos_bg.o

# Regla para ejecutar Valgrind en el programa
valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

# Regla para ejecutar el programa con sudo (con privilegios de superusuario)
sudo: $(TARGET)
	sudo ./$(TARGET)          # Ejecuta el programa con privilegios de superusuario

# Regla para limpiar los archivos generados
clean:
	rm -f $(OBJ) $(TARGET)   # Elimina los archivos objeto y el ejecutable

.PHONY: all clean valgrind  # Indica que "all", "clean" y "valgrind" no son archivos
