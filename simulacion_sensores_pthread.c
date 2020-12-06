#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <curl/curl.h>
#include <unistd.h> //Este es para el usleep
#include <semaphore.h>
#include "include/grafo.h"
#include "include/comunicacion_simulacion_servidor.h"

// Distancias máximas y mínimas que puede medir el sensor hc-sr04 en mm
#define DIST_MIN 2
#define DIST_MAX 4000

#define INT_GIRO_90 30          //Cantidad de interrupciones para hacer un giro de 90 grados
#define INT_GIRO_180 30         //Cantidad de interrupciones para hacer un giro de 180 grados
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

typedef enum //Indica la direccion en que debe moverse en estado manual
{
    AVANZAR,
    RETROCEDER,
    GIRAR_IZQUIERDA,
    GIRAR_DERECHA,
    NADA
} accion_manual;

typedef enum
{
    MOVIENDOSE,
    BARRIDO,
    ESQUIVANDO,
    MIRANDO_IZQUIERDA,
    MIRANDO_DERECHA,
    GIRANDO_DERECHA,
    GIRANDO_IZQUIERDA,
    GIRANDO_180,
    DETENIDO
} estado_automatico;

void MEF_Modo_Aspiradora();
void MEF_Automatico();
void MEF_Manual();
nodo *MEF_Accion_Modo(nodo *);
nodo *MEF_Accion_Automatico(nodo *);
void MEF_Accion_Manual();

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
nodo *Secuencia_Inicio(void);
nodo *crearVertice(nodo *);
void delay(void);
coordenadas coordenadasAyacente(int, int);
direccion direccionAdyacente();
direccion direccionAdyacenteAnterior();
void retorna(estado_automatico estadoRecursion, nodo *actual, nodo *anterior);
void recursion(nodo *actual, nodo *anterior);
void termino_tramo(nodo *actual);

// En esta variable se guarda el estado que llega del servidor
struct estadoRobot estadoRobot;

// Matriz que representa la habitación
// 0 -> lugar accesible, 1 -> obstáculo
int habitacion[10][10] =
    {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 1, 1, 1, 1, 1, 0, 0, 1},
        {1, 0, 0, 0, 1, 1, 1, 0, 0, 1},
        {1, 0, 1, 0, 1, 1, 1, 0, 0, 1},
        {1, 0, 0, 0, 1, 1, 1, 0, 0, 1},
        {1, 1, 1, 1, 1, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 0, 0, 1, 1, 0, 1},
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
int retornando;
int iniciaRecorrido = 0; //Flag que indica si se salio de la secuencia de inicio y comenzo el recorrido
int distanciaTramo;
int velocidadTramo;
vertice verticeInicioTramo;
char *nombreHabitacion = "Hola";

CURL *curl;

movimiento movimientoRuedaDerecha = ATRAS;
movimiento movimientoRuedaIzquierda = ADELANTE;

//Variables de las maquinas de estado
int hayObstaculo;
estado_automatico estadoAnterior;
estado_automatico estadoActual;
estado_automatico estadoActualModo;

int flagServo;
int flagMotor;
int flagAvanzo = 0;

