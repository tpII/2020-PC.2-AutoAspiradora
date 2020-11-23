#ifndef COMUNICACION_SIMULACION_SERVIDOR_H
#define COMUNICACION_SIMULACION_SERVIDOR_H

typedef struct datos_recorrido{
    float obstaculo_x;
    float obstaculo_y;
    float vel_max;
    float distancia;
} datos_recorrido;

CURL * configurar_conexion(char *);
int enviar_datos(CURL *, datos_recorrido);
int cerrar_conexion(CURL *);

#endif