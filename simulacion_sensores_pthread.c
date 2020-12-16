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

// Funciones que ejecutan los hilos que simulan los sensores
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
    REVERSA,
    DETENIDO
} estado_automatico; //Estado de la maquina de estados

// Funciones para la máquina de estados

// Controla el estado de la aspiradora (si pasa de manual a automatico o viceversa)
void MEF_Modo_Aspiradora();
// Controla los cambios de estado de la aspiradora cuando está en modo automático
void MEF_Automatico();
// Controla los cambios de estado de la aspiradora cuando está en modo manual
void MEF_Manual();
// Elige que funcion debe ejecutarse dependiendo si la aspiradora esta en modo manual o automático
nodo *MEF_Accion_Modo(nodo *);
// En modo automático, ejecuta la accion dependiendo del estado del robot
nodo *MEF_Accion_Automatico(nodo *);
// En modo manual, ejecuta la accion dependiendo del estado del robot
void MEF_Accion_Manual();

//Detiene las ruedas del robot
void Detener();
//Indica a las ruedas que deben moverce hacia adelante
void Mover_Adelante();
//Se controla las posiciones que se movio el robot en las coordenas x y
void Mover_Posicion();
//Indica al thread del ultrasonido que debe medir la distanccia, obtiene si hay obstaculo o no
//crea el vertice de ser necesario, guarda el tramo de ser necesario.
int Observar();
//Se mueve el servomotor al centro con respecto al robot
void Servo_Mirar_Centro();
//Se mueve el servomotor al la derecha con respecto al robot
void Servo_Mirar_Derecha();
//Se mueve el servomotor a la izquierda con respecto al robot
void Servo_Mirar_Izquierda();
//Se mueve las ruedas para realizar un giro a la izquierda
void Motor_Girar_Izquierda();
//Se mueve las ruedas para realizar un giro a la derecha
void Motor_Girar_Derecha();
//Se mueve las ruedas para realizar un giro de 180 grados
void Motor_Girar_180();
//Se disminuye la cantidad de interrupciones restantes para realizar un giro hasta llegar a 0
void Contar_Interrupciones(int cont);
//Realiza el algoritmo de inicio del robot
nodo *Secuencia_Inicio(void);
//Se crea un verice en el grafo
nodo *Crear_Vertice(nodo *);
//Funcion que se llama cada vez que se necesita hacer un delay, utilizado solo para generar un retardo
//para llegar a leer los mensajes que indican las acciones que realiza el robot
void Delay(void);
//Devuelve la coordenada siguiente a donde esta mirando el robot
coordenadas Coordenadas_Adyacente(int, int);
//Devuelve hacia donde mira el robot, teniendo en cuenta la posicion del robot y la posicion
//del servomotor con respecto al robot
direccion Direccion_Adyacente();
//Se lo llama cuando vuelve de la recursion y ejecuta la accion correspondiente teniendo en
//cuenta la accion que realizo antes de entrar a la recursion
void Retorna(estado_automatico estadoRecursion, nodo *actual, nodo *anterior);
//Entra a la recursion
void Recursion(nodo *actual, nodo *anterior);
//Se envian los datos del tramo entre obstaculos al servidor
void Termino_Tramo(nodo *actual);

// En esta variable se guarda el estado que llega del servidor
struct estadoRobot estadoRobot;

// Matriz que representa la habitación
// Es ultilizada por el ultrasonico para devolver datos que representen la realidad
// y poder verificar los resultados.
// 0 -> lugar accesible, 1 -> obstáculo
// int habitacion[10][10] =
//     {
//         {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
//         {1, 0, 1, 1, 1, 1, 1, 0, 0, 1},
//         {1, 0, 0, 0, 1, 1, 1, 0, 0, 1},
//         {1, 0, 1, 0, 1, 1, 1, 0, 0, 1},
//         {1, 0, 0, 0, 1, 1, 1, 0, 0, 1},
//         {1, 1, 1, 1, 1, 0, 0, 0, 0, 1},
//         {1, 0, 1, 0, 0, 0, 1, 1, 0, 1},
//         {1, 1, 1, 1, 0, 0, 1, 1, 0, 1},
//         {1, 1, 1, 1, 0, 0, 0, 0, 0, 1},
//         {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
    
