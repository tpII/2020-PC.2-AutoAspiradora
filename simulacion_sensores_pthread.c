#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> //Este es para el usleep
#include <semaphore.h>

// Distancias máximas y mínimas que puede medir el sensor hc-sr04 en mm
#define DIST_MIN 2
#define DIST_MAX 4000

#define INT_GIRO_90 900 //Cantidad de interrupciones para hacer un giro de 90 grados
#define INT_GIRO_180 1800 //Cantidad de interrupciones para hacer un giro de 180 grados

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
int Observar();
void servoMirarDerecha();
void servoMirarIzquierda();
void motorGirarIzquierda();
void motorGirarDerecha();
void motorGirar180();
void contar_interrupciones_giro(int cont);

char direccion = 'C';
double distancia = 150;
int encoderState_derecha = 0; //Indica el valor que devuelve el encoder derecho
int encoderState_derecha_anterior = 0;
int encoderState_izquierda = 0; //Indica el valor que devuelve el encoder izquierda
int encoderState_izquierda_anterior = 0;
int robotEncendido = 0; //Booleano
int trigger = 0;        //triger del ultrasonido
int echo;               //echo del ultrasonido
int distanciaDerecha; //Distancia medida por el ultrasonico a la derecha
int distanciaIzquierda; //Distancia medida por el ultrasonico a la izquierda
int distanciaCentro; //Distancia medida por el ultrasonico al centro

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

    estadoActual = MOVIENDOSE;
    estadoActualModo = AUTOMATICO;

    // while ((distancia > 10) && (ret == 0))
    // {
    //     ret = pthread_create(&ultrasonido, NULL, funcion_sensor_ultrasonido, (void *)mensaje);

    //     pthread_join(ultrasonido, NULL);

    //     printf("Distancia al obstáculo: %f \n", distancia);
    // }
    // while (1)
    // { //Habria que cambiar la condicion
    //     pthread_create(&ruedaDer, NULL, funcion_sensor_encoder_derecha, (void *)mensaje2);
    //     pthread_create(&ruedaIzq, NULL, funcion_sensor_encoder_izquierda, (void *)mensaje3);
    //     pthread_join(ruedaDer, NULL);
    //     pthread_join(ruedaIzq, NULL);
    //     printf("PROGRAMA PRINCIPAL: %d \n", cant);
    // }

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
        //printf("Estado: Moviendose, obstaculo=%d\n", hayObstaculo);
        if (hayObstaculo)
        {
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
            distanciaDerecha=distancia;
            distancia=0;
            flagServo = 0;
            if (!hayObstaculo)
            {
                if (estadoAnterior == MIRANDO_IZQUIERDA)
                {
                    if ((distanciaIzquierda > 10) && (distanciaDerecha < distanciaIzquierda)){
                        estadoAnterior = estadoActual;
                        estadoActual = GIRANDO_IZQUIERDA;

                    }
                    else{
                        estadoAnterior = estadoActual;
                        estadoActual = GIRANDO_DERECHA;
                    }
                }
                else{
                    estadoAnterior=estadoActual;
                    estadoActual=MIRANDO_IZQUIERDA;
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
                    estadoAnterior = estadoActual;
                    estadoActual = GIRANDO_180;
                }
            }
        }

        break;
    case MIRANDO_IZQUIERDA:
        // printf("Estado: Mirando izquierda\n");
        if (flagServo)
        {   
            distanciaIzquierda=distancia;
            distancia=0;
            flagServo = 0;
            if (!hayObstaculo)
            {
                if (estadoAnterior == MIRANDO_DERECHA)
                {
                    if ((distanciaDerecha > 10) && (distanciaIzquierda < distanciaDerecha)){
                        estadoAnterior = estadoActual;
                        estadoActual = GIRANDO_DERECHA;

                    }
                    else{
                        estadoAnterior = estadoActual;
                        estadoActual = GIRANDO_IZQUIERDA;
                    }
                }
                else{
                    estadoAnterior=estadoActual;
                    estadoActual=MIRANDO_DERECHA;
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
                    estadoAnterior = estadoActual;
                    estadoActual = GIRANDO_180;
                }
            }
        }

        break;

    case GIRANDO_IZQUIERDA:
    case GIRANDO_DERECHA:
    case GIRANDO_180:
        if (flagMotor)
        {
            flagMotor = 0;
            estadoAnterior = estadoActual;
            estadoActual = MOVIENDOSE;
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
        servoMirarDerecha();
        hayObstaculo = Observar();
        flagServo = 1;
        break;
    case MIRANDO_IZQUIERDA:
        servoMirarIzquierda();
        hayObstaculo = Observar();
        flagServo = 1;
        break;
    case GIRANDO_IZQUIERDA:
        contPrueba=0;
        motorGirarIzquierda();
        printf("Se generon %d interrupciones\n", contPrueba);
        // Detener();
        flagMotor = 1;
        break;
    case GIRANDO_DERECHA:
        contPrueba=0;
        motorGirarDerecha();
        printf("Se generon %d interrupciones\n", contPrueba);
        // Detener();
        flagMotor = 1;
        break;
    case GIRANDO_180:
        contPrueba=0;
        motorGirar180();
        printf("Se generon %d interrupciones\n", contPrueba);
        // Detener();
        flagMotor = 1;
        break;
    default:
        break;
    }
}

