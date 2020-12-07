#ifndef __GRAFO_H__
#define __GRAFO_H__

typedef struct coordenadas
{
    int x;
    int y;
} coordenadas;

typedef enum Direccion
{
    Derecha,
    Arriba,
    Izquierda,
    Abajo
} direccion;

typedef enum EstadoNodo
{
    Visitado,
    NoVisitado,
    Inaccesible
} estadoNodo;

typedef struct vertice
{
    estadoNodo estado; // 1 true, 0 false, -1 obstaculo
    // int x;
    // int y;
    coordenadas coordenadas;
} vertice;

typedef struct nodo
{
    vertice actual;
    struct nodo *adyacentes[4];
    struct nodo *proximo;
} nodo;

typedef struct lista
{
    nodo *cabeza;
    nodo *cola;
} listaVertices;

typedef struct grafo
{
    char * nombre;
    int xMax;
    int yMax;
    int vertices;
    listaVertices lista;
} grafo;

typedef struct datos_recorrido{
    int fin_x;
    int fin_y;
    int origen_x;
    int origen_y;
    float vel_max;
    float distancia;
} datos_recorrido;

// Inicializa la lista de vertices del grafo en NULL y le coloca el nombre
void inicializar_grafo(grafo *grafo, char *nombre);

// Agrega un nuevo vertice la lista de vertices
nodo* agregar_vertice(grafo *grafo, vertice actual);

// Agrega 2 vertices como adyacentes entre sí
void agregar_adyacente(nodo *actual, nodo *destino, direccion dirDestino);

// Imprime el grafo completo
void imprimir_grafo(grafo grafo);

// Dado un vertice y un grafo, busca todos los adyacentes que existen del mismo
// Utilizando la posicion X y la posicion Y. Siendo adyacentes aquellos que
// esten una posicion por delante en X o en Y (arriba, abajo, izquierda y derecha) 
void buscar_y_agregar_adyacentes(nodo *vertice, grafo grafo);

// Dado un grafo calcula el tamaño máximo en X y en Y para poder graficarlo en el servidor
void calculo_dimensiones_mapa(grafo *grafo);

#endif