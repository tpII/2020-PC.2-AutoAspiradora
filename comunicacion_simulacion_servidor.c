#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include "include/comunicacion_simulacion_servidor.h"

CURL * configurar_conexion(char *);
int enviar_datos(CURL *, datos_recorrido);
int cerrar_conexion(CURL *);


CURL * configurar_conexion(char * url){
    CURL * curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    return curl;
}

int enviar_datos(CURL * curl, datos_recorrido datos){
    char campos[80];
    sprintf(campos,"x=%f&y=%f&vel_max=%f&distancia=%f",
        datos.obstaculo_x,datos.obstaculo_y,datos.vel_max,datos.distancia);
    puts(campos);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, campos);
    curl_easy_perform(curl);
}

int cerrar_conexion(CURL * curl){
    curl_easy_cleanup(curl);
}
