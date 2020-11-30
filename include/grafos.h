#ifndef __GRAFO_H__
#define __GRAFO_H__

typedef enum Direccion
{
    Arriba,
    Derecha,
    Abajo,
    Izquierda
} direccion;

typedef enum EstadoNodo
{
    Visitado,
    NoVisitado,
    Inaccesible
} estadoNodo;

typedef struct vertice
{
    estadoNodo visitado; // 1 true, 0 false, -1 obstaculo
    int x;
    int y;
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


#endif