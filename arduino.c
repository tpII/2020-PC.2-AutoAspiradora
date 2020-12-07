#include <Servo.h>
// Conexiones sensor ultrasónico
#define trigPin 13
#define echoPin 12
// Conexiones del Motor 1
#define in1 11
#define in2 10
// Conexiones del Motor 2
#define in3 5
#define in4 4
// Conexion servomotor
#define servoPin 9 

// crea el objeto servo para controlar el servomotor
Servo myservo;  

char direccion; //(D,I o C)


void setup() {
  
  Serial.begin(9600);
  
  // Setea los pines de control del sensor ultrasónico
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Setea los pines de control del motor como salidas
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  // Apaga los motores - estado inciial 
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  
  myservo.attach(9);  // une el servo al pin 9

  // Se coloca el servo en la posición inicial -> 90º
  myservo.write(90);
  delay(1000);
}

void loop() {
  // Busca la dirección óptima
  direccion = direccionOptima();
  // Posiciona al robot en dicha dirección.
  posicionar(direccion);
  // Avanzar
  avanzar();
  parar();
}

char direccionOptima(){
  int distanciaMedida;
  int distanciaOptima = 9999;
  
  // Buscar obstáculo centro
  Serial.println("Buscando obstaculo centro...");
  myservo.write(90);
  delay(5000);
  distanciaMedida = distancia();
  if(distanciaMedida < distanciaOptima){
    distanciaOptima = distanciaMedida;
  	direccion = 'C';
  }
  
  // Buscar obstáculo derecha
  Serial.println("Buscando obstaculo derecha...");
  myservo.write(0);
  delay(5000);
  distanciaMedida = distancia();
  if(distanciaMedida < distanciaOptima){
    distanciaOptima = distanciaMedida;
  	direccion = 'D';
  }
            
  // Buscar obstáculo izquierda
  Serial.println("Buscando obstaculo izquierda...");
  myservo.write(180);
  delay(5000);
  distanciaMedida = distancia();
  if(distanciaMedida < distanciaOptima){
    distanciaOptima = distanciaMedida;
  	direccion = 'I';
  }
  
  // Devuelve el servo a la posicion inicial
  myservo.write(90);
  delay(5000);
  
  return direccion;
}

void avanzar(){
	digitalWrite(in1, LOW);
	digitalWrite(in2, HIGH);
	digitalWrite(in3, LOW);
	digitalWrite(in4, HIGH);
  	Serial.println("Avanzando");
  	while(distancia()>10);
}

void parar(){
  	digitalWrite(in1, LOW);
	digitalWrite(in2, LOW);
	digitalWrite(in3, LOW);
	digitalWrite(in4, LOW);
  	Serial.println("Parado");
  }
  

void posicionar(char direccion){
  switch(direccion){
    case 'C':
    	Serial.println("Posicionado al centro");
    	break;
    
    case 'D':
    	Serial.println("Posicionado a la derecha");
    	break;
    
    case 'I':
    	Serial.println("Posicionado a la izquierda");
    	break;
    
    default:
    	break;
  }  
}

int distancia(){
  long duration;
  int distance;
  // Limpia el bit de trigger poniendolo en LOW
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  // Dispara el sensor, setteando el pin trigger en alto 
  // por 10 microsegundos
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Lee el pin echo, la funcion pulseIn() retorna la duracion 
  // del pulso en microsegundos
  duration = pulseIn(echoPin, HIGH);
  // Calcula la distancia:
  distance = duration * 0.034 / 2;
  Serial.print(distance);
  Serial.println(" cm");
  return distance;
 }