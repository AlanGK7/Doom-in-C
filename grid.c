#include <stdlib.h>
#include <stdio.h>
#include "grid.h"

// Festa es una funcion auxiliar para encontrar la posición del heroe o monstruo
static Coordenada buscar_posicion(Grid* grid, int id, int tipo) {
    if (tipo == 1) { // es un heroe
        int idx = id - 1; // se asume que la ID 1 es índice 0
        pthread_mutex_lock(&grid->locks_heroes[idx]);
        Coordenada pos = grid->heroes[idx].posicion_actual;
        pthread_mutex_unlock(&grid->locks_heroes[idx]);
        return pos;
    } else { // es un monstruo
        int idx = id - 1; 
        pthread_mutex_lock(&grid->locks_monstruos[idx]);
        Coordenada pos = grid->monstruos[idx].posicion;
        pthread_mutex_unlock(&grid->locks_monstruos[idx]);
        return pos;
    }
}

// inicializa 
Grid* inicializar_grid(int ancho, int alto, Heroe* heroes_base, int num_heroes, Monstruo* monstruos_base, int num_monstruos) {
    Grid* grid = malloc(sizeof(Grid));
    grid->ancho = ancho;
    grid->alto = alto;
    grid->heroes = heroes_base;
    grid->num_heroes = num_heroes;
    grid->monstruos = monstruos_base;
    grid->num_monstruos = num_monstruos;

    // se ajusta las caracteristicas del campo de batalla
    grid->casillas = malloc(sizeof(Casilla*) * alto);
    for (int i = 0; i < alto; i++) {
        grid->casillas[i] = malloc(sizeof(Casilla) * ancho);
        for (int j = 0; j < ancho; j++) {
            pthread_mutex_init(&grid->casillas[i][j].lock_casilla, NULL);
            grid->casillas[i][j].ocupante = NULL;
            grid->casillas[i][j].tipo_ocupante = 0;
        }
    }
    
    // coloca al heroe en el plano
    for (int i = 0; i < num_heroes; i++) {
        int hx = grid->heroes[i].posicion_actual.x;
        int hy = grid->heroes[i].posicion_actual.y;
        grid->casillas[hy][hx].ocupante = &grid->heroes[i];
        grid->casillas[hy][hx].tipo_ocupante = 1; // 1 = Heroe
    }

    // coloca al mosntruo en el plano
    for (int i = 0; i < num_monstruos; i++) {
        int mx = grid->monstruos[i].posicion.x;
        int my = grid->monstruos[i].posicion.y;
        grid->casillas[my][mx].ocupante = &grid->monstruos[i];
        grid->casillas[my][mx].tipo_ocupante = 2; // 2 = Monstruo
    }

    // se inicializa el lock de heroes 
    grid->locks_heroes = malloc(sizeof(pthread_mutex_t) * num_heroes);
    for (int i = 0; i < num_heroes; i++) {
        pthread_mutex_init(&grid->locks_heroes[i], NULL);
    }
    
    // se inicializa el lock de mosntruos
    grid->locks_monstruos = malloc(sizeof(pthread_mutex_t) * num_monstruos);
    for (int i = 0; i < num_monstruos; i++) {
        pthread_mutex_init(&grid->locks_monstruos[i], NULL);
    }
    
    return grid;
}

// se destruye los locks
void destruir_grid(Grid* grid) {
    for (int i = 0; i < grid->alto; i++) {
        for (int j = 0; j < grid->ancho; j++) {
            pthread_mutex_destroy(&grid->casillas[i][j].lock_casilla);
        }
        free(grid->casillas[i]);
    }
    free(grid->casillas);

    // heroes
    for (int i = 0; i < grid->num_heroes; i++) {
        pthread_mutex_destroy(&grid->locks_heroes[i]);
    }
    free(grid->locks_heroes);

    // mosntruos
    for (int i = 0; i < grid->num_monstruos; i++) {
        pthread_mutex_destroy(&grid->locks_monstruos[i]);
    }
    free(grid->locks_monstruos);
    
    free(grid);
}

// mueve el monstruo o el heroe a una nueva casilla
int mover_entidad_a_casilla(Grid* grid, int id_entidad, int tipo, int x_nuevo, int y_nuevo) {
    
    if (x_nuevo < 0 || x_nuevo >= grid->ancho || y_nuevo < 0 || y_nuevo >= grid->alto) {
        return 0; // fuera del limite no hace nada
    }

    Coordenada pos_vieja = buscar_posicion(grid, id_entidad, tipo);
    int x_viejo = pos_vieja.x;
    int y_viejo = pos_vieja.y;
    
    if (x_viejo == x_nuevo && y_viejo == y_nuevo) return 1; // si es igual no se mueve

    if (pthread_mutex_trylock(&grid->casillas[y_nuevo][x_nuevo].lock_casilla) == 0) {
        if (grid->casillas[y_nuevo][x_nuevo].ocupante == NULL) {
            
            pthread_mutex_lock(&grid->casillas[y_viejo][x_viejo].lock_casilla);
            
            grid->casillas[y_nuevo][x_nuevo].ocupante = grid->casillas[y_viejo][x_viejo].ocupante;
            grid->casillas[y_nuevo][x_nuevo].tipo_ocupante = tipo;

            grid->casillas[y_viejo][x_viejo].ocupante = NULL;
            grid->casillas[y_viejo][x_viejo].tipo_ocupante = 0;

            pthread_mutex_unlock(&grid->casillas[y_viejo][x_viejo].lock_casilla);
            pthread_mutex_unlock(&grid->casillas[y_nuevo][x_nuevo].lock_casilla);

            return 1; //hubo exito
        } else {
            pthread_mutex_unlock(&grid->casillas[y_nuevo][x_nuevo].lock_casilla);
            return 0; // falla o que la casilla esta ocupada
        }
    }
    return 0; // conflicto con el lock
}