int habitacion[10][10] =
    {
        {1, 1, 1, 1},
        {1, 0, 1, 1},
        {1, 0, 0, 1},
        {1, 0, 1, 1},
        {1, 1, 1, 1},
       };

// Posicion del robot dentro de la matriz
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
char *nombreHabitacion; //Nombre que tendrá la habitacion a mapear

// Variable utilizada con libCURL para hacer los pedidos HTTP al servidor
CURL *curl;

// Representan el estado de las ruedas
movimiento movimientoRuedaDerecha = ATRAS;
movimiento movimientoRuedaIzquierda = ADELANTE;

int hayObstaculo; // Utilizada cuando el ultrasonico mide la distancia

// Variables de las maquinas de estado en modo automático
estado_automatico estadoAnterior;
estado_automatico estadoActual;
estado_automatico estadoActualModo;

// Flags utilizados para modificar los estados de la MEF
int flagServo;
int flagMotor;
int flagAvanzo = 0;
int flagRetrocedio = 0;

// URLs a las que se mandan los pedidos HTTP
char *url_info_grafo = "http://localhost:3000/api/robotAspiradora/grafos";
char *url_vertices = "http://localhost:3000/api/robotAspiradora/vertices";
char *url_datos_recorridos = "http://localhost:3000/api/robotAspiradora/dato";
char *url_consultar_estado = "http://localhost:3000/api/robotAspiradora/consultaEstado";
char *url_consultar_habitacion = "http://localhost:3000/api/robotAspiradora/consultaNombre";

// Semaforo utilizado para detectar cuándo termino de medir distancia el ultrasónico
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

// Grafo que se va a generar con los vertices de la habitacion
grafo grafoMapa;

