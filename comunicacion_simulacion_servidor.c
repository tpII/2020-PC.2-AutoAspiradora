#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include "include/comunicacion_simulacion_servidor.h"
#include "include/grafo.h"

CURL *configurar_conexion();

int cerrar_conexion(CURL *);

CURL *configurar_conexion()
{
    CURL *curl = curl_easy_init();
    return curl;
}

int enviar_info_grafo(CURL *curl, grafo grafo, char *url)
{
    char campos[80];
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    sprintf(campos, "nombre=%s&xMax=%d&yMax=%d&vertices=%d",
            grafo.nombre, grafo.xMax, grafo.yMax, grafo.vertices);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, campos);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
}

int enviar_datos_recorrido(CURL *curl, datos_recorrido datos, char *nombreGrafo, char *url)
{
    char campos[200]; //Este es mas grande que los demas porque se necesitan mas caracteres
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    sprintf(campos, "origen_x=%d&origen_y=%d&fin_x=%d&fin_y=%d&vel_max=%f&distancia=%f&nombreGrafo=%s",
            datos.origen_x, datos.origen_y, datos.fin_x, datos.fin_y, datos.vel_max, datos.distancia, nombreGrafo);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, campos);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
}

// Recibe un vertice y lo envía al servidor para almacenarlo en la BD
int enviar_vertices_grafo(CURL *curl, vertice v, char *nombreGrafo, char *url)
{
    char campos[80];
    curl = curl_easy_init();
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    sprintf(campos,"x=%d&y=%d&estado=%d&nombreGrafo=%s",
        v.coordenadas.x,v.coordenadas.y,v.estado,nombreGrafo);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, campos);
    
    int x = curl_easy_perform(curl);
    printf("La comunicacion retorno: %d\n",x);
    curl_easy_cleanup(curl);
}

void init_string(struct string *s)
{
    s->len = 0;
    s->ptr = malloc(s->len + 1);
    s->ptr[0] = '\0';
}

void leer_estado_robot(struct string s, struct estadoRobot *estado)
{
    estado->ventiladorEncendido = s.ptr[0] - '0';
    estado->modoAutomatico = s.ptr[1] - '0';
    estado->mapeando = s.ptr[2] - '0';
    estado->direccionManual = s.ptr[3] - '0';
}

size_t write_func(void *ptr, size_t size, size_t nmemb, struct string *s)
{
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);
    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size * nmemb;
}

void consultar_estado_robot(CURL *curl, char *url, struct estadoRobot *estado)
{
    curl = curl_easy_init();
    struct string s;
    init_string(&s);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    curl_easy_perform(curl);
    leer_estado_robot(s, estado);
    free(s.ptr);
    curl_easy_cleanup(curl);
}

void consultar_nombre_habitacion(CURL *curl, char *url, char **nombreHabitacion)
{
    curl = curl_easy_init();
    struct string s;
    init_string(&s);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    curl_easy_perform(curl);
    *nombreHabitacion = malloc(strlen(s.ptr) + 1);
    strcpy(*nombreHabitacion, s.ptr);
    curl_easy_cleanup(curl);
}

int cerrar_conexion(CURL *curl)
{
    curl_easy_cleanup(curl);
}

// void main()
// {
//     CURL *curl;
//     curl = configurar_conexion();
//     if (curl)
//     {
//         consultar_estado_robot(curl, "http://localhost:3000/api/robotAspiradora/consultaEstado");
//     }
// }

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