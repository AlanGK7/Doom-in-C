# Doom-in-C

## Descripción
Este proyecto es una simulación concurrente a menor escala del videojuego Doom, desarrollada para el curso de Sistemas Operativos. La simulación utiliza pthreads para gestionar N Héroes y M Monstruos  que interactúan de forma simultánea en un grid 2D.

Los monstruos poseen una IA básica que les permite estar PASIVO, ser ALERTADO (por héroes u otros monstruos) , y ATACAR, persiguiendo al héroe vivo más cercano.

## 🚀 Cómo Ejecutar
Al clonar el repositorio y acceder a la carpeta como tal se ejecuta el siguiente comando:

```bash
make clean && make run
```

> [!IMPORTANT]
> Existen 4 archivos en el cual se hacen pruebas.
> Se debe modificar la este estracto de codigo en *main.c*:

```C
// cargar la configuración
    // Cambia el campo 
    if (parsear_config("Archivo_ejemplo.txt", &config) != 0) { // <-- CAMBIA ESTO PARA ELEGIR OTRO ARCHIVO DE CONFIGURACION
        fprintf(stderr, "Error al leer config.txt\n");
        return 1;
    }
```

## ⚙️ Formato del archivo
El parser (parser.c) espera un formato estricto que utiliza guiones bajos (_) para las claves de las entidades.
Importante: El archivo debe contener HERO_COUNT y MONSTER_COUNT para que el parser pueda asignar la memoria correctamente.

Ejemplo:
```txt
# El tamaño del grid
GRID_SIZE [ancho] [alto]

# --- HÉROES ---
HERO_COUNT [N]

HERO_1_HP [valor]
HERO_1_ATTACK_DAMAGE [valor]
HERO_1_ATTACK_RANGE [valor]
HERO_1_START [x] [y]
HERO_1_PATH (x,y) (x,y) (x,y)...

HERO_2_HP [valor]
...

# --- MONSTRUOS ---
MONSTER_COUNT [M]

MONSTER_1_HP [valor]
MONSTER_1_ATTACK_DAMAGE [valor]
MONSTER_1_VISION_RANGE [valor]
MONSTER_1_ATTACK_RANGE [valor]
MONSTER_1_COORDS [x] [y]

MONSTER_2_HP [valor]
...
```

## 🧠 Resumen Técnico: Gestión de Concurrencia
El núcleo del proyecto es la gestión de la concurrencia entre N Héroes y M Monstruos. Para evitar conflictos, se implementó una estrategia de sincronización granular:

Locks de Entidades: Cada Héroe y cada Monstruo posee su propio pthread_mutex_t. Para modificar el estado de cualquier entidad (como su hp o posicion), el hilo atacante debe primero bloquear el mutex de la entidad objetivo. Esto asegura que el acceso a la vida (HP) sea atómico y previene que dos hilos modifiquen la misma vida simultáneamente.

Locks de Casillas (Grid): Para evitar colisiones (que dos hilos ocupen la misma casilla al mismo tiempo), cada celda (x, y) del grid tiene su propio pthread_mutex_t. El movimiento se gestiona de forma optimista usando pthread_mutex_trylock sobre la casilla destino. Si el bloqueo falla (porque otro hilo la está usando), la entidad simplemente cancela su movimiento en ese turno, evitando deadlocks.

Prevención de "Ataques Fantasma": Se corrigió una race condition donde una entidad podía ser asesinada pero aun así realizar un último ataque. La solución fue añadir un auto-chequeo: antes de que cualquier entidad intente atacar, primero bloquea su propio mutex y comprueba si self->hp > 0. Si está muerta, la función de ataque termina inmediatamente.