char *url_info_grafo = "http://localhost:3000/api/robotAspiradora/grafos";
char *url_vertices = "http://localhost:3000/api/robotAspiradora/vertices";
char *url_datos_recorridos = "http://localhost:3000/api/robotAspiradora/dato";
char *url_consultar_estado = "http://localhost:3000/api/robotAspiradora/consultaEstado";
char *url_consultar_habitacion = "http://localhost:3000/api/robotAspiradora/consultaNombre";

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

    struct estadoRobot estadoAnterior;

    // Se inicializa la variable para comunicarnos con el servidor
    curl = configurar_conexion();
    cerrar_conexion(curl);
    curl = configurar_conexion();
    nodo *inicial;

    ret = pthread_create(&ultrasonido, NULL, funcion_sensor_ultrasonido, (void *)mensaje);
    pthread_create(&ruedaDer, NULL, funcion_sensor_encoder_derecha, (void *)mensaje2);
    pthread_create(&ruedaIzq, NULL, funcion_sensor_encoder_izquierda, (void *)mensaje3);

    printf("La aspiradora entra en funcionamiento\n");
    consultar_estado_robot(curl, url_consultar_estado, &estadoRobot);
    if (estadoRobot.modoAutomatico)
    {
        printf("El robot está en modo automático\n");
        estadoActual = BARRIDO;
        estadoActualModo = AUTOMATICO;
    }
    else
    {
        printf("El robot está en modo manual\n");
        estadoActual = DETENIDO;
        estadoActualModo = MANUAL;
    }
    // pthread_join(ultrasonido, NULL);
    // pthread_join(ruedaDer, NULL);
    // pthread_join(ruedaIzq, NULL);

    // While principal
    while (1)

    {

        inicializar_grafo(&grafoMapa, "HolaHabitacion");
        printf("debug4\n");
        inicial = Secuencia_Inicio();
        iniciaRecorrido = 1;                  //Se indica que salio de la secuencia de inicio y se inicia el recorrido;
        verticeInicioTramo = inicial->actual; //Se guarda el vertice de inicio del tramo
        distanciaTramo = 0;                   //Se inicializa la distancia del tramo
        recursion(inicial, NULL);
        printf("Mapeado de la habitación finalizado\n");
        estadoRobot.mapeando = 0;
        consultar_estado_robot(curl, url_consultar_estado, &estadoRobot);
        if (estadoRobot.ventiladorEncendido != estadoAnterior.ventiladorEncendido)
        {
            if (estadoRobot.ventiladorEncendido == 1)
            {
                printf("Robot comienza a aspirar\n");
            }
            else
            {
                printf("Robot deja de aspirar\n");
            }
        }
        if (estadoRobot.modoAutomatico != estadoAnterior.modoAutomatico)
        {
            if (estadoRobot.modoAutomatico == 1)
            {
                estadoActual = BARRIDO;
                estadoActualModo = AUTOMATICO;
                printf("PASA A MODO AUTOMATICO\n");
            }
            else
            {
                printf("PASA A MODO MANUAL\n");
                estadoActualModo = MANUAL;
                estadoActual = DETENIDO;
            }
        }

        if (!estadoRobot.modoAutomatico)
        {
            MEF_Modo_Aspiradora();
            MEF_Accion_Modo(NULL);
        }

        if ((estadoRobot.modoAutomatico) && (estadoRobot.mapeando))
        {
            if (estadoRobot.mapeando)
            {
                //consultar_nombre_habitacion(curl, url_consultar_habitacion, nombreHabitacion);
                printf("PASA A MODO AUTOMATICO -> MAPEANDO\n");
                printf("%s", nombreHabitacion);
            }
        }
        estadoAnterior = estadoRobot;
        usleep(1000000);
    }
    // MEF_Modo_Aspiradora();
    // MEF_Accion_Modo(inicial);

    //printf("SE ALCANZO EL OBSTÁCULO \n");

    // Se calculan las dimensiones del mapa
    calculo_dimensiones_mapa(&grafoMapa);

    // Se envía la información del grafo al servidor
    enviar_info_grafo(curl, grafoMapa, url_info_grafo);
}

void MEF_Modo_Aspiradora()
{ //Esta es la maquina de estados principal
    switch (estadoActualModo)
    { //Si esta en manual solo se fija si el estado pasa a automatico
    case MANUAL:
        MEF_Manual();
        break;
    case AUTOMATICO: //Si esta en automatico se fija si pasa a manual y ejecuta la sub-maquina de estados del estado automatico
        MEF_Automatico();
        break;
    default:
        break;
    }
}

