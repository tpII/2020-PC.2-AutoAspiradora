#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Distancias máximas y mínimas que puede medir el sensor hc-sr04 en mm
#define DIST_MIN 2
#define DIST_MAX 4000

void *funcion_sensor_ultrasonido(void *ptr);

char direccion = 'C';
double distancia = 150;

void main(){

    int ret;

    srand(time(NULL));

    pthread_t ultrasonido;
    char *mensaje = "Thread 1";


    while((distancia > 10) && (ret == 0)){
        ret = pthread_create(&ultrasonido, NULL, funcion_sensor_ultrasonido,(void*)mensaje);
        pthread_join(ultrasonido,NULL);
        printf("Distancia al obstáculo: %f \n", distancia);
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