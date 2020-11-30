#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> //Este es para el usleep
#include <semaphore.h>
#include "include/grafo.h"

// Distancias máximas y mínimas que puede medir el sensor hc-sr04 en mm
#define DIST_MIN 2
#define DIST_MAX 4000

#define INT_GIRO_90 300         //Cantidad de interrupciones para hacer un giro de 90 grados
#define INT_GIRO_180 1800       //Cantidad de interrupciones para hacer un giro de 180 grados
#define INT_AVANZAR_POSICION 23 //Cantidad de interruciones para avanzar 22 cm

// 56 ranuras por segundo => 571.8 mm/s

void *funcion_sensor_ultrasonido(void *ptr);
void *funcion_sensor_encoder_derecha(void *ptr);
void *funcion_sensor_encoder_izquierda(void *ptr);


typedef enum
{
    ADELANTE,
    ATRAS,
    QUIETO
} movimiento; //Indicar el movimiento de las ruedas
typedef enum
{
    MANUAL,
    AUTOMATICO
} estado_modo;
typedef enum
{
    MOVIENDOSE,
    BARRIDO,
    ESQUIVANDO,
    MIRANDO_IZQUIERDA,
    MIRANDO_DERECHA,
    GIRANDO_DERECHA,
    GIRANDO_IZQUIERDA,
    GIRANDO_180
} estado_automatico;

void MEF_Modo_Aspiradora();
void MEF_Automatico();
void MEF_Accion_Modo();
void MEF_Accion_Automatico();

void Detener();
void MoverAdelante();
void MoverPosicion();
int Observar();
void servoMirarCentro();
void servoMirarDerecha();
void servoMirarIzquierda();
void motorGirarIzquierda();
void motorGirarDerecha();
void motorGirar180();
void contar_interrupciones(int cont);
void Secuencia_Inicio(void);
void delay(void);
coordenadas coordenadasAyacente(int, int);
direccion direccionAdyacente();

