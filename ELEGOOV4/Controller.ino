#include "FastLED.h"
#include <ArduinoJson.h>


//--------LED----------
#define PIN_RBGLED 4
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];
int r=0,g=0,b=0;
//--------------------

#define FORWARD 100
#define GIRO 100

//-------Motor------------------
// Enable/Disable motor control.
//  HIGH: motor control enabled
//  LOW: motor control disabled
#define PIN_Motor_STBY 3

// Group A Motors (Right Side)
// PIN_Motor_AIN_1: Digital output. HIGH: Forward, LOW: Backward
#define PIN_Motor_AIN_1 7
// PIN_Motor_PWMA: Analog output [0-255]. It provides speed.
#define PIN_Motor_PWMA 5

// Group B Motors (Left Side)
// PIN_Motor_BIN_1: Digital output. HIGH: Forward, LOW: Backward
#define PIN_Motor_BIN_1 8
// PIN_Motor_PWMB: Analog output [0-255]. It provides speed.
#define PIN_Motor_PWMB 6

uint32_t Color(uint8_t r, uint8_t g, uint8_t b)
{
  return (((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

void led(String color){
  if(color == "rojo"){
    r=255;
    g=0;
    b=0;

  } else if (color == "verde"){
    r=0;
    g=255;
    b=0;

  } else if (color == "amarillo"){
    r=255;
    g=255;
    b=0;
    
  } else if (color == "rosa"){
    r=255;
    g=192;
    b=203;
    
  } else if (color == "azul"){
    r=0;
    g=0;
    b=255;
    
  } else if (color == "marron"){
    r=165;
    g=42;
    b=42;
    
  } else if (color == "blanco"){
    r=255;
    g=255;
    b=255;
    
  } else if (color == "apagar"){
    r=0;
    g=0;
    b=0;
    
  } else {
    Serial.print("Error");
  }

  FastLED.showColor(Color(r, g, b));
}

// Function to control motors
void motorControl(bool motorAForward, int speedA, bool motorBForward, int speedB) {
  // Control motor A (RIGHT)
  digitalWrite(PIN_Motor_AIN_1, motorAForward ? HIGH : LOW);
  analogWrite(PIN_Motor_PWMA, speedA);

  // Control motor B (LEFT)
  digitalWrite(PIN_Motor_BIN_1, motorBForward ? HIGH : LOW);
  analogWrite(PIN_Motor_PWMB, speedB);
}

void forward(int vel){
  led("verde");
  motorControl(true,vel,true,vel);
}
void turnLeft(int vel){
  led("amarillo");
  motorControl(true,vel,false,vel);
}
void turnRight(int vel){
  led("azul");
  motorControl(false,vel,true,vel);
}
void back(int vel){
  led("rojo");
  motorControl(false,vel,false,vel);
}
void stop_motors() {
  led("blanco");
  motorControl(false,0,false,0);
}


void setup() {
  // Inicializar el puerto serial
  Serial.begin(9600);   // Comunicación serial para el monitor
  delay(1000);
  Serial.println("Iniciando comunicación con ESP32...");
  //motors
  // Turn on the engines !!!!
  digitalWrite(PIN_Motor_STBY, HIGH);
  // Set motor control pins as outputs
  pinMode(PIN_Motor_STBY, OUTPUT);
  pinMode(PIN_Motor_AIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMA, OUTPUT);
  pinMode(PIN_Motor_BIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMB, OUTPUT);

  // LED
  FastLED.addLeds<NEOPIXEL, PIN_RBGLED>(leds, NUM_LEDS);
  FastLED.setBrightness(20);
}

void loop() {
  // Verificar si hay datos disponibles en el puerto serial
  if (Serial.available()) {

    char direction = Serial.read();

    switch (direction) {
    case 'U':  // Arriba
      forward(FORWARD);
      break;
    case 'D':  // Abajo
      back(FORWARD);
      break;
    case 'L':  // Izquierda
      turnLeft(FORWARD);
      break;
    case 'R':  // Derecha
      turnRight(FORWARD);
      break;
    case 'C':
      // Si no hay ninguna dirección, no se hace nada
      stop_motors();
      break;
    }

    Serial.print(direction);    
  }

}