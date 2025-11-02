#ifndef GRID_H
#define GRID_H

#include <pthread.h>
#include "hero.h"
#include "monster.h"


// cada casilla tiene su propio grid
typedef struct {
    pthread_mutex_t lock_casilla; 

    void* ocupante; // puede ser un heroe o mosntruo
    int tipo_ocupante; // 0=vacio, 1=heroe, 2=monstruo
} Casilla;

typedef struct {
    int ancho;
    int alto;
    Casilla** casillas; 
    Monstruo* monstruos; 
    int num_monstruos;

    Heroe* heroes;
    int num_heroes; // añadir contador
    
    // locks de heroes y monstruos
    pthread_mutex_t* locks_heroes; 
    pthread_mutex_t* locks_monstruos; 

} Grid;

// la firma de la función DEBE cambiar
Grid* inicializar_grid(int ancho, int alto, Heroe* heroes_base, int num_heroes, Monstruo* monstruos_base, int num_monstruos);

void destruir_grid(Grid* grid);
int mover_entidad_a_casilla(Grid* grid, int id_entidad, int tipo, int x_nuevo, int y_nuevo); 

#endif