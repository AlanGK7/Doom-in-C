#ifndef HEROE_H
#define HEROE_H

// cada heroe tiene su estructura de cordenadas
typedef struct {
    int x;
    int y;
} Coordenada;

// esta almacena su ruta
typedef struct {
    Coordenada* pasos; // es arreglo dinamico de coordenadas
    int num_pasos;
    int paso_actual;
} Ruta;

// estructura principal del heroe
typedef struct {
    int id;
    int hp; 
    int attack_damage; 
    int attack_range; 
    Coordenada posicion_actual; 
    Ruta ruta; 
    // se podria agregar un estado
} Heroe;

void* logica_heroe(void* arg);

#endif