void MEF_Manual()
{
    //Esta es la sub-máquina de estados del estado manual
    switch (estadoActual)
    {
    case MOVIENDOSE:
        if (flagAvanzo)
        {
            flagAvanzo = 0;
            estadoActual = DETENIDO;
        }
        break;
    case GIRANDO_DERECHA:
        if (flagMotor)
        {
            flagMotor = 0;
            estadoActual = DETENIDO;
        }
        break;
    case GIRANDO_IZQUIERDA:
        if (flagMotor)
        {
            flagMotor = 0;
            estadoActual = DETENIDO;
        }
        break;
    case GIRANDO_180:
        if (flagMotor)
        {
            flagMotor = 0;
            estadoActual = DETENIDO;
        }
        break;
    case DETENIDO:
        switch (estadoRobot.direccionManual)
        {
        case AVANZAR:
            estadoActual = MOVIENDOSE;
            break;
        case RETROCEDER:
            estadoActual = GIRANDO_180;
            break;
        case GIRAR_IZQUIERDA:
            estadoActual = GIRANDO_IZQUIERDA;
            break;
        case GIRAR_DERECHA:
            estadoActual = GIRANDO_DERECHA;
            break;
        case NADA:
            estadoActual = DETENIDO;
            break;
        default:
            break;
        }
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
        retornando = 1;
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
                //estadoActual = MOVIENDOSE;
                estadoActual = 40;
            }
        }
        break;
    default:
        break;
    }
}

nodo *MEF_Accion_Modo(nodo *inicial)
{
    switch (estadoActualModo)
    {
    case MANUAL:
        MEF_Accion_Manual();
        break;
    case AUTOMATICO:
        return MEF_Accion_Automatico(inicial);
        break;
    default:
        break;
    }
}

void MEF_Accion_Manual()
{
    switch (estadoActual)
    {
    case MOVIENDOSE:
        // Se simula la activacion de ambos motores
        movimientoRuedaIzquierda = ADELANTE;
        movimientoRuedaDerecha = ADELANTE;
        printf("El robot avanzo 25 cm\n");
        flagAvanzo = 1;
        break;
    case GIRANDO_DERECHA:
        motorGirarDerecha();
        flagMotor = 1;
        break;
    case GIRANDO_IZQUIERDA:
        motorGirarIzquierda();
        flagMotor = 1;
        break;
    case GIRANDO_180:
        motorGirar180();
        flagMotor = 1;
        break;
    case DETENIDO:
        // Se simula la detención de los motores
        movimientoRuedaIzquierda = QUIETO;
        movimientoRuedaDerecha = QUIETO;

        break;
    default:
        break;
    }
}

