#include <Servo.h>

// Configuración del servo
Servo servoX; // Servo para el eje X (horizontal)
Servo servoY; // Servo para el eje Y (vertical)

const int pinServoX = 10; // Pin Servo1 (Eje X) conectado al SmartCar Shield
const int pinServoY = 9;  // Pin Servo2 (Eje Y) si se necesita otro servo

// Coordenadas del centro de la cámara
const int centerX = 920; // Coordenada X del centro
const int centerY = 735; // Coordenada Y del centro

// Tolerancia para considerar la persona centrada
const int toleranceX = 20; // Tolerancia en píxeles para X
const int toleranceY = 20; // Tolerancia en píxeles para Y

// Coordenadas actuales del servo
int currentX = 180; // Ángulo inicial del eje X (centro lógico)
int currentY = 180; // Ángulo inicial del eje Y (centro lógico)

// Velocidad de movimiento
const int stepDelay = 15; // Retardo entre pasos (ms)
const int stepSize = 1;   // Tamaño del paso (grados)

void setup() {
  Serial.begin(9600); // Comunicación Serial para datos del ESP32
  Serial.println("Sistema de control de servos iniciado.");

  // Inicializar los servos
  servoX.attach(pinServoX);
  servoY.attach(pinServoY);

  // Posicionar los servos en el centro
  servoX.write(currentX);
  servoY.write(currentY);
}

void moveServoTowards(Servo &servo, int &currentAngle, int targetAngle) {
  if (currentAngle < targetAngle) {
    currentAngle += stepSize;
    if (currentAngle > targetAngle) {
      currentAngle = targetAngle; // Asegurar que no exceda el objetivo
    }
  } else if (currentAngle > targetAngle) {
    currentAngle -= stepSize;
    if (currentAngle < targetAngle) {
      currentAngle = targetAngle; // Asegurar que no exceda el objetivo
    }
  }
  servo.write(currentAngle);
  delay(stepDelay); // Pequeño retardo para movimiento suave
}

void loop() {
  // Leer datos del puerto Serial
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n'); // Leer hasta el final de la línea
    input.trim(); // Eliminar caracteres extra

    // Imprimir el mensaje recibido para depuración
    Serial.print("Mensaje recibido: ");
    Serial.println(input);

    // Verificar si el mensaje tiene el formato esperado
    if (input.indexOf("'center'") > -1 && input.indexOf("'confidence'") > -1) {
      // Extraer la confianza
      int confIndex = input.indexOf("'confidence': ") + 13;
      String confidenceStr = input.substring(confIndex, input.indexOf(',', confIndex));
      float confidence = confidenceStr.toFloat();

      // Solo continuar si la confianza es mayor a 0.8
      if (confidence <= 0.8) {
        Serial.println("Confianza insuficiente, no se realiza ningún movimiento.");
        return;
      }

      // Extraer las coordenadas x e y
      int xIndex = input.indexOf("'x': ") + 5;
      int yIndex = input.indexOf("'y': ") + 5;

      String xCoord = input.substring(xIndex, input.indexOf(',', xIndex));
      String yCoord = input.substring(yIndex, input.indexOf('}', yIndex));

      int x = xCoord.toInt();
      int y = yCoord.toInt();

      // Imprimir las coordenadas para depuración
      Serial.print("Coordenadas extraídas - X: ");
      Serial.print(x);
      Serial.print(", Y: ");
      Serial.print(y);
      Serial.print(", Confianza: ");
      Serial.println(confidence);

      // Verificar si las coordenadas están dentro de la tolerancia
      if (abs(x - centerX) <= toleranceX && abs(y - centerY) <= toleranceY) {
        Serial.println("Persona centrada.");
        return; // No mover el servo si está centrado
      }

      // Calcular los ángulos de movimiento
      int deltaX = x - centerX;
      int deltaY = y - centerY;

      currentX += deltaX / 50; // Ajustar movimiento proporcional
      currentY += deltaY / 50; // Ajustar movimiento proporcional

      // Mover los servos hacia las nuevas coordenadas lentamente
      moveServoTowards(servoX, currentX, currentX);
      moveServoTowards(servoY, currentY, currentY);

      // Depuración
      Serial.print("Servo movido a - X: ");
      Serial.print(currentX);
      Serial.print(", Y: ");
      Serial.println(currentY);
    } else {
      Serial.println("Formato de mensaje no válido.");
    }
  }
}
