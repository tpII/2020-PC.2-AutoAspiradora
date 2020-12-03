#ifndef __COMUNICACION_SIMULACION_SERVIDOR_H__
#define __COMUNICACION_SIMULACION_SERVIDOR_H__

#include "grafo.h"

typedef struct datos_recorrido{
    int obstaculo_x;
    int obstaculo_y;
    int origen_x;
    int origen_y;
    float vel_max;
    float distancia;
} datos_recorrido;

// Devuelve una variable CURL con la conexión seteada con la url pasada como parámetro.
CURL * configurar_conexion();

// Envía los metadatos del grafo.
int enviar_info_grafo(CURL *, grafo, char *);

// Envía un grafo completo para almacenar en la base de datos.
int enviar_vertices_grafo(CURL *, grafo, char *);

// Envía los datos de los recorridos que realiza entre obstáculos.
int enviar_datos_recorrido(CURL *, datos_recorrido, char *);

// Termina la conexión, debe usarse una vez que no vayan a enviarse más datos.
int cerrar_conexion(CURL *);

#endif