// Programa principal
void main()
{

    srand(time(NULL));

    // Hilos de sensores
    pthread_t ultrasonido;
    pthread_t ruedaDer;
    pthread_t ruedaIzq;

    sem_init(&semaforoDistancia, 0, 0);

    char *mensaje = "Thread 1";
    char *mensaje2 = "Thread 2";
    char *mensaje3 = "Thread 3";

    struct estadoRobot estadoAnterior;

    // Se inicializa la variable para comunicarnos con el servidor
    curl = configurar_conexion();

    // Nodo que almacenará el vertice inicial
    nodo *inicial;

    // Se crean los hilos que simulan los sensores
    pthread_create(&ultrasonido, NULL, funcion_sensor_ultrasonido, (void *)mensaje);
    pthread_create(&ruedaDer, NULL, funcion_sensor_encoder_derecha, (void *)mensaje2);
    pthread_create(&ruedaIzq, NULL, funcion_sensor_encoder_izquierda, (void *)mensaje3);

    printf("La aspiradora entra en funcionamiento\n");
    printf("-------------------------------------\n");
    // Se consulta el estado inicial del robot
    consultar_estado_robot(curl, url_consultar_estado, &estadoRobot);
    if (estadoRobot.modoAutomatico)
    {
        printf("- El robot inicia en modo automático\n");
        estadoActual = BARRIDO;
        estadoActualModo = AUTOMATICO;
    }
    else
    {
        printf("- El robot inicia en modo manual\n");
        estadoActual = DETENIDO;
        estadoActualModo = MANUAL;
    }

    // While principal
    while (1)
    {
        consultar_estado_robot(curl, url_consultar_estado, &estadoRobot);
        // Si cambia el estado del ventilador
        if (estadoRobot.ventiladorEncendido != estadoAnterior.ventiladorEncendido)
        {
            if (estadoRobot.ventiladorEncendido == 1)
            {
                printf("- Robot comienza a aspirar\n");
            }
            else
            {
                printf("- Robot deja de aspirar\n");
            }
        }
        // Si cambia el modo de la aspiradora
        if (estadoRobot.modoAutomatico != estadoAnterior.modoAutomatico)
        {
            if (estadoRobot.modoAutomatico == 1)
            {
                estadoActual = BARRIDO;
                estadoActualModo = AUTOMATICO;
                printf("- El robot pasa a modo automatico\n");
            }
            else
            {
                printf("- El robot pasa a modo manual\n");
                estadoActualModo = MANUAL;
                estadoActual = DETENIDO;
            }
        }

        // La aspiradora está en modo manual
        if (!estadoRobot.modoAutomatico)
        {
            MEF_Modo_Aspiradora();
            MEF_Accion_Modo(NULL);
        }

        // El robot esta en modo automatico y debe comenzar a mapear
        if ((estadoRobot.modoAutomatico) && (estadoRobot.mapeando) && (!estadoAnterior.mapeando))
        {
            direccionRobot = 0;
            direccionServo = 0;
            // Si ya hay un grafo en memoria, cuando se quiere crear uno nuevo se libera el anterior
            while (grafoMapa.lista.cabeza != NULL)
            {
                nodo *aux = grafoMapa.lista.cabeza;
                grafoMapa.lista.cabeza = grafoMapa.lista.cabeza->proximo;
                free(aux);
            }

            consultar_nombre_habitacion(curl, url_consultar_habitacion, &nombreHabitacion);
            printf("-------------------------------------\n");
            printf("Se comienza el mapeado de la habitacion\n");
            printf("-------------------------------------\n");
            getchar(); //Se pausa
            inicializar_grafo(&grafoMapa, nombreHabitacion);
            inicial = Secuencia_Inicio();
            iniciaRecorrido = 1;                  //Se indica que salio de la secuencia de inicio y se inicia el recorrido;
            verticeInicioTramo = inicial->actual; //Se guarda el vertice de inicio del tramo
            distanciaTramo = 0;                   //Se inicializa la distancia del tramo
            // Se hace el primer llamado a la función recursiva
            // donde se llenará el grafo, con el vertice inicial
            Recursion(inicial, NULL);
            printf("-------------------------------------\n");
            printf("Mapeado de la habitación finalizado\n");
            printf("-------------------------------------\n");
            estadoRobot.mapeando = 0;
            consultar_estado_robot(curl, url_consultar_estado, &estadoRobot);
            x = 1;
            y = 1;
            direccionRobot = 0;
            direccionServo = 0;
            retornando = 0;
            // Se calculan las dimensiones del mapa
            calculo_dimensiones_mapa(&grafoMapa);

            // Se envía la información del grafo al servidor
            enviar_info_grafo(curl, grafoMapa, url_info_grafo);

            // Se imprime el grafo
            printf("Grafo final:\n");
            printf("-------------------------------------\n");
            imprimir_grafo(grafoMapa);
            printf("-------------------------------------\n");
        }
        estadoAnterior = estadoRobot;
        //Se espera 1 segundo para enviar denuevo el request al servidor para obtener el estado
        usleep(1000000);
    }
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
        if (flagAvanzo) //Se espera a que se haya avanzado una posicion
        {
            flagAvanzo = 0;
            estadoActual = DETENIDO;
        }
        break;
    case GIRANDO_DERECHA:
        if (flagMotor) //Se espera a que se haya detenido el motor
        {
            flagMotor = 0;
            estadoActual = DETENIDO;
        }
        break;
    case GIRANDO_IZQUIERDA:
        if (flagMotor) //Se espera a que se haya detenido el motor
        {
            flagMotor = 0;
            estadoActual = DETENIDO;
        }
        break;
    case REVERSA:
        if (flagRetrocedio) //Se espera a que se haya retrocedid
        {
            flagRetrocedio = 0;
            estadoActual = DETENIDO;
        }
        break;
    case DETENIDO:
        switch (estadoRobot.direccionManual) //Se evalua la accion que debe realizar el robot
        {
        case AVANZAR:
            estadoActual = MOVIENDOSE;
            break;
        case RETROCEDER:
            estadoActual = REVERSA;
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
        distancia = 0;
        estadoAnterior = estadoActual;
        if (hayObstaculo)
            estadoActual = BARRIDO;
        else
            estadoActual = MOVIENDOSE;

        break;
    case MIRANDO_DERECHA:
        if (flagServo) //Se espera a que se detenga el servomotor
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
        Motor_Girar_Derecha();
        flagMotor = 1;
        break;
    case GIRANDO_IZQUIERDA:
        Motor_Girar_Izquierda();
        flagMotor = 1;
        break;
    case REVERSA:
        movimientoRuedaIzquierda = ATRAS;
        movimientoRuedaDerecha = ATRAS;
        printf("El robot retrocedió 25 cm\n");
        flagRetrocedio = 1;
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

    getchar(); // Se pausa 
    printf("--------------------------\n");
    printf("Pos X: %d Pos Y: %d\n",x,y);
    printf("--------------------------\n");

    switch (estadoActual)
    {
    case MOVIENDOSE:
        Mover_Adelante();
        posiciones--;
        actual->actual.estado = Visitado;
        enviar_vertices_grafo(curl, actual->actual, grafoMapa.nombre, url_vertices);
        hayObstaculo = Observar(actual);
        // Si hay obstaculo vuelve con el mismo vertice y si no hay obstáculo devuelve el próximo vertice
        if (!hayObstaculo)
        {
            proximo = actual->adyacentes[Direccion_Adyacente()];
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
        Servo_Mirar_Derecha();
        hayObstaculo = Observar(actual);
        proximo = actual;
        // Vuelve a colocar el servomotor al centro
        Servo_Mirar_Centro();
        flagServo = 1;
        break;
    case MIRANDO_IZQUIERDA:
        printf("MIRANDO_IZQUIERDA\n");
        Servo_Mirar_Izquierda();
        hayObstaculo = Observar(actual);
        proximo = actual;
        // Vuelve a colocar el servomotor al centro
        Servo_Mirar_Centro();
        flagServo = 1;
        break;
    case GIRANDO_IZQUIERDA:
        contPrueba = 0;
        Motor_Girar_Izquierda();
        printf("Se generon %d interrupciones\n", contPrueba);
        proximo = actual->adyacentes[Direccion_Adyacente()];
        // Detener();
        flagMotor = 1;
        break;
    case GIRANDO_DERECHA:
        contPrueba = 0;
        Motor_Girar_Derecha();
        printf("Se generon %d interrupciones\n", contPrueba);
        proximo = actual->adyacentes[Direccion_Adyacente()];
        // Detener();
        flagMotor = 1;
        break;
    case GIRANDO_180:
        contPrueba = 0;
        Motor_Girar_180();
        printf("Se generon %d interrupciones\n", contPrueba);
        flagMotor = 1;
        hayObstaculo = Observar(actual);
        if (!hayObstaculo)
        {
            hayObstaculo180 = 0;
            // Si no hay obstaculo pasamos el vertice anterior
            proximo = actual->adyacentes[Direccion_Adyacente()];
        }
        else
        {
            hayObstaculo180 = 1;
            proximo = actual;
        }
        retornando = 1;
        break;
    default:
        break;
    }

    int i;
    return proximo;
}

void *funcion_sensor_ultrasonido(void *ptr)
{

    char *mensaje;
    mensaje = (char *)ptr;

    while (1)
    {
        if (trigger == 1)
        {

            // Utiliza la matrix que representa la habitación para calcular la distancia
            int xAux = x;
            int yAux = y;
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
                xAux++;
                while ((habitacion[yAux][xAux] != 1) && (posicionesLibres < 16))
                {
                    posicionesLibres++;
                    xAux++;
                }
                break;
            case 90:
                // Esta observando hacia arriba en la matriz
                yAux--;
                while ((habitacion[yAux][xAux] != 1) && (posicionesLibres < 16))
                {
                    posicionesLibres++;
                    yAux--;
                }
                break;
            case 180:
                // Esta observando hacia izquierda en la matriz
                xAux--;
                while ((habitacion[yAux][xAux] != 1) && (posicionesLibres < 16))
                {
                    posicionesLibres++;
                    xAux--;
                }
                break;
            case 270:
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
    while (1)
    {
        //Si la rueda se mueve en alguna direccion se modifica el valor que devuelve el encoder
        if ((movimientoRuedaDerecha == ATRAS) || (movimientoRuedaDerecha == ADELANTE))
        {
            encoderState_derecha = 1 - encoderState_derecha; //Se hace un toggle entre 1 y 0
            if (encoderState_derecha == 0)
                contPrueba++;
            usleep(8928); //Se espera 0.008928 seg, esto cambiaria segun la velocidad de la rueda
        }
    }
}

void *funcion_sensor_encoder_izquierda(void *ptr)
{
    char *mensaje;
    mensaje = (char *)ptr;
    while (1)
    {
        //Si la rueda se mueve en alguna direccion se modifica el valor que devuelve el encoder
        if ((movimientoRuedaIzquierda == ATRAS) || (movimientoRuedaIzquierda == ADELANTE))
        {

            encoderState_izquierda = 1 - encoderState_izquierda; //Se hace un toggle entre 1 y 0
            if (encoderState_izquierda == 0)
                contPrueba++;
            usleep(8928); //Se espera 0.008928 seg, esto cambiaria segun la velocidad de la rueda
        }
    }
}

void Detener()
{
    movimientoRuedaIzquierda = QUIETO;
    movimientoRuedaDerecha = QUIETO;
    printf("- Se detuvo el robot\n");
    Delay();
}

void Mover_Adelante()
{
    movimientoRuedaIzquierda = ADELANTE;
    movimientoRuedaDerecha = ADELANTE;

    switch (direccionRobot)
    {
    case 0:
        x++;
        Mover_Posicion();
        break;

    case 90:
        y--;
        Mover_Posicion();
        break;

    case 180:
        x--;
        Mover_Posicion();
        break;

    case 270:
        y++;
        Mover_Posicion();
        break;

    default:
        break;
    }
    // Incrementa la distancia en 25cm;
    distanciaTramo += 25;
    printf("- Avanzando . . .\n");
    Delay();
}

void Mover_Posicion()
{
    int cont = INT_AVANZAR_POSICION;

    movimientoRuedaIzquierda = ADELANTE;
    movimientoRuedaDerecha = ADELANTE;
    Contar_Interrupciones(cont);
    printf("- El robot avanzó una posicion\n");
    Delay();
}

int Observar(nodo *actual)
{
    int visitado = 0;
    int obs;
    printf("- Sensor ultrasónico observando . . .\n");
    Delay();
    obs = Ultrasonico_Trigger();
    // Si el proximo vertice YA EXISTE y esta visitado se lo coloca como obstáculo, para que no avance
    if (actual->adyacentes[Direccion_Adyacente()] != NULL)
    {
        if (actual->adyacentes[Direccion_Adyacente()]->actual.estado == Visitado)
        {
            obs = 1;
        }
    }
    else
    {
        // Si el vertice no existe, lo crea
        Crear_Vertice(actual);
        printf("creo el vertice\n");
    }

    // Si hay obstáculo:
    if (obs)
    {
        printf("- Hay un obstaculo proximo\n");
        printf("Distancia al obstáculo: %f cm\n", distancia);
        // Si el servo esta mirando al centro ,no esta en la posicion inicial y no es un
        // vertice visidado (inicio recorrido) quiere decir que estaba avanzando
        // y encontró un obstáculo, por lo que lo envía al servidor
        if ((direccionServo == 0) && (iniciaRecorrido) &&
            ((verticeInicioTramo.coordenadas.x != actual->actual.coordenadas.x) ||
             (verticeInicioTramo.coordenadas.y != actual->actual.coordenadas.y)))
        {
            // Envia los datos del tramo realizado al servidor
            Termino_Tramo(actual);
        }
        return 1;
    }
    else
    {
        printf("- No hay obstaculo inminente\n");
        printf("Distancia al proximo obstáculo: %f cm\n", distancia);
        return 0;
    }
    Delay();
}

void Servo_Mirar_Centro()
{
    printf("- Girando servo al centro\n");
    Delay();
    printf("- Servo girado al centro\n");
    Delay();
    direccionServo = 0;
}

void Servo_Mirar_Derecha()
{
    printf("- Girando servo a derecha\n");
    Delay();
    printf("- Servo girado a derecha\n");
    Delay();
    direccionServo = 270;
}

void Servo_Mirar_Izquierda()
{
    printf("- Girando servo a izquierda\n");
    Delay();
    printf("- Servo girado a izquierda\n");
    Delay();
    direccionServo = 90;
}

void Motor_Girar_Izquierda()
{
    int cont = INT_GIRO_90;
    printf("- Girando robot a izquierda\n");
    movimientoRuedaIzquierda = ATRAS;
    movimientoRuedaDerecha = QUIETO;
    Contar_Interrupciones(cont);
    movimientoRuedaIzquierda = QUIETO;
    movimientoRuedaDerecha = QUIETO;
    printf("- Robot girado a izquierda\n");
    direccionRobot = (direccionRobot + 90) % 360;
    Delay();
}

void Motor_Girar_Derecha()
{
    int cont = INT_GIRO_90;
    printf("- Girando robot a derecha\n");
    movimientoRuedaIzquierda = ADELANTE;
    movimientoRuedaDerecha = QUIETO;
    Contar_Interrupciones(cont);
    movimientoRuedaIzquierda = QUIETO;
    movimientoRuedaDerecha = QUIETO;
    printf("- Robot girado a derecha\n");
    direccionRobot = (direccionRobot + 270) % 360;
    Delay();
}

void Motor_Girar_180()
{
    printf("- Girando robot 180º\n");
    int cont = INT_GIRO_180;
    movimientoRuedaIzquierda = ADELANTE;
    movimientoRuedaDerecha = QUIETO;
    Contar_Interrupciones(cont);
    //Delay();
    printf("- Robot girado 180º\n");
    direccionRobot = (direccionRobot + 180) % 360;
    Delay();
}

void Contar_Interrupciones(int cont)
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

    Servo_Mirar_Centro();
    // Se crea el vertice inicial
    v.coordenadas.x = x;
    v.coordenadas.y = y;
    v.estado = Visitado;
    // Se agrega el vertice inicial al grafo
    inicial = agregar_vertice(&grafoMapa, v);
    // Se incrementa la cantidad de vertices del grafo
    grafoMapa.vertices++;
    // Se envía el vertice al servidor
    printf("Se va a mapear la habitacion: %s\n", grafoMapa.nombre);
    getchar(); //Se pausa
    enviar_vertices_grafo(curl, inicial->actual, grafoMapa.nombre, url_vertices);

    hayObstaculo = Observar(inicial);
    distanciaCentro = distancia;
    distancia = 0;
    distanciaMaxima = distanciaCentro;
    if (!hayObstaculo)
    {
        direccionMayorDistancia = 'C';
    }


    getchar(); //Se pausa
    Servo_Mirar_Derecha();
    hayObstaculo = Observar(inicial);
    distanciaDerecha = distancia;
    distancia = 0;
    if (distanciaDerecha > distanciaMaxima)
    {
        distanciaMaxima = distanciaDerecha;
        direccionMayorDistancia = 'D';
    }


    getchar(); //Se pausa
    Servo_Mirar_Izquierda();
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
        break;

    case 'D':
        estadoActual = GIRANDO_DERECHA;
        break;

    case 'I':
        estadoActual = GIRANDO_IZQUIERDA;
        break;
    deafult:
        estadoActual = GIRANDO_180;
        break;
    }

    getchar(); //Se pausa
    Servo_Mirar_Centro();
    return inicial;
}

void Delay(void)
{
    usleep(20);
}

coordenadas Coordenadas_Adyacente(int xAux, int yAux)
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

direccion Direccion_Adyacente()
{
    int direccion;
    int direccionTotal = (direccionRobot + direccionServo) % 360;

    // Se toma la direccion total que puede ser 0, 90, 180 o 270 y se la divide por 90
    // para que devuelva un valor de 0 a 3 que represente la direccion;
    direccion = direccionTotal / 90;
    return direccion;
}

nodo *Crear_Vertice(nodo *actual)
{
    vertice v;
    nodo *adyacente;
    int hayObstaculo;
    // Se crea el proximo vertice
    v.coordenadas = Coordenadas_Adyacente(x, y);
    if (hayObstaculo)
    {
        v.estado = Inaccesible;
    }
    else
    {
        v.estado = NoVisitado;
    }

    if ((actual->adyacentes[Direccion_Adyacente()]) == NULL)
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
            enviar_vertices_grafo(curl, adyacente->actual, grafoMapa.nombre, url_vertices);
        }
    }
    else
    {
        adyacente = actual->adyacentes[Direccion_Adyacente()];
    }

    return adyacente;
}

