#include <stdio.h>
#include <stdlib.h>
#include "include/grafo.h"
// Se usa la implementacion de lista de adyacentes

nodo *agregar_vertice(grafo *grafo, vertice actual)
{
    int i;

    nodo *nuevoNodo = (nodo *)malloc(sizeof(nodo));
    nuevoNodo->actual = actual;
    // Se inicializan en NULL los 4 nodos adyacentes
    for (i = 0; i < 4; i++)
    {
        nuevoNodo->adyacentes[i] = NULL;
    }
    nuevoNodo->proximo = NULL;
    if (grafo->lista.cabeza == NULL)
    {
        grafo->lista.cabeza = nuevoNodo;
        grafo->lista.cola = nuevoNodo;
    }
    else
    {
        grafo->lista.cola->proximo = nuevoNodo;
        grafo->lista.cola = grafo->lista.cola->proximo;
    }
    return nuevoNodo;
}

void agregar_adyacente(nodo *actual, nodo *destino, direccion dirDestino)
{
    // Se agrega el nodo destino como adyacente del actual
    actual->adyacentes[dirDestino] = destino;

    // Se agrega el nodo actual como adyacente del nodo destino
    switch (dirDestino)
    {
    case Arriba:
        destino->adyacentes[Abajo] = actual;
        break;
    case Abajo:
        destino->adyacentes[Arriba] = actual;
        break;
    case Derecha:
        destino->adyacentes[Izquierda] = actual;
        break;
    case Izquierda:
        destino->adyacentes[Derecha] = actual;
        break;
    default:
        break;
    }
}

void inicializar_grafo(grafo *grafo, char *nombre)
{
    grafo->lista.cola = NULL;
    grafo->lista.cabeza = grafo->lista.cola;
    grafo->vertices=0;
    grafo->nombre=nombre;
}

void buscar_y_agregar_adyacentes(nodo *vertice, grafo grafo)
{
    int x = vertice->actual.coordenadas.x;
    int y = vertice->actual.coordenadas.y;
    nodo *listaVertices = grafo.lista.cabeza;

    // Se buscan y agregan todos los adyacentes que existan en el grafo
    while(listaVertices != NULL){
        if(listaVertices->actual.coordenadas.x == x){
            if(listaVertices->actual.coordenadas.y == (y + 1)){
                agregar_adyacente(vertice,listaVertices,Abajo);
            }
            else if(listaVertices->actual.coordenadas.y == (y - 1))
            {
                agregar_adyacente(vertice,listaVertices,Arriba);
            }
        }
        else if(listaVertices->actual.coordenadas.y == y){
            if(listaVertices->actual.coordenadas.x == (x + 1)){
                agregar_adyacente(vertice,listaVertices,Derecha);
            }
            else if(listaVertices->actual.coordenadas.x == (x - 1))
            {
                agregar_adyacente(vertice,listaVertices,Izquierda);
            }
        }
        listaVertices = listaVertices->proximo;
    }
}

void imprimir_grafo(grafo grafo)
{
    int i;

    while (grafo.lista.cabeza != NULL)
    {
        printf("\n x: %d y: %d |", grafo.lista.cabeza->actual.coordenadas.x, grafo.lista.cabeza->actual.coordenadas.y);
        for (i = 0; i < 4; i++)
        {
            if (grafo.lista.cabeza->adyacentes[i] != NULL)
            {
                printf("-> x: %d y: %d", grafo.lista.cabeza->adyacentes[i]->actual.coordenadas.x, grafo.lista.cabeza->adyacentes[i]->actual.coordenadas.y);
            }
        }
        grafo.lista.cabeza = grafo.lista.cabeza->proximo;
    }

    printf("\n");
}

void calculo_dimensiones_mapa(grafo *grafo){
    int xMax = 0;
    int yMax = 0;
    nodo *listaVertices = grafo->lista.cabeza;

    // Se recorre la lista de vertices calculando el x e y maximos.
    while(listaVertices != NULL){

        if(xMax<listaVertices->actual.coordenadas.x){
            xMax=listaVertices->actual.coordenadas.x;
        }

        if(yMax<listaVertices->actual.coordenadas.y){
            yMax=listaVertices->actual.coordenadas.y;
        }
        
        listaVertices = listaVertices->proximo;
    }

    // Se le suma uno porque los indices comienzan en 0.
    grafo->xMax = xMax+1;
    grafo->yMax = yMax+1;
}