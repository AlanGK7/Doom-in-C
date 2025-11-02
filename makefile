# Nombre del ejecutable final
TARGET = simulacion

# Compilador
CC = gcc

# Flags de compilación:
# -g       : Habilita la información de depuración
# -Wall    : Muestra todas las advertencias (Warnings)
# -pthread : Esencial para enlazar la librería de Pthreads
CFLAGS = -g -Wall -pthread

# Archivos fuente (.c)
SOURCES = main.c grid.c hero.c monster.c parser.c

# Archivos objeto (.o) - se generan a partir de los .c
OBJECTS = $(SOURCES:.c=.o)

# --- Reglas ---

# Regla por defecto: compila todo
all: $(TARGET)

# Regla para crear el ejecutable final
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

# Regla para compilar cada .c en un .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regla para limpiar (borrar archivos compilados)
clean:
	rm -f $(TARGET) $(OBJECTS)

# Regla para compilar y ejecutar
run: all
	./$(TARGET)