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

void inicializar_grafo(grafo *grafo)
{
    grafo->lista.cola = NULL;
    grafo->lista.cabeza = grafo->lista.cola;
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

// void main()
// {
//     nodo *nodoAux;;
//     grafo grafo;

//     int i;

//     nodo *verticeActual;
//     nodo *verticeProximo;
//     inicializar_grafo(&grafo);
//     vertice actual;
//     actual.coordenadas.x = 0;
//     actual.coordenadas.y = 0;
//     actual.estado = Visitado;

//     verticeActual = agregar_vertice(&grafo, actual);

//     vertice proximo;
//     proximo.coordenadas.x = 1;
//     proximo.coordenadas.y = 0;
//     proximo.estado = NoVisitado;

//     verticeProximo = agregar_vertice(&grafo, proximo);

//     agregar_adyacente(verticeActual, verticeProximo, Derecha);

//     nodoAux=grafo.lista.cabeza;
//     while(nodoAux!=NULL){
//         printf("\n x: %d y: %d |",nodoAux->actual.coordenadas.x,nodoAux->actual.coordenadas.y);
//         for(i=0; i<4; i++){
//             if(nodoAux->adyacentes[i] != NULL){
//                 printf("-> x: %d y: %d", nodoAux->adyacentes[i]->actual.coordenadas.x, nodoAux->adyacentes[i]->actual.coordenadas.y);
//             }
//         }
//          nodoAux = nodoAux->proximo;
//     }
// }

// buscarDireccionOptima();
// posicionarse()
// avanzo(direccionOptima){
//     crearVertice();
//     while (no revise 4 direcciones){
//         if (no hay obstaculo && No visite el siguiente)
//             avanzo(direccionOptima);
//         else
//             buscarDireccionOptima();
//             posicionarse();
//             avanzo(direccionOptima);
//     }
// }
