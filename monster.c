#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h> 
#include <math.h>   
#include "monster.h"
#include "grid.h"

typedef struct {
    Grid* grid;
    int monstruo_idx;
} ArgMonstruo;

// funcion auxiliar para calcular distancia
static int distancia_manhattan(Coordenada a, Coordenada b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

static void buscar_heroes(Grid* grid, Monstruo* self) {
    // iterar sobre todos los heroes
    for (int i = 0; i < grid->num_heroes; i++) {
        pthread_mutex_lock(&grid->locks_heroes[i]);
        if (grid->heroes[i].hp <= 0) {
            pthread_mutex_unlock(&grid->locks_heroes[i]);
            continue; // heroe muerto
        }
        Coordenada pos_heroe = grid->heroes[i].posicion_actual;
        pthread_mutex_unlock(&grid->locks_heroes[i]);
        
        // revisa en su campo de vision y cambia su estado y calcula la distancia para llegar al heroe
        int dist = distancia_manhattan(self->posicion, pos_heroe);
        if (dist <= self->vision_range) {
            pthread_mutex_lock(&grid->locks_monstruos[self->id - 1]);
            if (self->estado == PASIVO) {
                self->estado = ALERTADO;
                printf("Monstruo %d [ALERTADO] por Héroe %d.\n", self->id, grid->heroes[i].id);
            }
            pthread_mutex_unlock(&grid->locks_monstruos[self->id - 1]);
            break; // ya estan alertados, sale del bucle
        }
    }
}

static void alertar_otros(Grid* grid, Monstruo* self) {
    int mi_idx = self->id - 1; // nuestro propio indice

    // Iterar sobre todos los monstruos
    for (int i = 0; i < grid->num_monstruos; i++) {
        if (i == mi_idx) continue; // Nno se alerta a el mismo

        Monstruo* otro_monstruo = &grid->monstruos[i];

        pthread_mutex_lock(&grid->locks_monstruos[i]);
        { // va a la posicion de un mosntruo alertado
            if (otro_monstruo->estado == PASIVO) {
                int dist = distancia_manhattan(self->posicion, otro_monstruo->posicion);
                if (dist <= self->vision_range) {
                    otro_monstruo->estado = ALERTADO;
                    printf("Monstruo %d [ALERTÓ] a Monstruo %d!\n", self->id, otro_monstruo->id);
                }
            }
        }
        pthread_mutex_unlock(&grid->locks_monstruos[i]);
    }
}

static void perseguir_y_atacar(Grid* grid, Monstruo* self) {
    pthread_mutex_lock(&grid->locks_monstruos[self->id - 1]);
    int estoy_vivo = self->hp > 0;
    pthread_mutex_unlock(&grid->locks_monstruos[self->id - 1]);
    if (!estoy_vivo) return;

    // busca al heroe mas cercano
    Heroe* heroe_objetivo = NULL;
    int idx_objetivo = -1;
    int min_dist = INT_MAX;

    for (int i = 0; i < grid->num_heroes; i++) {
        pthread_mutex_lock(&grid->locks_heroes[i]);
        if (grid->heroes[i].hp <= 0) {
            pthread_mutex_unlock(&grid->locks_heroes[i]);
            continue; // si esta muerto el heroe
        }
        Coordenada pos_heroe = grid->heroes[i].posicion_actual;
        pthread_mutex_unlock(&grid->locks_heroes[i]);

        int dist = distancia_manhattan(self->posicion, pos_heroe);
        if (dist < min_dist) {
            min_dist = dist;
            heroe_objetivo = &grid->heroes[i];
            idx_objetivo = i;
        }
    }

    if (heroe_objetivo == NULL) return; 

    // ataca al heroe mas cercano
    if (min_dist <= self->attack_range) {
        printf("Monstruo %d [ATACANDO] a Héroe %d (dist: %d)\n", self->id, heroe_objetivo->id, min_dist);
        pthread_mutex_lock(&grid->locks_heroes[idx_objetivo]);
        if (heroe_objetivo->hp > 0) {
            heroe_objetivo->hp -= self->attack_damage;
            printf("Héroe %d HP: %d\n", heroe_objetivo->id, heroe_objetivo->hp);
        }
        pthread_mutex_unlock(&grid->locks_heroes[idx_objetivo]);
        return; 
    }

    // logica de movimiento
    Coordenada prox_paso = self->posicion;
    Coordenada pos_heroe = heroe_objetivo->posicion_actual; // pos del objetivo
    int dx = pos_heroe.x - self->posicion.x;
    int dy = pos_heroe.y - self->posicion.y;

    if (abs(dx) > abs(dy)) {
        prox_paso.x += (dx > 0) ? 1 : -1;
    } else if (abs(dy) > 0) { 
        prox_paso.y += (dy > 0) ? 1 : -1;
    }

    if (mover_entidad_a_casilla(grid, self->id, 2, prox_paso.x, prox_paso.y)) {
        pthread_mutex_lock(&grid->locks_monstruos[self->id - 1]);
        self->posicion = prox_paso;
        pthread_mutex_unlock(&grid->locks_monstruos[self->id - 1]);
        // printf("Monstruo %d [MOVIÉNDOSE] a (%d, %d)\n", self->id, prox_paso.x, prox_paso.y);
    }
}

//  logica para el hilo
void* logica_monstruo(void* arg) {
    ArgMonstruo* mis_args = (ArgMonstruo*) arg;
    Grid* grid = mis_args->grid;
    int mi_idx = mis_args->monstruo_idx;
    Monstruo* self = &grid->monstruos[mi_idx];
    free(mis_args);
    
    self->estado = PASIVO;

    printf("Monstruo %d [PASIVO] en (%d, %d)\n", self->id, self->posicion.x, self->posicion.y);
    
    while (self->hp > 0) {
        sleep(1); 
        
        if (self->estado == PASIVO) {
            buscar_heroes(grid, self);
            continue; 
        }
        
        if (self->estado == ALERTADO) {
            alertar_otros(grid, self);
            pthread_mutex_lock(&grid->locks_monstruos[mi_idx]);
            self->estado = ATACANDO; 
            pthread_mutex_unlock(&grid->locks_monstruos[mi_idx]);
            continue; 
        }
        
        if (self->estado == ATACANDO) {
            perseguir_y_atacar(grid, self);
        }
    }
    
    printf("Monstruo %d ha muerto.\n", self->id);
    return NULL;
}