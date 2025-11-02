#include "hero.h"
#include "grid.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> 

typedef struct {
    Grid* grid;
    int heroe_idx;
} ArgHeroe;

static int distancia_manhattan(Coordenada a, Coordenada b) { // para manejar su ruta
    return abs(a.x - b.x) + abs(a.y - b.y);
}

static int buscar_y_atacar(Grid* grid, Heroe* self, int mi_idx) {
    int en_combate = 0; 

    pthread_mutex_lock(&grid->locks_heroes[mi_idx]);
    int estoy_vivo = self->hp > 0;
    pthread_mutex_unlock(&grid->locks_heroes[mi_idx]);
    
    if (!estoy_vivo) {
        return 0; // importante, es necesario este if ya que asegura que ataque siempre y cuando el heroe esta vivo
    }

    for (int i = 0; i < grid->num_monstruos; i++) {
        Monstruo* monstruo = &grid->monstruos[i];
        
        // bloquea al monstruo para poder modificar sus caracteristicas
        pthread_mutex_lock(&grid->locks_monstruos[i]);
        
        if (monstruo->hp <= 0) {
            pthread_mutex_unlock(&grid->locks_monstruos[i]);
            continue; // al matarlo pasa al siguiente monstruo
        }

        int dist = distancia_manhattan(self->posicion_actual, monstruo->posicion);
        
        //  ataca si esta en rango de ataque
        if (dist <= self->attack_range) {
            printf("Héroe %d [ATACANDO] a Monstruo %d\n", self->id, monstruo->id);
            
            en_combate = 1; // el heroe queda marcado que esta en combate
            monstruo->hp -= self->attack_damage; 
            
            if (monstruo->hp > 0) {
                printf("Monstruo %d HP: %d\n", monstruo->id, monstruo->hp);
            } else {
                printf("Monstruo %d HA MUERTO!\n", monstruo->id);
            }
            
            pthread_mutex_unlock(&grid->locks_monstruos[i]);
            break; // importante, ataca a un solo monstruo por turno
        }
        
        pthread_mutex_unlock(&grid->locks_monstruos[i]);
    }
    return en_combate; 
}

static void moverse_en_ruta(Grid* grid, Heroe* self, int mi_idx) {
    // si no hay mas pasos ya no hace nada
    if (self->ruta.paso_actual >= self->ruta.num_pasos) {
        return;
    }

    Coordenada prox_paso = self->ruta.pasos[self->ruta.paso_actual];

    // llama a la función de grid 
    if (mover_entidad_a_casilla(grid, self->id, 1, prox_paso.x, prox_paso.y)) {
        
        // cuando el movimiento es exitoso, acuatilza nuestra posicion
        pthread_mutex_lock(&grid->locks_heroes[mi_idx]);
        {
            self->posicion_actual = prox_paso;
            self->ruta.paso_actual++;
        }
        pthread_mutex_unlock(&grid->locks_heroes[mi_idx]);
        
        printf("Héroe %d [MOVIÉNDOSE] a (%d, %d)\n", self->id, prox_paso.x, prox_paso.y);
    } else {
        // printf("Héroe %d [BLOQUEADO] en (%d, %d)\n", self->id, self->posicion_actual.x, self->posicion_actual.y);
    }
}

// lógica principal
void* logica_heroe(void* arg) {
    // argumentos
    ArgHeroe* mis_args = (ArgHeroe*) arg;
    Grid* grid = mis_args->grid;
    int mi_idx = mis_args->heroe_idx;
    Heroe* self = &grid->heroes[mi_idx];
    free(mis_args);

    printf("Héroe %d iniciando en (%d, %d)\n",self->id, self->posicion_actual.x, self->posicion_actual.y);

    while (self->hp > 0) { 
        sleep(1); // simula 1 segundo entre turno
        
        // intenta atacar segun el indice para el lock
        int en_combate = buscar_y_atacar(grid, self, mi_idx); 
        
        // si no hay nada para atacar se mueve
        if (!en_combate) {
            
            // revisa nuevamente el HP, es posible que un monstruo nos matara mientras el heroe trata de buscar y atacar 
            pthread_mutex_lock(&grid->locks_heroes[mi_idx]);
            int aun_vivo = self->hp > 0;
            pthread_mutex_unlock(&grid->locks_heroes[mi_idx]);

            if (aun_vivo) {
                moverse_en_ruta(grid, self, mi_idx); 
            }
        }
    }
    
    printf("¡Héroe %d ha muerto!\n", self->id);
    return NULL;
}