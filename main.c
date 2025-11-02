// en main.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "parser.h" 
#include "grid.h"
#include "hero.h"
#include "monster.h"

// argumento para los hilos de los heroes
typedef struct {
    Grid* grid;
    int heroe_idx;
} ArgHeroe;

// argumentos para los hilos de monstruos
typedef struct {
    Grid* grid;
    int monstruo_idx;
} ArgMonstruo;


int main() {
    Configuracion config = {0};

    // se carga la configuracion
    if (parsear_config("ejemplo2.txt", &config) != 0) { // <------------- se cambia el campo txt para elegir configuraciones (campo, heroes, mosntruos)
        fprintf(stderr, "Error al leer config.txt\n");
        return 1;
    }

    // se inicializa el grid con los heroes
    Grid* grid = inicializar_grid(
        config.grid_ancho, config.grid_alto,
        config.heroes_iniciales, config.num_heroes,
        config.monstruos_iniciales, config.num_monstruos
    );

    if (grid == NULL) {
        fprintf(stderr, "Error al inicializar el grid\n");
        return 1;
    }
    printf("Campo de batalla (%dx%d) inicializado con %d héroes y %d monstruos.\n",
           config.grid_ancho, config.grid_alto, config.num_heroes, config.num_monstruos);

    // crea los n hilos segun la config
    pthread_t* hilos_heroes = malloc(sizeof(pthread_t) * config.num_heroes);
    for (int i = 0; i < config.num_heroes; i++) {
        ArgHeroe* arg = malloc(sizeof(ArgHeroe));
        arg->grid = grid;
        arg->heroe_idx = i;
        pthread_create(&hilos_heroes[i], NULL, logica_heroe, (void*)arg);
    }

    // m hilos para monstruos
    pthread_t* hilos_monstruos = malloc(sizeof(pthread_t) * config.num_monstruos);
    for (int i = 0; i < config.num_monstruos; i++) {
        ArgMonstruo* arg = malloc(sizeof(ArgMonstruo));
        arg->grid = grid;
        arg->monstruo_idx = i;
        pthread_create(&hilos_monstruos[i], NULL, logica_monstruo, (void*)arg);
    }

    // se espera que todos los hilos terminen
    for (int i = 0; i < config.num_heroes; i++) {
        pthread_join(hilos_heroes[i], NULL);
    }
    for (int i = 0; i < config.num_monstruos; i++) {
        pthread_join(hilos_monstruos[i], NULL);
    }

    // limpia y libera memoria
    printf("Simulación terminada. Limpiando...\n");
    destruir_grid(grid);
    for (int i = 0; i < config.num_heroes; i++) {
        free(config.heroes_iniciales[i].ruta.pasos);
    }
    free(config.heroes_iniciales);
    free(config.monstruos_iniciales);
    free(hilos_heroes);
    free(hilos_monstruos);
    
    return 0;
}