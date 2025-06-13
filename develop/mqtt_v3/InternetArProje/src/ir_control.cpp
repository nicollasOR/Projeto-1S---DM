#include "ir_control.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>

const uint16_t kIrLed = 4;  // Altere para o pino correto do seu LED IR
IRsend irsend(kIrLed);

void inicializarIR() {
  irsend.begin();
}

void tratarMensagemIR(JsonDocument &doc) {
  if (doc.containsKey("raw") && doc.containsKey("len")) {
    JsonArray raw = doc["raw"];
    uint16_t len = doc["len"];

    if (len > 0 && raw.size() >= len) {
      uint16_t rawData[raw.size()];
      for (int i = 0; i < len; i++) {
        rawData[i] = raw[i];
      }
      irsend.sendRaw(rawData, len, 38); // frequência IR padrão
      Serial.println("Sinal IR enviado.");
    }
  }
}
