#include <stdio.h>
#include <stdlib.h>
// Se usa la implementacion de lista de adyacentes

// typedef struct nodoAdyentes {
//     int visitado; // 1 true, 0 false
//     int x;
//     int y;
//     nodoAdyacentes* proximo;
// } as nodoAdyacentes;

typedef struct vertice {
    int visitado; // 1 true, 0 false, -1 obstaculo
    int x;
    int y;
} vertice;

typedef struct nodo{
    vertice actual;
    vertice *adyacentes;
    struct nodo* proximo;
} nodo;

typedef struct lista{
    nodo* cabeza;
    nodo* cola;
} listaVertices;

typedef struct grafo{
    listaVertices lista;
} grafo;

void agregar_vertice(grafo* grafo, vertice actual, vertice* adyacentes){ 
    nodo* nuevoNodo = (nodo*) malloc(sizeof(nodo));  
    nuevoNodo->actual = actual;
    nuevoNodo->adyacentes = adyacentes;
    nuevoNodo->proximo = NULL;
    if(grafo->lista.cabeza == NULL){
        grafo->lista.cabeza = nuevoNodo;
        grafo->lista.cola = nuevoNodo;
    }
    else{
        grafo->lista.cola->proximo = nuevoNodo;
        grafo->lista.cola = grafo->lista.cola->proximo;
    }
}

void inicializar_grafo(grafo * grafo){
    grafo->lista.cola = NULL;
    grafo->lista.cabeza = grafo->lista.cola;
}

void main(){
    grafo grafo;    
    inicializar_grafo(&grafo);
    vertice v;
    v.x = 0;
    v.y = 0;
    v.visitado = 1;
    vertice ady[4];
    ady[0].x = 0;
    ady[0].y = -1;
    ady[0].visitado = 0;

    ady[1].x = 1;
    ady[1].y = 0;
    ady[1].visitado = 0;
    
    ady[2].x = 0;
    ady[2].y = 1;
    ady[2].visitado = 0;
    
    ady[3].x = -1;
    ady[3].y = 0;
    ady[3].visitado = -1;

    agregar_vertice(&grafo,v,ady);

    v.x = 1;
    v.y = 0; 

    agregar_vertice(&grafo,v,ady);

    // Imprimimos
    printf("Coordenada X del primer vertice: %d \n",grafo.lista.cabeza->actual.x);

    // Imprimimos
    printf("Adyacentes del primer vertice %d %d %d %d \n",grafo.lista.cabeza->adyacentes[0].x,grafo.lista.cabeza->adyacentes[1].x,grafo.lista.cabeza->adyacentes[2].x,grafo.lista.cabeza->adyacentes[3].x);

    // Imprimimos
    printf("Coordenada X del ultimo vertice: %d \n",grafo.lista.cola->actual.x);

}



buscarDireccionOptima();
posicionarse()
avanzo(direccionOptima){
    crearVertice();
    while (no revise 4 direcciones){
        if (no hay obstaculo && No visite el siguiente)
            avanzo(direccionOptima);
        else 
            buscarDireccionOptima();
            posicionarse();
            avanzo(direccionOptima);
    }
}