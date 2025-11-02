#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

static void parsear_ruta(char* linea, Ruta* ruta) {
    // cuenta a cuantos pasos hay
    char* copia_linea = strdup(linea); // se copia la linea porque strtok la modifica
    char* token = strtok(copia_linea, " ");
    int contador = 0;
    while (token != NULL) {
        if (token[0] == '(') { //  se cuenta cada x y
            contador++;
        }
        token = strtok(NULL, " ");
    }
    free(copia_linea);

    // signa memoria y rellena con la ruta
    ruta->num_pasos = contador;
    ruta->pasos = malloc(sizeof(Coordenada) * contador);
    ruta->paso_actual = 0; 

    token = strtok(linea, " "); // la clave (ej "HERO_1_PATH")
    token = strtok(NULL, " "); // primer (x,y)
    int i = 0;
    while (token != NULL && i < contador) {
        // sscanf para obtener x y
        sscanf(token, "(%d,%d)", &ruta->pasos[i].x, &ruta->pasos[i].y);
        i++;
        token = strtok(NULL, " ");
    }
}

int parsear_config(const char* filename, Configuracion* config) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) { /* ... manejo de error ... */ }

    // inicializar contadores a 0
    config->num_heroes = 0;
    config->num_monstruos = 0;

    char linea[512]; // aumentar buffer para lineas de ruta
    int id_heroe, id_monstruo;

    while (fgets(linea, sizeof(linea), file)) {
        linea[strcspn(linea, "\r\n")] = 0; // limpia los saltos de linea
        if (linea[0] == '#' || linea[0] == '\0') continue; // ignorar comentarios y lineas vacias

        char copia_linea[512];
        strcpy(copia_linea, linea);
        
        char* key = strtok(copia_linea, " ");
        if (key == NULL) continue; 

        // parsea el grid
        if (strcmp(key, "GRID_SIZE") == 0) {
            sscanf(linea, "GRID_SIZE %d %d", &config->grid_ancho, &config->grid_alto);
        } 
        
        // parsea los contadores
        else if (strcmp(key, "HERO_COUNT") == 0) { 
            sscanf(linea, "HERO_COUNT %d", &config->num_heroes);
            config->heroes_iniciales = malloc(sizeof(Heroe) * config->num_heroes);
        } else if (strcmp(key, "MONSTER_COUNT") == 0) {
            sscanf(linea, "MONSTER_COUNT %d", &config->num_monstruos);
            config->monstruos_iniciales = malloc(sizeof(Monstruo) * config->num_monstruos);
        }
        
        // parseo de heroes
        else if (strncmp(key, "HERO_", 5) == 0) {
            char sub_key[50];
            if (sscanf(key, "HERO_%d_%s", &id_heroe, sub_key) != 2) continue;
            int idx = id_heroe - 1;
            config->heroes_iniciales[idx].id = id_heroe; // asigna ID
            
            if (strcmp(sub_key, "HP") == 0) {
                sscanf(linea, "%*s %d", &config->heroes_iniciales[idx].hp);
            } else if (strcmp(sub_key, "ATTACK_DAMAGE") == 0) {
                sscanf(linea, "%*s %d", &config->heroes_iniciales[idx].attack_damage);
            } else if (strcmp(sub_key, "ATTACK_RANGE") == 0) {
                sscanf(linea, "%*s %d", &config->heroes_iniciales[idx].attack_range);
            } else if (strcmp(sub_key, "START") == 0) {
                sscanf(linea, "%*s %d %d", &config->heroes_iniciales[idx].posicion_actual.x, &config->heroes_iniciales[idx].posicion_actual.y);
            } else if (strcmp(sub_key, "PATH") == 0) {
                parsear_ruta(linea, &config->heroes_iniciales[idx].ruta);
            }
        }
        
        else if (strncmp(key, "MONSTER_", 8) == 0) {
            char sub_key[50];
            if (sscanf(key, "MONSTER_%d_%s", &id_monstruo, sub_key) != 2) continue;
            int idx = id_monstruo - 1;
            config->monstruos_iniciales[idx].id = id_monstruo;

            if (strcmp(sub_key, "HP") == 0) {
                sscanf(linea, "%*s %d", &config->monstruos_iniciales[idx].hp);
            } else if (strcmp(sub_key, "ATTACK_DAMAGE") == 0) {
                sscanf(linea, "%*s %d", &config->monstruos_iniciales[idx].attack_damage);
            } else if (strcmp(sub_key, "VISION_RANGE") == 0) {
                sscanf(linea, "%*s %d", &config->monstruos_iniciales[idx].vision_range);
            } else if (strcmp(sub_key, "ATTACK_RANGE") == 0) {
                sscanf(linea, "%*s %d", &config->monstruos_iniciales[idx].attack_range);
            } else if (strcmp(sub_key, "COORDS") == 0) {
                sscanf(linea, "%*s %d %d", &config->monstruos_iniciales[idx].posicion.x, &config->monstruos_iniciales[idx].posicion.y);
            }
        }
    }
    fclose(file);
    return 0;
}