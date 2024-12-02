import cv2
import numpy as np
import pyautogui
import math
from ultralytics import YOLO
import paho.mqtt.client as mqtt

# Configuración de MQTT
MQTT_SERVER = "teachinghub.eif.urjc.es"
MQTT_USERNAME = "srs"
MQTT_KEY = "33"
MQTT_PORT = 21883
MQTT_TOPIC = "/wukong/"

# Conectar al servidor MQTT
mqtt_client = mqtt.Client()
mqtt_client.username_pw_set(MQTT_USERNAME, MQTT_KEY)
mqtt_client.connect(MQTT_SERVER, MQTT_PORT, keepalive=60)

# YOLO Model
model = YOLO("yolo-Weights/yolov8n.pt")

# Clase objetivo
TARGET_CLASS = "person"

def capture_screen(region=None):
    """
    Captura una porción de la pantalla o toda la pantalla si no se especifica una región.

    :param region: Tuple (x, y, width, height) para especificar la región a capturar
    :return: Imagen como un array numpy
    """
    screenshot = pyautogui.screenshot(region=region)
    frame = np.array(screenshot)
    # Convertir de RGB a BGR para OpenCV
    return cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)

def process_image():
    cv2.namedWindow("live transmission", cv2.WINDOW_AUTOSIZE)

    while True:
        # Captura la pantalla completa
        img = capture_screen()

        # Procesar la imagen con YOLO
        results = model(img, stream=True)

        # Dibujar cajas y etiquetas
        for r in results:
            boxes = r.boxes
            for box in boxes:
                # Coordenadas de la caja
                x1, y1, x2, y2 = box.xyxy[0]
                x1, y1, x2, y2 = int(x1), int(y1), int(x2), int(y2)

                # Nombre de la clase detectada
                cls = int(box.cls[0])
                class_name = model.names[cls]

                # Filtrar solo personas
                if class_name != TARGET_CLASS:
                    continue

                # Centro de la caja
                center_x = (x1 + x2) // 2
                center_y = (y1 + y2) // 2

                # Dibujar el rectángulo
                cv2.rectangle(img, (x1, y1), (x2, y2), (255, 0, 255), 3)

                # Confianza del modelo
                confidence = math.ceil((box.conf[0] * 100)) / 100
                print(f"Detected {class_name} with confidence {confidence}")

                # Publicar al servidor MQTT
                mqtt_message = {
                    "class": class_name,
                    "confidence": confidence,
                    "center": {"x": center_x, "y": center_y}
                }
                mqtt_client.publish(MQTT_TOPIC, str(mqtt_message))

                # Mostrar detalles del objeto
                org = (x1, y1 - 10)
                font = cv2.FONT_HERSHEY_SIMPLEX
                fontScale = 1
                color = (255, 0, 0)
                thickness = 2

                cv2.putText(img, f"{class_name} {confidence}", org, font, fontScale, color, thickness)

        # Mostrar la imagen procesada
        cv2.imshow('live transmission', img)
        key = cv2.waitKey(1)
        if key == ord('q'):
            break

    cv2.destroyAllWindows()

if __name__ == '__main__':
    print("Iniciando...")
    mqtt_client.loop_start()  # Iniciar el cliente MQTT
    process_image()
    mqtt_client.loop_stop()   # Detener el cliente MQTT al salir