// Matriz que representa la habitación
// 0 -> lugar accesible, 1 -> obstáculo
int habitacion[10][10] =
    {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 1, 1, 1, 1, 0, 0, 1},
        {1, 0, 0, 1, 1, 1, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 1, 0, 0, 1},
        {1, 0, 1, 1, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 1, 1, 0, 1},
        {1, 1, 1, 1, 0, 0, 1, 1, 0, 1},
        {1, 1, 1, 1, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
int x = 1;
int y = 1;
int direccionServo = 0;
int direccionRobot = 0; // Medimos en grados (0º, 90º, 180º, 270º)
double distancia = 150;
int distanciaMaxima;
int encoderState_derecha = 0; //Indica el valor que devuelve el encoder derecho
int encoderState_derecha_anterior = 0;
int encoderState_izquierda = 0; //Indica el valor que devuelve el encoder izquierda
int encoderState_izquierda_anterior = 0;
int robotEncendido = 0; //Booleano
int trigger = 0;        //triger del ultrasonido
int echo;               //echo del ultrasonido
int distanciaDerecha;   //Distancia medida por el ultrasonico a la derecha
int distanciaIzquierda; //Distancia medida por el ultrasonico a la izquierda
int distanciaCentro;    //Distancia medida por el ultrasonico al centro
int hayObstaculo180;    //Indica si hay obstaculo luego de hacer un giro de 180 grados
int posiciones;
int contPrueba;

movimiento movimientoRuedaDerecha = ATRAS;
movimiento movimientoRuedaIzquierda = ADELANTE;

//Variables de las maquinas de estado
int hayObstaculo;
estado_automatico estadoAnterior;
estado_automatico estadoActual;
estado_automatico estadoActualModo;

int flagServo;
int flagMotor;

sem_t semaforoDistancia;

int Ultrasonico_Trigger(void)
{ //Si hay un obstaculo a menos de 10cm devuelve 1 si no devuelve 0
    int obstaculo = 0;
    trigger = 1;
    sem_wait(&semaforoDistancia); //Se espera con un semaforo al thread del ultrasonico a que calcule la distancia

    if (distancia < 10)
    {
        obstaculo = 1;
    }
    else
    {
        obstaculo = 0;
    }
    return obstaculo;
}

grafo grafoMapa;
void main()
{

    int ret = 0;

    srand(time(NULL));

    pthread_t ultrasonido;
    pthread_t ruedaDer;
    pthread_t ruedaIzq;

    sem_init(&semaforoDistancia, 0, 0);

    char *mensaje = "Thread 1";
    char *mensaje2 = "Thread 2";
    char *mensaje3 = "Thread 3";

    int cant;

    estadoActual = BARRIDO;
    estadoActualModo = AUTOMATICO;

    // Se inicializa el grafo
    inicializar_grafo(&grafoMapa);

    ret = pthread_create(&ultrasonido, NULL, funcion_sensor_ultrasonido, (void *)mensaje);
    pthread_create(&ruedaDer, NULL, funcion_sensor_encoder_derecha, (void *)mensaje2);
    pthread_create(&ruedaIzq, NULL, funcion_sensor_encoder_izquierda, (void *)mensaje3);

    // pthread_join(ultrasonido, NULL);
    // pthread_join(ruedaDer, NULL);
    // pthread_join(ruedaIzq, NULL);
    Secuencia_Inicio();
    while (1)
    {
        MEF_Modo_Aspiradora();
        MEF_Accion_Modo();
    }
    //printf("SE ALCANZO EL OBSTÁCULO \n");
}

void MEF_Modo_Aspiradora()
{ //Esta es la maquina de estados principal
    switch (estadoActualModo)
    { //Si esta en manual solo se fija si el estado pasa a automatico
    case MANUAL:
        break;
    case AUTOMATICO: //Si esta en automatico se fija si pasa a manual y ejecuta la sub-maquina de estados del estado automatico
        MEF_Automatico();
        break;
    default:
        break;
    }
}

void MEF_Automatico()
{ //Esta es la sub-maquina de estados del estado automatico
    switch (estadoActual)
    {
    case MOVIENDOSE:
        if ((posiciones == 0) || (hayObstaculo))
        {
            distanciaMaxima = 0;
            estadoAnterior = estadoActual;
            estadoActual = ESQUIVANDO;
        }
        break;
    case BARRIDO:
        // printf("Estado: Barrido\n");
        if (rand() % 2 == 0)
        {
            estadoAnterior = estadoActual;
            estadoActual = MIRANDO_DERECHA;
        }
        else
        {
            estadoAnterior = estadoActual;
            estadoActual = MIRANDO_IZQUIERDA;
        }

        break;
    case ESQUIVANDO:
        // printf("Estado: Esquivando\n");
        distancia = 0;
        estadoAnterior = estadoActual;
        if (hayObstaculo)
            estadoActual = BARRIDO;
        else
            estadoActual = MOVIENDOSE;

        break;
    case MIRANDO_DERECHA:
        // printf("Estado: Mirando derecha\n");
        if (flagServo)
        {
            distanciaDerecha = distancia;
            distancia = 0;
            flagServo = 0;
            if (!hayObstaculo)
            {
                if (estadoAnterior == MIRANDO_IZQUIERDA)
                {
                    if ((distanciaIzquierda > 10) && (distanciaDerecha < distanciaIzquierda))
                    {
                        estadoAnterior = estadoActual;
                        estadoActual = GIRANDO_IZQUIERDA;
                        distanciaMaxima = distanciaIzquierda;
                        posiciones = distanciaMaxima / 25;
                    }
                    else
                    {
                        estadoAnterior = estadoActual;
                        estadoActual = GIRANDO_DERECHA;
                        distanciaMaxima = distanciaDerecha;
                        posiciones = distanciaMaxima / 25;
                    }
                }
                else
                {
                    distanciaMaxima = distanciaDerecha;
                    estadoAnterior = estadoActual;
                    estadoActual = MIRANDO_IZQUIERDA;
                }
            }
            else
            {
                if (estadoAnterior != MIRANDO_IZQUIERDA)
                {
                    estadoAnterior = estadoActual;
                    estadoActual = MIRANDO_IZQUIERDA;
                }
                else
                {
                    if (distanciaIzquierda > 10)
                    {
                        distanciaMaxima = distanciaIzquierda;
                        estadoAnterior = estadoActual;
                        estadoActual = GIRANDO_IZQUIERDA;
                    }
                    else
                    {
                        estadoAnterior = estadoActual;
                        estadoActual = GIRANDO_180;
                    }
                }
            }
        }

        break;
    case MIRANDO_IZQUIERDA:
        // printf("Estado: Mirando izquierda\n");
        if (flagServo)
        {
            distanciaIzquierda = distancia;
            distancia = 0;
            flagServo = 0;
            if (!hayObstaculo)
            {
                if (estadoAnterior == MIRANDO_DERECHA)
                {
                    if ((distanciaDerecha > 10) && (distanciaIzquierda < distanciaDerecha))
                    {
                        estadoAnterior = estadoActual;
                        estadoActual = GIRANDO_DERECHA;
                        distanciaMaxima = distanciaDerecha;
                        posiciones = distanciaMaxima / 25;
                    }
                    else
                    {
                        estadoAnterior = estadoActual;
                        estadoActual = GIRANDO_IZQUIERDA;
                        distanciaMaxima = distanciaIzquierda;
                        posiciones = distanciaMaxima / 25;
                    }
                }
                else
                {
                    distanciaMaxima = distanciaIzquierda;
                    estadoAnterior = estadoActual;
                    estadoActual = MIRANDO_DERECHA;
                }
            }
            else
            {
                if (estadoAnterior != MIRANDO_DERECHA)
                {
                    estadoAnterior = estadoActual;
                    estadoActual = MIRANDO_DERECHA;
                }

                else
                {
                    if (distanciaDerecha > 10)
                    {
                        distanciaMaxima = distanciaDerecha;
                        estadoAnterior = estadoActual;
                        estadoActual = GIRANDO_DERECHA;
                    }
                    else
                    {
                        estadoAnterior = estadoActual;
                        estadoActual = GIRANDO_180;
                    }
                }
            }
        }

        break;

    case GIRANDO_IZQUIERDA:
    case GIRANDO_DERECHA:
        if (flagMotor)
        {
            flagMotor = 0;
            estadoAnterior = estadoActual;
            estadoActual = MOVIENDOSE;
        }
        break;
    case GIRANDO_180:
        if (flagMotor)
        {
            flagMotor = 0;
            if (hayObstaculo180)
            {
                estadoAnterior = estadoActual;
                estadoActual = BARRIDO;
            }
            else
            {
                estadoAnterior = estadoActual;
                estadoActual = MOVIENDOSE;
            }
        }
        break;
    default:
        break;
    }
}

void MEF_Accion_Modo()
{
    switch (estadoActualModo)
    {
    case MANUAL:
        usleep(1000000);
        Detener();
        break;
    case AUTOMATICO:
        MEF_Accion_Automatico();
        break;
    default:
        break;
    }
}

void MEF_Accion_Automatico()
{
    switch (estadoActual)
    {
    case MOVIENDOSE:
        MoverAdelante();
        posiciones--;
        hayObstaculo = Observar();
        break;
    case BARRIDO:
        //Detener();
        //usleep(500);
        break;
    case ESQUIVANDO:
        Detener();
        usleep(500);
        //hayObstaculo = Observar();
        break;
    case MIRANDO_DERECHA:
        printf("MIRANDO_DERECHA\n");
        servoMirarDerecha();
        hayObstaculo = Observar();
        // Vuelve a colocar el servomotor al centro
        servoMirarCentro();
        flagServo = 1;
        break;
    case MIRANDO_IZQUIERDA:
        printf("MIRANDO_IZQUIERDA\n");
        servoMirarIzquierda();
        hayObstaculo = Observar();
        // Vuelve a colocar el servomotor al centro
        servoMirarCentro();
        flagServo = 1;
        break;
    case GIRANDO_IZQUIERDA:
        contPrueba = 0;
        motorGirarIzquierda();
        printf("Se generon %d interrupciones\n", contPrueba);
        // Detener();
        flagMotor = 1;
        break;
    case GIRANDO_DERECHA:
        contPrueba = 0;
        motorGirarDerecha();
        printf("Se generon %d interrupciones\n", contPrueba);
        // Detener();
        flagMotor = 1;
        break;
    case GIRANDO_180:
        contPrueba = 0;
        motorGirar180();
        printf("Se generon %d interrupciones\n", contPrueba);
        flagMotor = 1;
        Observar();
        if (distancia > 10)
        {
            hayObstaculo180 = 0;
        }
        else
        {
            hayObstaculo180 = 1;
        }
        // Detener();

        break;
    default:
        break;
    }
}

void *funcion_sensor_ultrasonido(void *ptr)
{
    // CON LAS POSICIONES X E Y Y LA DIRECCION, SABER IS HAY OBSTACULO O NO USANDO LA MATRIZ

    char *mensaje;
    mensaje = (char *)ptr;

    while (1)
    {
        if (trigger == 1)
        {

            // Utiliza la matrix que representa la habitación para calcular la distancia
            int xAux = x;
            int yAux = y;
            printf("x: %d, y: %d \n", xAux, yAux);
            int posicionesLibres = 0;
            // Se hace una cuenta para calcular hacia que lugar de la matriz esta observando
            int direccionTotal = (direccionRobot + direccionServo) % 360;
            switch (direccionTotal)
            {
            // Se revisa cuantas posiciones hay en la matriz libres y se lo multiplica
            // por 25 cm que es el tamaño que representa cada posicion de la matriz
            // El sensor de ultrasonido alcanza como máximo 400cm (16 posiciones).
            case 0:
                // Esta observando  hacia derecha en la matriz
                printf("0 GRADOS\n");
                xAux++;
                while ((habitacion[yAux][xAux] != 1) && (posicionesLibres < 16))
                {
                    posicionesLibres++;
                    xAux++;
                }
                break;
            case 90:
                printf("90 GRADOS\n");
                // Esta observando hacia arriba en la matriz
                yAux--;
                while ((habitacion[yAux][xAux] != 1) && (posicionesLibres < 16))
                {
                    posicionesLibres++;
                    yAux--;
                }
                break;
            case 180:
                printf("180 GRADOS\n");
                // Esta observando hacia izquierda en la matriz
                xAux--;
                while ((habitacion[yAux][xAux] != 1) && (posicionesLibres < 16))
                {
                    posicionesLibres++;
                    xAux--;
                }
                break;
            case 270:
                printf("270 GRADOS\n");
                // Esta observando hacia abajo en la matriz
                yAux++;
                while ((habitacion[yAux][xAux] != 1) && (posicionesLibres < 16))
                {
                    posicionesLibres++;
                    yAux++;
                }
                break;
            default:
                break;
            }
            // Se calcula la distancia = posiciones libres de la matriz * 25 cm
            distancia = posicionesLibres * 25;
            sem_post(&semaforoDistancia);
            trigger = 0;
        }
    }
}

void *funcion_sensor_encoder_derecha(void *ptr)
{
    char *mensaje;
    int cant;
    mensaje = (char *)ptr;
    // printf("%s \n", (char *)ptr);
    while (1)
    {
        //Si la rueda se mueve en alguna direccion se modifica el valor que devuelve el encoder
        if ((movimientoRuedaDerecha == ATRAS) || (movimientoRuedaDerecha == ADELANTE))
        {
            encoderState_derecha = 1 - encoderState_derecha; //Se hace un toggle entre 1 y 0
            if (encoderState_derecha == 0)
                contPrueba++;
            // printf("Rueda derecha: %d\n", encoderState_derecha);
            usleep(8928); //Se espera 0.008928 seg, esto cambiaria segun la velocidad de la rueda
        }
    }
}

void *funcion_sensor_encoder_izquierda(void *ptr)
{
    char *mensaje;
    mensaje = (char *)ptr;
    //printf("%s \n", (char *)ptr);
    while (1)
    {
        //Si la rueda se mueve en alguna direccion se modifica el valor que devuelve el encoder
        if ((movimientoRuedaIzquierda == ATRAS) || (movimientoRuedaIzquierda == ADELANTE))
        {

            encoderState_izquierda = 1 - encoderState_izquierda; //Se hace un toggle entre 1 y 0
            if (encoderState_izquierda == 0)
                contPrueba++;
            // printf("Rueda izquierda: %d\n", encoderState_izquierda);
            usleep(8928); //Se espera 0.008928 seg, esto cambiaria segun la velocidad de la rueda
        }
    }
}

void Detener()
{
    movimientoRuedaIzquierda = QUIETO;
    movimientoRuedaDerecha = QUIETO;
    printf("Detenido\n");
    delay();
}

void MoverAdelante()
{
    movimientoRuedaIzquierda = ADELANTE;
    movimientoRuedaDerecha = ADELANTE;

    switch (direccionRobot)
    {
    case 0:
        printf("CASO 0\n");
        x++;
        MoverPosicion();
        break;

    case 90:
        printf("CASO 90\n");
        y--;
        MoverPosicion();
        break;

    case 180:
        printf("CASO 180\n");
        x--;
        MoverPosicion();
        break;

    case 270:
        printf("CASO 270\n");
        y++;
        MoverPosicion();
        break;

    default:
        break;
    }
    printf("Avanzando\n");
    delay();
}

void MoverPosicion()
{
    int cont = INT_AVANZAR_POSICION;

    movimientoRuedaIzquierda = ADELANTE;
    movimientoRuedaDerecha = ADELANTE;
    //delay();
    contar_interrupciones(cont);

    // Se crea el nodo en el grafo

    printf("El robot avanzó una posicion\n");
    delay();
}

int Observar()
{
    int obs;
    printf("Observando\n");
    delay();
    obs = Ultrasonico_Trigger();
    if (obs)
    {
        printf("Hay obstaculo\n");
        printf("Distancia: %f\n", distancia);
        return 1;
    }
    else
    {
        printf("No hay obstaculo\n");
        printf("Distancia: %f\n", distancia);
        return 0;
    }
    delay();
}

void servoMirarCentro()
{
    printf("Girando servo al centro\n");
    delay();
    printf("Servo girado al centro\n");
    delay();
    direccionServo = 0;
}

void servoMirarDerecha()
{
    printf("Girando servo a derecha\n");
    delay();
    printf("Servo girado a derecha\n");
    delay();
    direccionServo = 270;
}

void servoMirarIzquierda()
{
    printf("Girando servo a izquierda\n");
    delay();
    printf("Servo girado a izquierda\n");
    delay();
    direccionServo = 90;
}

void motorGirarIzquierda()
{
    int cont = INT_GIRO_90;
    printf("Girando robot a izquierda\n");
    movimientoRuedaIzquierda = ATRAS;
    movimientoRuedaDerecha = QUIETO;
    //delay();
    contar_interrupciones(cont);
    movimientoRuedaIzquierda = QUIETO;
    movimientoRuedaDerecha = QUIETO;
    printf("Robot girado a izquierda\n");
    direccionRobot = (direccionRobot + 90) % 360;
    delay();
}

void motorGirarDerecha()
{
    int cont = INT_GIRO_90;
    printf("Girando robot a derecha\n");
    movimientoRuedaIzquierda = ADELANTE;
    movimientoRuedaDerecha = QUIETO;
    //delay();
    contar_interrupciones(cont);
    movimientoRuedaIzquierda = QUIETO;
    movimientoRuedaDerecha = QUIETO;
    printf("Robot girado a derecha\n");
    direccionRobot = (direccionRobot + 270) % 360;
    delay();
}

void motorGirar180()
{
    printf("Girando robot 180º\n");
    int cont = INT_GIRO_180;
    movimientoRuedaIzquierda = ADELANTE;
    movimientoRuedaDerecha = QUIETO;
    contar_interrupciones(cont);
    //delay();
    printf("Robot girado 180º\n");
    direccionRobot = (direccionRobot + 180) % 360;
    delay();
}

void contar_interrupciones(int cont)
{
    while (cont != 0)
    {
        if (encoderState_izquierda != encoderState_izquierda_anterior)
        {
            if (encoderState_izquierda == 0)
            {
                cont--;
            }
            encoderState_izquierda_anterior = encoderState_izquierda;
        }
    }
}

void Secuencia_Inicio(void)
{
    char direccionMayorDistancia;
    vertice v;
    coordenadas coordenadasAdy;
    nodo * inicial;
    nodo * adyacente;

    servoMirarCentro();

    // Se crea el vertice inicial
    v.coordenadas.x = x;
    v.coordenadas.y = y;
    v.estado = Visitado;
    // Se agrega el vertice inicial al grafo
    inicial = agregar_vertice(&grafoMapa, v);

    hayObstaculo = Observar();
    distanciaCentro = distancia;
    distancia = 0;
    distanciaMaxima = distanciaCentro;
    direccionMayorDistancia = 'C';

    // Se crea el vertice con el servomotor mirando al centro
    v.coordenadas = coordenadasAyacente(x,y);
    if(hayObstaculo){
        v.estado = Inaccesible;
    }
    else{
        v.estado = NoVisitado;
    }
    // Se agrega al grafo
    adyacente = agregar_vertice(&grafoMapa,v);
    // Se agregan como vertices adyacentes bidireccionalmente con el inicial
    agregar_adyacente(inicial,adyacente,direccionAdyacente());

    servoMirarDerecha();
    hayObstaculo = Observar();
    distanciaDerecha = distancia;
    distancia = 0;
    if (distanciaDerecha > distanciaMaxima)
    {
        distanciaMaxima = distanciaDerecha;
        direccionMayorDistancia = 'D';
    }

    // Se crea el vertice con el servomotor mirando a la derecha
    v.coordenadas = coordenadasAyacente(x,y);
    if(hayObstaculo){
        v.estado = Inaccesible;
    }
    else{
        v.estado = NoVisitado;
    }
    // Se agrega al grafo
    adyacente = agregar_vertice(&grafoMapa,v);
    // Se agregan como vertices adyacentes bidireccionalmente con el inicial
    agregar_adyacente(inicial,adyacente,direccionAdyacente());

    servoMirarIzquierda();
    hayObstaculo = Observar();
    distanciaIzquierda = distancia;
    distancia = 0;
    if (distanciaIzquierda > distanciaMaxima)
    {
        distanciaMaxima = distanciaIzquierda;
        direccionMayorDistancia = 'I';
    }

    // Se crea el vertice con el servomotor mirando a la izquierda
    v.coordenadas = coordenadasAyacente(x,y);
    if(hayObstaculo){
        v.estado = Inaccesible;
    }
    else{
        v.estado = NoVisitado;
    }
    // Se agrega al grafo
    adyacente = agregar_vertice(&grafoMapa,v);
    // Se agregan como vertices adyacentes bidireccionalmente con el inicial
    agregar_adyacente(inicial,adyacente,direccionAdyacente());

    switch (direccionMayorDistancia)
    {
    case 'C':
        estadoActual = MOVIENDOSE;
        break;

    case 'D':
        estadoActual = GIRANDO_DERECHA;
        break;

    case 'I':
        estadoActual = GIRANDO_IZQUIERDA;
        break;
    }
    servoMirarCentro();

    printf("IMPRIMIMOS EL GRAFO A VER COMO ESTA\n");
    printf("===================================\n");
    imprimir_grafo(grafoMapa);


}

void delay(void)
{
    usleep(3000000);
}

coordenadas coordenadasAyacente(int xAux, int yAux) 
{
    coordenadas coordenadasAdyacente; 
    int direccionTotal = (direccionRobot + direccionServo) % 360;
    switch (direccionTotal)
    {
    case 0:
        coordenadasAdyacente.x = xAux + 1;
        coordenadasAdyacente.y = yAux;
        break;
    case 90:
        coordenadasAdyacente.x = xAux;
        coordenadasAdyacente.y = yAux - 1;
        break;
    case 180:
        coordenadasAdyacente.x = xAux - 1;
        coordenadasAdyacente.y = yAux;
        break;
    case 270:
        coordenadasAdyacente.x = xAux;
        coordenadasAdyacente.y = yAux +1;
        break;
    default:
        break;
    }
    
    return coordenadasAdyacente;
}

direccion direccionAdyacente(){
    int direccion;
    int direccionTotal = (direccionRobot + direccionServo) % 360;
    
    // Se toma la direccion total que puede ser 0, 90, 180 o 270 y se la divide por 90
    // para que devuelva un valor de 0 a 3 que represente la direccion;
    direccion = direccionTotal / 90;
    return direccion;
}