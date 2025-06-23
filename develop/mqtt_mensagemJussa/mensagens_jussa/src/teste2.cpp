#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Bounce2.h>
#include "internet.h"

// ====== CONFIGURAÇÕES WI-FI / MQTT ======
const char *mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char *mqtt_id = "esp32-senai134-jul"; // id = nome do dispositivo
const char *mqtt_topic_sub = "notificacoes";
const char *mqtt_topic_pub = "notificacoes/estado";

// ====== HARDWARE ======
constexpr uint8_t pinLed = 2;
constexpr uint8_t botaoMsg = 0;

// ====== OBJETOS ======
WiFiClient espClient;
PubSubClient client(espClient);
Bounce botao = Bounce();

JsonDocument doc;

// ====== ESTADOS ======
bool reuniaoAtiva = false;
bool reuniaoAnterior = false;
bool mensagemAtiva = false;
unsigned long tempoAnterior = 0;
bool estadoLed = false;

void mqttConnect()
{
  while (!client.connected())
  {
    Serial.print("Conectando ao MQTT...");
    if (client.connect(mqtt_id))
    {
      Serial.println("Conectado com sucesso!");
      client.subscribe(mqtt_topic_sub);
    }
    else
    {
      Serial.print("Falha, rc=");
      Serial.println(client.state());
      Serial.println("tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void mqttCallback(char *topic, byte *payLoad, unsigned int length)
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

  if (!doc["reuniao"].isNull())
  {
    reuniaoAtiva = doc["reuniao"];
  }

  if (!doc["mensagem"].isNull())
  {
    mensagem = doc["mensagem"].as<String>();
    mensagemAtiva = true;
  }
}

void enviarMensagemLida()
{
  JsonDocument resp;
  resp["id"] = (mqtt_id);
  resp["resposta"] = "mensagem lida";

  char buffer[64];
  serializeJson(resp, buffer);
  client.publish(mqtt_topic_pub, buffer);
  Serial.println("Resposta enviada: mensagem lida");
}

void setup()
{
  Serial.begin(9600);
  pinMode(pinLed, OUTPUT);
  pinMode(botaoMsg, INPUT_PULLUP);

  botao.attach(botaoMsg);
  botao.interval(25);

  conectaWiFi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);

  Serial.println("Sistema iniciado");
}

void loop()
{
  // === Reconectar MQTT se cair ===
  if (!client.connected())
  {
    mqttConnect();
  }

  client.loop();
  botao.update();

  // === Reunião ativa: piscar LED ===
  if (reuniaoAtiva)
  {
    if (millis() - tempoAnterior >= 500)
    {
      estadoLed = !estadoLed;
      digitalWrite(pinLed, estadoLed);
      tempoAnterior = millis();
    }

    if (reuniaoAtiva && reuniaoAnterior == 0)
    {
      Serial.println("Reunião!");
      reuniaoAnterior = reuniaoAtiva;
    }
  }
  // === Mensagem ativa: LED aceso constante ===
  else if (mensagemAtiva)
  {
    digitalWrite(pinLed, HIGH);
  }
  // === Nenhum estado: LED desligado ===
  else
  {
    digitalWrite(pinLed, LOW);
  }

  // === Botão pressionado ===
  if (botao.fell())
  {
    digitalWrite(pinLed, LOW);
    enviarMensagemLida();
    reuniaoAtiva = false;
    mensagemAtiva = false;
  }
}
