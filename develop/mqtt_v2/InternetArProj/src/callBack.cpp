#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "ir_control.cpp"
#include "codgiuliano.cpp"
#include <Bounce2.h>



void callBack(char *topic, byte *payLoad, unsigned int length)
{
  Serial.printf("Mensagem recebida em %s: ", topic);

  String mensagem = "";
  for (unsigned int i = 0; i < length; i++)
  {
    mensagem += (char)payLoad[i];
  }
  Serial.println(mensagem);

  JsonDocument doc;
  deserializeJson(doc, mensagem);


if (!doc["Alternar"].isNull() && doc["Alternar"] == true) {
  selectedDevice = (selectedDevice == 0) ? 1 : 0;
  atualizarLCD();
}

  /*  if (btn[0].fell()) {
    selectedDevice = (selectedDevice == 0) ? 1 : 0;
    atualizarLCD();
    return;
  }*/

  // Lógica de IR
  tratarMensagemIR(doc);
}
