// Copyright 2021 - 2023, Ricardo Quesada
// SPDX-License-Identifier: Apache 2.0 or LGPL-2.1-or-later

/*
 * This example shows how to use the Controller API.
 *
 * Supported on boards with NINA W10x. In particular, these boards:
 *  - Arduino MKR WiFi 1010,
 *  - UNO WiFi Rev.2,
 *  - Nano RP2040 Connect,
 *  - Nano 33 IoT,
 *  - Arduino MKR Vidor 4000
 */
#include <Bluepad32.h>

ControllerPtr myControllers[BP32_MAX_CONTROLLERS];

#define RXD2 33
#define TXD2 4

// Arduino setup function. Runs in CPU 1
void setup() {
  // Initialize serial
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  while (!Serial) {
    // Wait for serial port to connect.
    // You don't have to do this in your game. This is only for debugging
    // purposes, so that you can see the output in the serial console.
    ;
  }

  String fv = BP32.firmwareVersion();
  Serial.print("Firmware version installed: ");
  Serial.println(fv);

  // To get the BD Address (MAC address) call:
  const uint8_t* addr = BP32.localBdAddress();
  Serial.print("BD Address: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(addr[i], HEX);
    if (i < 5)
      Serial.print(":");
    else
      Serial.println();
  }

  // BP32.pinMode(27, OUTPUT);
  // BP32.digitalWrite(27, 0);

  // This call is mandatory. It sets up Bluepad32 and creates the callbacks.
  BP32.setup(&onConnectedController, &onDisconnectedController);

  // "forgetBluetoothKeys()" should be called when the user performs
  // a "device factory reset", or similar.
  // Calling "forgetBluetoothKeys" in setup() just as an example.
  // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
  // But it might also fix some connection / re-connection issues.
  BP32.forgetBluetoothKeys();
}

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
  bool foundEmptySlot = false;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      Serial.print("CALLBACK: Controller is connected, index=");
      Serial.println(i);
      myControllers[i] = ctl;
      foundEmptySlot = true;

      // Optional, once the gamepad is connected, request further info about the
      // gamepad.
      ControllerProperties properties = ctl->getProperties();
      char buf[80];
      sprintf(buf,
              "BTAddr: %02x:%02x:%02x:%02x:%02x:%02x, VID/PID: %04x:%04x, "
              "flags: 0x%02x",
              properties.btaddr[0], properties.btaddr[1], properties.btaddr[2],
              properties.btaddr[3], properties.btaddr[4], properties.btaddr[5],
              properties.vendor_id, properties.product_id, properties.flags);
      Serial.println(buf);
      break;
    }
  }
  if (!foundEmptySlot) {
    Serial.println(
        "CALLBACK: Controller connected, but could not found empty slot");
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  bool foundGamepad = false;

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Serial.print("CALLBACK: Controller is disconnected from index=");
      Serial.println(i);
      myControllers[i] = nullptr;
      foundGamepad = true;
      break;
    }
  }

  if (!foundGamepad) {
    Serial.println(
        "CALLBACK: Controller disconnected, but not found in myControllers");
  }
}

void processGamepad(ControllerPtr gamepad) {
  // Declaramos 'data' como char para almacenar la dirección de la cruceta
  char data = 'C';  // Centro por defecto

  // Usa bitwise AND para verificar cada dirección
  int dpadState = gamepad->dpad();
  
  if (dpadState & 0x01) {
    data = 'U';  // Arriba
  } else if (dpadState & 0x02) {
    data = 'D';  // Abajo
  } else if (dpadState & 0x04) {
    data = 'R';  // Derecha
  } else if (dpadState & 0x08) {
    data = 'L';  // Izquierda
  }

  // Enviar el valor de data por el puerto serial
  Serial2.println(data);
  Serial.println(data);
}


void loop() {
  // Actualizar el estado de los controladores
  BP32.update();

  // Recorrer todos los controladores y procesarlos si están conectados
  for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
    ControllerPtr myController = myControllers[i];

    if (myController && myController->isConnected()) {
      // Si el controlador es un gamepad, procesarlo
      if (myController->isGamepad()) {
        processGamepad(myController); // Llamamos a la función para procesar el gamepad
      }
    }
  }

  delay(150);
}
