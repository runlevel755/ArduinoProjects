#include <IRremote.hpp>

#define IR_RECEIVE_PIN 2

void setup() {
  Serial.begin(115200);

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  Serial.println("RECEPTOR IR - Nano");
}

void loop() {
  if (IrReceiver.decode()) {

    Serial.println("========== SEÑAL RECIBIDA ==========");

    IrReceiver.printIRResultShort(&Serial);

    Serial.println();

    IrReceiver.resume();
  }
}