nodo *MEF_Accion_Automatico(nodo *actual)
{
    vertice v;
    nodo *proximo = NULL;
    int posiblesCaminos = 4;

    switch (estadoActual)
    {
    case MOVIENDOSE:
        MoverAdelante();
        posiciones--;
        actual->actual.estado = Visitado;
        //enviar_vertices_grafo(curl, actual->actual, grafoMapa.nombre, url_vertices);
        hayObstaculo = Observar(actual);
        // Si hay obstaculo vuelve con el mismo vertice y si no hay obstáculo devuelve el próximo vertice
        if (!hayObstaculo)
        {
            proximo = actual->adyacentes[direccionAdyacente()];
        }
        else
        {
            proximo = actual;
        }

        break;
    case BARRIDO:
        //Detener();
        //usleep(500);
        proximo = actual;
        break;
    case ESQUIVANDO:
        Detener();
        usleep(500);
        proximo = actual;
        //hayObstaculo = Observar();
        break;
    case MIRANDO_DERECHA:
        printf("MIRANDO_DERECHA\n");
        servoMirarDerecha();
        hayObstaculo = Observar(actual);
        proximo = actual;
        // Vuelve a colocar el servomotor al centro
        servoMirarCentro();
        flagServo = 1;
        break;
    case MIRANDO_IZQUIERDA:
        printf("MIRANDO_IZQUIERDA\n");
        servoMirarIzquierda();
        hayObstaculo = Observar(actual);
        proximo = actual;
        // Vuelve a colocar el servomotor al centro
        servoMirarCentro();
        flagServo = 1;
        break;
    case GIRANDO_IZQUIERDA:
        contPrueba = 0;
        motorGirarIzquierda();
        printf("Se generon %d interrupciones\n", contPrueba);
        proximo = actual->adyacentes[direccionAdyacente()];
        // Detener();
        flagMotor = 1;
        break;
    case GIRANDO_DERECHA:
        contPrueba = 0;
        motorGirarDerecha();
        printf("Se generon %d interrupciones\n", contPrueba);
        proximo = actual->adyacentes[direccionAdyacente()];
        // Detener();
        flagMotor = 1;
        break;
    case GIRANDO_180:
        contPrueba = 0;
        motorGirar180();
        printf("Se generon %d interrupciones\n", contPrueba);
        flagMotor = 1;
        hayObstaculo = Observar(actual);
        if (!hayObstaculo)
        {
            hayObstaculo180 = 0;
            // Si no hay obstaculo pasamos el vertice anterior
            proximo = actual->adyacentes[direccionAdyacente()];
            // if (proximo->actual.estado == Visitado)
            // {
            //     return;
            // }
        }
        else
        {
            hayObstaculo180 = 1;
            proximo = actual;
        }
        // Detener();
        retornando = 1;
        break;
    default:
        break;
    }

    int i;
    // for(i = 0; i < 4; i++) {
    //     if(actual->adyacentes[i] != Visitado)
    //     MEF_Modo_Aspiradora();
    //     MEF_Accion_Modo(proximo);
    //     posiblesCaminos--;
    // }
    // MEF_Modo_Aspiradora();
    // MEF_Accion_Modo(proximo);
    return proximo;
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
    // Incrementa la distancia en 25cm;
    distanciaTramo += 25;
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

int Observar(nodo *actual)
{
    int visitado = 0;
    int obs;
    printf("Observando\n");
    delay();
    obs = Ultrasonico_Trigger();
    if (actual->adyacentes[direccionAdyacente()] != NULL)
    {
        if (actual->adyacentes[direccionAdyacente()]->actual.estado == Visitado)
        {
            obs = 1;
        }
    }
    else
    {
        // Si el vertice no existe, lo crea
        crearVertice(actual);
        printf("creo el vertice\n");
    }

    if (obs)
    {
        printf("Hay obstaculo\n");
        printf("Distancia: %f\n", distancia);
        // Si el servo esta mirando al centro ,no esta en la posicion inicial y no es un
        // vertice visidado (inicio recorrido) quiere decir que estaba avanzando
        // y encontró un obstáculo, por lo que lo envía al servidor
        if ((direccionServo == 0) && (iniciaRecorrido) &&
            ((verticeInicioTramo.coordenadas.x != actual->actual.coordenadas.x) ||
             (verticeInicioTramo.coordenadas.y != actual->actual.coordenadas.y)))
        {
            // Envia los datos del tramo realizado al servidor
            termino_tramo(actual);
        }
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

nodo *Secuencia_Inicio(void)
{
    char direccionMayorDistancia;
    vertice v;
    coordenadas coordenadasAdy;
    nodo *inicial;
    nodo *proximo;
    nodo *adyacenteCentro;
    nodo *adyacenteDerecha;
    nodo *adyacenteIzquierda;

    servoMirarCentro();

    // Se crea el vertice inicial
    v.coordenadas.x = x;
    v.coordenadas.y = y;
    v.estado = Visitado;
    // Se agrega el vertice inicial al grafo
    printf("debug5\n");
    inicial = agregar_vertice(&grafoMapa, v);
    printf("debug6\n");
    // Se incrementa la cantidad de vertices del grafo
    grafoMapa.vertices++;
    // Se envía el vertice al servidor
    printf("%s\n", grafoMapa.nombre);
    printf("debug6.5\n");
    //enviar_vertices_grafo(curl, inicial->actual, grafoMapa.nombre, url_vertices);
    printf("debug7\n");
    hayObstaculo = Observar(inicial);
    distanciaCentro = distancia;
    distancia = 0;
    distanciaMaxima = distanciaCentro;
    direccionMayorDistancia = 'C';

    servoMirarDerecha();
    hayObstaculo = Observar(inicial);
    distanciaDerecha = distancia;
    distancia = 0;
    if (distanciaDerecha > distanciaMaxima)
    {
        distanciaMaxima = distanciaDerecha;
        direccionMayorDistancia = 'D';
    }

    servoMirarIzquierda();
    hayObstaculo = Observar(inicial);
    distanciaIzquierda = distancia;
    distancia = 0;
    if (distanciaIzquierda > distanciaMaxima)
    {
        distanciaMaxima = distanciaIzquierda;
        direccionMayorDistancia = 'I';
    }

    switch (direccionMayorDistancia)
    {
    case 'C':
        estadoActual = MOVIENDOSE;
        //proximo = adyacenteCentro;
        break;

    case 'D':
        estadoActual = GIRANDO_DERECHA;
        //proximo = adyacenteDerecha;
        break;

    case 'I':
        estadoActual = GIRANDO_IZQUIERDA;
        //proximo = adyacenteIzquierda;
        break;
    }
    servoMirarCentro();

    printf("IMPRIMIMOS EL GRAFO A VER COMO ESTA\n");
    printf("===================================\n");

    return inicial;
}

void delay(void)
{
    usleep(20);
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
        coordenadasAdyacente.y = yAux + 1;
        break;
    default:
        break;
    }

    return coordenadasAdyacente;
}

direccion direccionAdyacente()
{
    int direccion;
    int direccionTotal = (direccionRobot + direccionServo) % 360;

    // Se toma la direccion total que puede ser 0, 90, 180 o 270 y se la divide por 90
    // para que devuelva un valor de 0 a 3 que represente la direccion;
    direccion = direccionTotal / 90;
    return direccion;
}

direccion direccionAdyacenteAnterior()
{
    int direccion;
    int direccionTotal = (direccionRobot + direccionServo) % 360;

    switch (direccionTotal)
    {
    case 0:
        return 2;
        break;
    case 90:
        return 3;
        break;
    case 180:
        return 0;
        break;
    case 270:
        return 1;
        break;
    }
}

nodo *crearVertice(nodo *actual)
{
    vertice v;
    nodo *adyacente;
    int hayObstaculo;
    // Se crea el proximo vertice
    v.coordenadas = coordenadasAyacente(x, y);
    if (hayObstaculo)
    {
        v.estado = Inaccesible;
    }
    else
    {
        v.estado = NoVisitado;
    }

    if ((actual->adyacentes[direccionAdyacente()]) == NULL)
    {
        // Se agrega al grafo
        adyacente = agregar_vertice(&grafoMapa, v);
        // Se buscan todos los vertices adyacentes al vertice creado, que ya existan
        buscar_y_agregar_adyacentes(adyacente, grafoMapa);
        // Se incrementa la cantidad de vertices del grafo
        grafoMapa.vertices++;
        // Se envía el vertice al servidor sólo si es un obstáculo, si no se espera a que se visite
        // para enviarlo
        if (hayObstaculo)
        {
            //enviar_vertices_grafo(curl, adyacente->actual, grafoMapa.nombre, url_vertices);
        }
    }
    else
    {
        adyacente = actual->adyacentes[direccionAdyacente()];
    }

    // Se imprime el grafo para ver como queda
    imprimir_grafo(grafoMapa);

    return adyacente;
}

void recursion(nodo *actual, nodo *anterior)
{

    nodo *proximo = NULL;
    estado_automatico estadoRecursion = estadoActual;
    int i;

    printf("LLAMADA A LA RECURSION CON NODO ACTUAL: X: %d Y: %d Y ESTADO: %d\n", actual->actual.coordenadas.x, actual->actual.coordenadas.y, estadoRecursion);

    proximo = MEF_Accion_Modo(actual);
    MEF_Modo_Aspiradora();
    if (!retornando)
    {
        // Si el proximo vertice == actual, se envia el actual y anterior, para que no se pierda el anterior
        if (proximo == actual)
        {
            recursion(actual, anterior);
        }
        else
        {
            recursion(proximo, actual);
        }
        printf("Volvio a la recursion que tiene x actual: %d y actual: %d y estado: %d\n", actual->actual.coordenadas.x, actual->actual.coordenadas.y, estadoRecursion);
    }
    if (retornando)
    {
        if (((estadoRecursion == GIRANDO_DERECHA) || (estadoRecursion == GIRANDO_IZQUIERDA) || (estadoRecursion == MOVIENDOSE)) && (actual != NULL))
        {
            for (i = 0; i < 2; i++)
            {
                switch (i)
                {
                case 0:
                    printf("MIRANDO_DERECHA FOR\n");
                    servoMirarDerecha();
                    hayObstaculo = Observar(actual);
                    // Vuelve a colocar el servomotor al centro
                    servoMirarCentro();
                    if (!hayObstaculo)
                    {
                        retornando = 0;
                        estadoActual = GIRANDO_DERECHA;
                        recursion(actual, anterior);
                    }
                    break;
                case 1:
                    printf("MIRANDO_IZQUIERDA FOR\n");
                    servoMirarIzquierda();
                    hayObstaculo = Observar(actual);
                    // Vuelve a colocar el servomotor al centro
                    servoMirarCentro();
                    if (!hayObstaculo)
                    {
                        retornando = 0;
                        estadoActual = GIRANDO_IZQUIERDA;
                        recursion(actual, anterior);
                    }
                    break;
                default:
                    break;
                }
            }
            retorna(estadoRecursion, actual, anterior);
        }
    }
    printf("Vuelve de la recursion con:\n");
    printf("X: %d Y: %d\n", actual->actual.coordenadas.x, actual->actual.coordenadas.y);
    printf("x real: %d, y real: %d\n", x, y);
    printf("Estado Recursion: %d \n", estadoRecursion);
    return;
}

void retorna(estado_automatico estadoRecursion, nodo *actual, nodo *anterior)
{
    printf("RETORNANDO: ");
    int xAux;
    int yAux;
    switch (estadoRecursion)
    {
    // Si el robot estaba moviendose, al volver de la recursión debe moverse en
    // la nueva dirección, que será la opuesta a la que tenía.
    case MOVIENDOSE:
        printf("Mueve adelante\n");
        MoverAdelante();
        break;
    // Si el robot estaba haciendo un giro a derecha o izquierda, al volver de la recursion
    // debe realizar el giro hacia la direccion del nodo anterior
    case GIRANDO_IZQUIERDA:
    case GIRANDO_DERECHA:
        // Si anterior == NULL, es el nodo inicial y debe terminar la recursion
        if (anterior != NULL)
        {
            xAux = actual->actual.coordenadas.x - anterior->actual.coordenadas.x;
            yAux = actual->actual.coordenadas.y - anterior->actual.coordenadas.y;

            // Se determina la direccion de retorno utilizando el nodo actual y el anterior
            if (yAux == 0)
            {
                // Si xAux > 0 debe ir hacia la izquierda en la matriz
                if (xAux > 0)
                {
                    while ((direccionRobot % 360) != 180)
                    {
                        contPrueba = 0;
                        motorGirarIzquierda();
                    }
                }
                // Si xAux < 0 debe ir hacia la derecha en la matriz
                else
                {
                    while ((direccionRobot % 360) != 0)
                    {
                        contPrueba = 0;
                        motorGirarIzquierda();
                    }
                }
            }
            else
            {
                // Si yAux > 0 debe ir hacia arriba en la matriz
                if (yAux > 0)
                {
                    while ((direccionRobot % 360) != 90)
                    {
                        contPrueba = 0;
                        motorGirarIzquierda();
                    }
                }
                // Si yAux < 0 debe ir hacia abajo en la matriz
                else
                {
                    while ((direccionRobot % 360) != 270)
                    {
                        contPrueba = 0;
                        motorGirarIzquierda();
                    }
                }
            }
        }
        break;
    default:
        printf("default\n");
        break;
    }
}

void termino_tramo(nodo *actual)
{
    datos_recorrido datos;
    vertice verticeActual = actual->actual;

    datos.fin_x = verticeActual.coordenadas.x;
    datos.fin_y = verticeActual.coordenadas.y;
    datos.origen_x = verticeInicioTramo.coordenadas.x;
    datos.origen_y = verticeInicioTramo.coordenadas.y;
    datos.distancia = distanciaTramo;

    // Se calcula una velocidad seudo-aleatoria, para la simulación
    // 2km/h es aproximadamente la velocidad promedio del robot
    velocidadTramo = 2 + (double)rand() / (double)RAND_MAX;

    datos.vel_max = velocidadTramo;

    enviar_datos_recorrido(curl, datos, grafoMapa.nombre, url_datos_recorridos);
    verticeInicioTramo = actual->actual;
    distanciaTramo = 0;
    velocidadTramo = 0;
}