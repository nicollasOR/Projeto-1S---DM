
#include <WiFi.h>
#include "internet.h"


#include "senhas.h"



void conectaWiFi() {
  const unsigned long tempoEsperaConexao = 20000;
  const unsigned long tempoEsperaReconexao = 10000;


unsigned long tempoInicialWiFi = millis();
  Serial.print("Conectando-se ao Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED && millis() - tempoInicialWiFi < tempoEsperaConexao) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado!");
}


void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado. Reconectando...");
    conectaWiFi();
  }
}

