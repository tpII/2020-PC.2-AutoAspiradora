#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>//Este es para el usleep


// Distancias máximas y mínimas que puede medir el sensor hc-sr04 en mm
#define DIST_MIN 2
#define DIST_MAX 4000

void *funcion_sensor_ultrasonido(void *ptr);
void *funcion_sensor_encoder_derecha(void * ptr);
void *funcion_sensor_encoder_izquierda(void * ptr);

typedef enum{ADELANTE, ATRAS, QUIETO} movimiento;//Indicar el movimiento de las ruedas

char direccion = 'C';
double distancia = 150;
int encoderState_derecha=0; //Indica el valor que devuelve el encoder derecho
int encoderState_izquierda=0; //Indica el valor que devuelve el encoder izquierda

movimiento movimientoRuedaDerecha = ATRAS;
movimiento movimientoRuedaIzquierda = ADELANTE;

void main(){

    int ret=0;

    srand(time(NULL));

    pthread_t ultrasonido;
    pthread_t ruedaDer;
    pthread_t ruedaIzq;
    
    char *mensaje = "Thread 1";
    char *mensaje2 = "Thread 2";
    char *mensaje3 = "Thread 3";


    while((distancia > 10) && (ret == 0)){
        ret = pthread_create(&ultrasonido, NULL, funcion_sensor_ultrasonido,(void*)mensaje);
        
        pthread_join(ultrasonido,NULL);
        
        printf("Distancia al obstáculo: %f \n", distancia);
    }
    while(1){//Habria que cambiar la condicion
        pthread_create(&ruedaDer, NULL, funcion_sensor_encoder_derecha,(void*)mensaje2);
        pthread_create(&ruedaIzq, NULL, funcion_sensor_encoder_izquierda,(void*)mensaje3);
        pthread_join(ruedaDer,NULL);
        pthread_join(ruedaIzq,NULL);
    }
        

    printf("SE ALCANZO EL OBSTÁCULO \n");

}

void *funcion_sensor_ultrasonido(void * ptr){
    char *mensaje;
    mensaje = (char *) ptr;
    printf("%s \n", (char *) ptr);

    // Interpreta que todavía no se calculo la distancia en esta direccion
    if(distancia == 0){
        // Numero aleatorio en cm, con un decimal. En el rango de 2 a 4000mm
        distancia = (double)((rand() % (DIST_MAX - DIST_MIN + 1)) + DIST_MIN) / 10; 
    }
    // Interpreta que el auto se está moviendo y calculando la distancia
    else {
        // Numero aleatorio en el rango de 2 al valor de distancia.
        distancia = (double)((rand() % ((int)distancia*10 - DIST_MIN + 1)) + DIST_MIN) / 10; 
    }
}

void *funcion_sensor_encoder_derecha(void * ptr){
    char *mensaje;
    mensaje = (char *) ptr;
    printf("%s \n", (char *) ptr);
    if((movimientoRuedaDerecha==ATRAS) || (movimientoRuedaDerecha==ADELANTE)){//Si la rueda se mueve en alguna direccion se modifica el valor que devuelve el encoder
        encoderState_derecha= 1 - encoderState_derecha; //Se hace un toggle entre 1 y 0
        printf("Rueda derecha: %d\n", encoderState_derecha);
        usleep(25000);//Se espera 0.025 seg, esto cambiaria segun la velocidad de la rueda
    }
}

void *funcion_sensor_encoder_izquierda(void * ptr){
    char *mensaje;
    mensaje = (char *) ptr;
    printf("%s \n", (char *) ptr);
    if((movimientoRuedaIzquierda==ATRAS) || (movimientoRuedaIzquierda==ADELANTE)){//Si la rueda se mueve en alguna direccion se modifica el valor que devuelve el encoder
        encoderState_izquierda = 1 - encoderState_izquierda; //Se hace un toggle entre 1 y 0
        printf("Rueda izquierda: %d\n", encoderState_izquierda);
        usleep(25000);//Se espera 0.025 seg, esto cambiaria segun la velocidad de la rueda
    }
    
}