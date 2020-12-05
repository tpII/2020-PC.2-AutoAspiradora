#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include "include/comunicacion_simulacion_servidor.h"
#include "include/grafo.h"

CURL * configurar_conexion();

int cerrar_conexion(CURL *);


CURL * configurar_conexion(){
    CURL * curl = curl_easy_init();
    return curl;
}

int enviar_info_grafo(CURL * curl, grafo grafo, char * url){
    char campos[80];

    curl_easy_setopt(curl, CURLOPT_URL, url);
    sprintf(campos,"nombre=%s&xMax=%d&yMax=%d&vertices=%d",
    grafo.nombre,grafo.xMax,grafo.yMax,grafo.vertices);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, campos);
    curl_easy_perform(curl);
}

int enviar_datos_recorrido(CURL * curl, datos_recorrido datos, char *nombreGrafo, char * url){
    char campos[200];//Este es mas grande que los demas porque se necesitan mas caracteres

    curl_easy_setopt(curl, CURLOPT_URL, url);
    sprintf(campos,"origen_x=%d&origen_y=%d&fin_x=%d&fin_y=%d&vel_max=%f&distancia=%f&nombreGrafo=%s",
        datos.origen_x, datos.origen_y,datos.fin_x,datos.fin_y,datos.vel_max,datos.distancia,nombreGrafo);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, campos);
    curl_easy_perform(curl);
}


// Recibe un vertice y lo envía al servidor para almacenarlo en la BD
int enviar_vertices_grafo(CURL * curl, vertice v, char *nombreGrafo, char * url){
    char campos[80];
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    sprintf(campos,"x=%d&y=%d&estado=%d&nombreGrafo=%s",
        v.coordenadas.x,v.coordenadas.y,v.estado,nombreGrafo);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, campos);
    int x = curl_easy_perform(curl);
    printf("La comunicacion retorno: %d\n",x);
}



int cerrar_conexion(CURL * curl){
    curl_easy_cleanup(curl);
}


// void main(){

//     grafo g;
//     CURL * curl;

//     g.nombre = "Grafo";
//     g.vertices = 10;
//     g.xMax = 5;
//     g.yMax = 2;

//     curl = configurar_conexion();
//     if (curl != NULL){
//         enviar_info_grafo(curl,g,"http://localhost:3000/curl");
//     }
//     // cerrar_conexion(curl);

// }