void *funcion_sensor_ultrasonido(void *ptr)
{
    char *mensaje;
    mensaje = (char *)ptr;
    //printf("%s \n", (char *)ptr);

    while (1)
    {
        if (trigger == 1)
        {
            // Interpreta que todavía no se calculo la distancia en esta direccion
            if (distancia == 0)
            {
                // Numero aleatorio en cm, con un decimal. En el rango de 2 a 4000mm
                distancia = (double)((rand() % (DIST_MAX - DIST_MIN + 1)) + DIST_MIN) / 10;
            }
            // Interpreta que el auto se está moviendo y calculando la distancia
            else
            {
                // Numero aleatorio en el rango de 2 al valor de distancia
                distancia = (double)((rand() % ((int)distancia * 10 - DIST_MIN + 1)) + DIST_MIN) / 10;
            }

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
            if (encoderState_derecha==0)
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
    printf("%s \n", (char *)ptr);
    while (1)
    {
        //Si la rueda se mueve en alguna direccion se modifica el valor que devuelve el encoder
        if ((movimientoRuedaIzquierda == ATRAS) || (movimientoRuedaIzquierda == ADELANTE))
        {        
                                                             
            encoderState_izquierda = 1 - encoderState_izquierda; //Se hace un toggle entre 1 y 0
            if (encoderState_izquierda==0)
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
    usleep(2000000);
}

void MoverAdelante()
{
    movimientoRuedaIzquierda = ADELANTE;
    movimientoRuedaDerecha = ADELANTE;
    printf("Avanzando\n");
    usleep(2000000);
}

int Observar()
{
    int obs;
    printf("Observando\n");
    usleep(2000000);
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
    usleep(2000000);
}

void servoMirarDerecha()
{
    printf("Girando servo a derecha\n");
    usleep(2000000);
    printf("Servo girado a derecha\n");
    usleep(2000000);
}

void servoMirarIzquierda()
{
    printf("Girando servo a izquierda\n");
    usleep(2000000);
    printf("Servo girado a izquierda\n");
    usleep(2000000);
}

void motorGirarIzquierda()
{   
    int cont=INT_GIRO_90;
    printf("Girando robot a izquierda\n");
    movimientoRuedaIzquierda = ATRAS;
    movimientoRuedaDerecha = QUIETO;
    //usleep(2000000);
    contar_interrupciones_giro(cont);
    movimientoRuedaIzquierda = QUIETO;
    movimientoRuedaDerecha = QUIETO;
    printf("Robot girado a izquierda\n");
    usleep(2000000);
}

void motorGirarDerecha()
{   
    int cont=INT_GIRO_90;
    printf("Girando robot a derecha\n");
    movimientoRuedaIzquierda = ADELANTE;
    movimientoRuedaDerecha = QUIETO;
    //usleep(2000000);
    contar_interrupciones_giro(cont);
    movimientoRuedaIzquierda = QUIETO;
    movimientoRuedaDerecha = QUIETO;
    printf("Robot girado a derecha\n");
    usleep(2000000);
}

void motorGirar180()
{   
    printf("Girando robot 180º\n");
    int cont=INT_GIRO_180;
    movimientoRuedaIzquierda = ADELANTE;
    movimientoRuedaDerecha = QUIETO;
    contar_interrupciones_giro(cont);
    //usleep(2000000);
    printf("Robot girado 180º\n");
    usleep(2000000);
}

void contar_interrupciones_giro(int cont){
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