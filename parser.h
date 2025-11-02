#ifndef PARSER_H
#define PARSER_H

#include "hero.h"
#include "monster.h"

// esta estructura agrupara toda la configuración leida del archivo
typedef struct {
    int grid_ancho;
    int grid_alto;
    
    Heroe* heroes_iniciales; 
    int num_heroes;           

    Monstruo* monstruos_iniciales; // arreglo dinamico de monstruos
    int num_monstruos;
} Configuracion;

int parsear_config(const char* filename, Configuracion* config);

#endif