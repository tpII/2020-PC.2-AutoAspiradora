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
    // int vertices;
    listaVertices lista;
} grafo;


void inicializar_grafo(grafo *grafo);
nodo* agregar_vertice(grafo *grafo, vertice actual);
void agregar_adyacente(nodo *actual, nodo *destino, direccion dirDestino);
void imprimir_grafo(grafo grafo);

#endif