#include "esp_camera.h"
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// ===================
// Select camera model
// ===================
#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
#include "camera_pins.h"
#define RXD2 33
#define TXD2 4
// ===========================
// Enter your WiFi credentials
// ===========================
const char* ssid = "****";
const char* password = "****";

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

void startCameraServer();
void setupLedFlash(int pin);

void MQTT_connect() {
  int8_t ret;

  while ((ret = mqttClient.connect()) != 0) {
    Serial.println(mqttClient.connectErrorString(ret));
    Serial.println("Reintentando conexión MQTT en 5 segundos...");
    delay(5000);
  }
  Serial.println("Conexión MQTT establecida.");
}

void setup() {
  Serial.begin(115200); // Comunicación para depuración
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.setDebugOutput(true);
  Serial.println();

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
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    config.frame_size = FRAMESIZE_240X240;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }
  if (config.pixel_format == PIXFORMAT_JPEG) {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");

  // Configurar MQTT
  mqttClient.subscribe(&mqttSubscription);

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  if (!mqttClient.connected()) {
    MQTT_connect();
  }
  mqttClient.processPackets(10000); // Procesar mensajes recibidos

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqttClient.readSubscription(5000))) {
    if (subscription == &mqttSubscription) {
      String message = String((char *)mqttSubscription.lastread);
      Serial.print("Mensaje recibido: ");
      Serial.println(message);

      // Enviar mensaje al Arduino conectado a Serial2
      Serial2.println(message);
      Serial.println("Enviado al Arduino por Serial2: " + message);
    }
  }
  delay(10);
}
