#include "FastLED.h"
#include <ArduinoJson.h>
#include <Servo.h>

//--------LED----------
#define PIN_RBGLED 4
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];
int r = 0, g = 0, b = 0;
//--------------------

// Constantes para motores
#define FORWARD 100
#define GIRO 100

//-------Motor------------------
#define PIN_Motor_STBY 3
#define PIN_Motor_AIN_1 7
#define PIN_Motor_PWMA 5
#define PIN_Motor_BIN_1 8
#define PIN_Motor_PWMB 6

//--------Servo----------------
Servo servoX; // Servo horizontal
const int pinServoX = 10; // Pin del servo conectado
int currentServoAngle = 90; // Ángulo inicial del servo
const int servoStepSize = 1; // Tamaño del paso para el servo
const int servoStepDelay = 15; // Retardo entre pasos del servo (ms)

// Función para configurar el color del LED
uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
  return (((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

void led(String color) {
  if (color == "rojo") {
    r = 255; g = 0; b = 0;
  } else if (color == "verde") {
    r = 0; g = 255; b = 0;
  } else if (color == "amarillo") {
    r = 255; g = 255; b = 0;
  } else if (color == "azul") {
    r = 0; g = 0; b = 255;
  } else if (color == "blanco") {
    r = 255; g = 255; b = 255;
  } else {
    r = 0; g = 0; b = 0; // Apagar LED
  }
  FastLED.showColor(Color(r, g, b));
}

// Función para controlar los motores
void motorControl(bool motorAForward, int speedA, bool motorBForward, int speedB) {
  digitalWrite(PIN_Motor_AIN_1, motorAForward ? HIGH : LOW);
  analogWrite(PIN_Motor_PWMA, speedA);
  digitalWrite(PIN_Motor_BIN_1, motorBForward ? HIGH : LOW);
  analogWrite(PIN_Motor_PWMB, speedB);
}

void forward(int vel) {
  led("verde");
  motorControl(true, vel, true, vel);
}

void turnLeft(int vel) {
  led("amarillo");
  motorControl(true, vel, false, vel);
}

void turnRight(int vel) {
  led("azul");
  motorControl(false, vel, true, vel);
}

void back(int vel) {
  led("rojo");
  motorControl(false, vel, false, vel);
}

void stop_motors() {
  motorControl(false, 0, false, 0);
  led("blanco"); // LED blanco para indicar stop
}

void moveServoTowards(int targetAngle) {
  while (currentServoAngle != targetAngle) {
    if (currentServoAngle < targetAngle) {
      currentServoAngle++;
    } else if (currentServoAngle > targetAngle) {
      currentServoAngle--;
    }
    servoX.write(currentServoAngle);
    delay(servoStepDelay);
  }
}

void setup() {
  Serial.begin(9600);   // Comunicación serial para el monitor
  delay(1000);
  Serial.println("Iniciando sistema...");

  // Configuración motores
  pinMode(PIN_Motor_STBY, OUTPUT);
  pinMode(PIN_Motor_AIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMA, OUTPUT);
  pinMode(PIN_Motor_BIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMB, OUTPUT);
  digitalWrite(PIN_Motor_STBY, HIGH);

  // Configuración LED
  FastLED.addLeds<NEOPIXEL, PIN_RBGLED>(leds, NUM_LEDS);
  FastLED.setBrightness(20);

  // Configuración del servo
  servoX.attach(pinServoX);
  servoX.write(currentServoAngle);
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n'); // Leer datos del serial
    input.trim(); // Eliminar espacios y caracteres extra

    // Parsear el JSON si está presente
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, input);

    if (!error) {
      if (doc.containsKey("class") && doc["class"] == "person" && doc["confidence"] >= 0.8) {
        int centerX = doc["center"]["x"];
        int centerY = doc["center"]["y"];
        int servoTarget = doc["servo_target"]; // Leer el ángulo objetivo para el servo

        Serial.print("Persona detectada en - X: ");
        Serial.print(centerX);
        Serial.print(", Y: ");
        Serial.println(centerY);

        // Ajustar servo hacia el objetivo
        moveServoTowards(servoTarget);

        // Control de motores según la posición
        if (centerX < 400) {
          turnLeft(GIRO);
        } else if (centerX > 1000) {
          turnRight(GIRO);
        } else if (centerY < 500) {
          forward(FORWARD);
        } else {
          stop_motors();
        }
      } else {
        stop_motors();
      }
    } else {
      // Manejo de datos simples (no JSON)
      char direction = input.charAt(0);
      switch (direction) {
      case 'U':
        forward(FORWARD);
        break;
      case 'D':
        back(FORWARD);
        break;
      case 'L':
        turnLeft(FORWARD);
        break;
      case 'R':
        turnRight(FORWARD);
        break;
      default:
        stop_motors();
        break;
      }
      Serial.println(direction);
    }
  }
}