void Recursion(nodo *actual, nodo *anterior)
{   
    nodo *proximo = NULL;
    estado_automatico estadoRecursion = estadoActual;
    int i;

    proximo = MEF_Accion_Modo(actual);
    MEF_Modo_Aspiradora();
    // Si el robot esta volviendo de la recursion, por el camino que fue realizando
    if (!retornando)
    {
        // Si el proximo vertice == actual, se envia el actual y anterior, para que no se pierda el anterior
        if (proximo == actual)
        {
            Recursion(actual, anterior);
        }
        else
        {
            Recursion(proximo, actual);
        }
    }
    if (retornando)
    {
        if (((estadoRecursion == GIRANDO_DERECHA) || (estadoRecursion == GIRANDO_IZQUIERDA) || (estadoRecursion == MOVIENDOSE)) && (actual != NULL))
        {
            getchar(); // Se pausa
            // Revisa el camino derecho
            Servo_Mirar_Derecha();
            hayObstaculo = Observar(actual);
            getchar(); // Se pausa
            // Vuelve a colocar el servomotor al centro
            Servo_Mirar_Centro();
            if (!hayObstaculo)
            {
                retornando = 0;
                estadoActual = GIRANDO_DERECHA;
                Recursion(actual, anterior);
            }
            getchar(); // Se pausa
            // Revisa el camino izquierdo
            Servo_Mirar_Izquierda();
            hayObstaculo = Observar(actual);
            getchar(); // Se pausa
            // Vuelve a colocar el servomotor al centro
            Servo_Mirar_Centro();
            if (!hayObstaculo)
            {
                retornando = 0;
                estadoActual = GIRANDO_IZQUIERDA;
                Recursion(actual, anterior);
            }
            Retorna(estadoRecursion, actual, anterior);
        }
    }
    return;
}

void Retorna(estado_automatico estadoRecursion, nodo *actual, nodo *anterior)
{
    getchar(); // Se pausa
    printf("CAMINO SIN SALIDA, RETORNANDO\n");
    int xAux;
    int yAux;
    switch (estadoRecursion)
    {
    // Si el robot estaba moviendose, al volver de la recursión debe moverse en
    // la nueva dirección, que será la opuesta a la que tenía.
    case MOVIENDOSE:
        Mover_Adelante();
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
                        Motor_Girar_Izquierda();
                    }
                }
                // Si xAux < 0 debe ir hacia la derecha en la matriz
                else
                {
                    while ((direccionRobot % 360) != 0)
                    {
                        contPrueba = 0;
                        Motor_Girar_Izquierda();
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
                        Motor_Girar_Izquierda();
                    }
                }
                // Si yAux < 0 debe ir hacia abajo en la matriz
                else
                {
                    while ((direccionRobot % 360) != 270)
                    {
                        contPrueba = 0;
                        Motor_Girar_Izquierda();
                    }
                }
            }
        }
        break;
    default:
        break;
    }
}

void Termino_Tramo(nodo *actual)
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