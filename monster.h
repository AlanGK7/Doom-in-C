#ifndef MONSTRUO_H
#define MONSTRUO_H
#include "hero.h" 

typedef enum {
    PASIVO,
    ALERTADO,
    ATACANDO
} EstadoMonstruo;

// Estructura que define al monstruo
typedef struct {
    int id;                 // identificador único
    int hp;                 // vida 
    int attack_damage;      // daño de ataque 
    int vision_range;       // rango de vision 
    int attack_range;       // rango de ataque 
    Coordenada posicion;    // coordenadas actuales 
    EstadoMonstruo estado;  // estado actual (inicia en PASIVO )
} Monstruo;

void* logica_monstruo(void* arg);

#endif