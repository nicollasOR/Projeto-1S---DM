#include <WiFi.h>
#include "internet.h"
#include "senhas.h"

// const char *WIFI_SSID = "SALA 09";
// const char *WIFI_PASS = "info@134";


void conectaWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado. IP: " + WiFi.localIP().toString());
}

void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado. Reconectando...");
    conectaWiFi();
  }
}