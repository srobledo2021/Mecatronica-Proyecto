#include "esp_camera.h"
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Bluepad32.h>

// ===================
// Select camera model
// ===================
#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
#include "camera_pins.h"

// Pines para Serial2
#define RXD2 33
#define TXD2 4

// ===========================
// Configuración WiFi
// ===========================
const char* ssid = "****";
const char* password = "***";

// Configuración MQTT
#define MQTT_SERVER "teachinghub.eif.urjc.es"
#define MQTT_USERNAME "srs"
#define MQTT_KEY "33"
#define MQTT_PORT 21883
#define MQTT_TOPIC "/wukong/"

// Variables MQTT
WiFiClient wifiClient;
Adafruit_MQTT_Client mqttClient(&wifiClient, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_KEY);
Adafruit_MQTT_Subscribe mqttSubscription = Adafruit_MQTT_Subscribe(&mqttClient, MQTT_TOPIC);

// Variables Bluetooth
ControllerPtr myControllers[BP32_MAX_CONTROLLERS];

// Función para inicializar la cámara
void startCameraServer();
void MQTT_connect();
void processGamepad(ControllerPtr gamepad);
void onConnectedController(ControllerPtr ctl);
void onDisconnectedController(ControllerPtr ctl);

void setup() {
  // Inicializar comunicación serial
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Iniciando sistema...");

  // Configuración de la cámara
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Configuración WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");

  // Configurar MQTT
  mqttClient.subscribe(&mqttSubscription);

  // Configurar Bluetooth
  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys();

  Serial.println("Sistema listo.");
}

void loop() {
  // Conectar a MQTT si no está conectado
  if (!mqttClient.connected()) {
    MQTT_connect();
  }
  mqttClient.processPackets(100);

  // Procesar mensajes MQTT
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqttClient.readSubscription(1000))) {
    if (subscription == &mqttSubscription) {
      String message = (char *)mqttSubscription.lastread;
      Serial.print("Mensaje MQTT recibido: ");
      Serial.println(message);

      // Enviar al Serial2
      Serial2.println(message);
    }
  }

  // Actualizar Bluetooth
  BP32.update();
  for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
    ControllerPtr myController = myControllers[i];
    if (myController && myController->isConnected()) {
      if (myController->isGamepad()) {
        processGamepad(myController);
      }
    }
  }

  delay(10);
}

// Conectar a MQTT
void MQTT_connect() {
  while (mqttClient.connect() != 0) {
    Serial.println("Intentando reconectar a MQTT...");
    delay(5000);
  }
  Serial.println("Conectado a MQTT.");
}

// Callback Bluetooth
void onConnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
    if (myControllers[i] == nullptr) {
      myControllers[i] = ctl;
      Serial.println("Controlador Bluetooth conectado.");
      break;
    }
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
    if (myControllers[i] == ctl) {
      myControllers[i] = nullptr;
      Serial.println("Controlador Bluetooth desconectado.");
      break;
    }
  }
}

// Procesar Gamepad
void processGamepad(ControllerPtr gamepad) {
  char data = 'C';
  int dpadState = gamepad->dpad();
  if (dpadState & 0x01) {
    data = 'U'; // Arriba
  } else if (dpadState & 0x02) {
    data = 'D'; // Abajo
  } else if (dpadState & 0x04) {
    data = 'R'; // Derecha
  } else if (dpadState & 0x08) {
    data = 'L'; // Izquierda
  }

  Serial2.println(data);
  Serial.print("Dirección Bluetooth: ");
  Serial.println(data);
}
