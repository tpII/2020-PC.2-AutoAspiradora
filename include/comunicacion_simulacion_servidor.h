#ifndef __COMUNICACION_SIMULACION_SERVIDOR_H__
#define __COMUNICACION_SIMULACION_SERVIDOR_H__

#include "grafo.h"

struct estadoRobot
{
    int ventiladorEncendido;
    int modoAutomatico;
    int mapeando;
    int direccionManual;
};

struct string
{
    char *ptr;
    size_t len;
};

// Devuelve una variable CURL con la conexión seteada con la url pasada como parámetro.
CURL * configurar_conexion();

// Envía los metadatos del grafo.
int enviar_info_grafo(CURL *, grafo, char *);

// Envía un grafo completo para almacenar en la base de datos.
int enviar_vertices_grafo(CURL * curl, vertice v, char *nombreGrafo, char * url);

// Envía los datos de los recorridos que realiza entre obstáculos.
int enviar_datos_recorrido(CURL *, datos_recorrido, char *nombreGrafo, char *);

// Termina la conexión, debe usarse una vez que no vayan a enviarse más datos.
int cerrar_conexion(CURL *);

void consultar_estado_robot(CURL *curl, char * url, struct estadoRobot * estado);

void consultar_nombre_habitacion(CURL *curl, char * url, char ** nombreHabitacion);

#endif