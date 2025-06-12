#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "internet.h"
#include "ir_control.h"



WiFiClient espClient;
PubSubClient client(espClient);


const char *mqtt_server = "broker.hivemq.com"; //endereco do broker, pode ser um ip, nds
const int mqtt_port = 1883; //porta de comunicacao sem seguranca
const char *mqtt_id = "esp32-senai134-julia"; //id = nome do dispositivo
const char *mqtt_topic_sub = "senai134/mesa05/projetoAProjSub"; //o esp esta ouvindo este topico, 
const char *mqtt_topic_pub = "senai134/mesa05/projetoAProjPub"; //o esp publica mensagens

const int botoes[] = {12, 13, 14, 15, 5, 19 ,23 };

void callBack(char *, byte *, unsigned int);
void mqttConnect(void);

int tempoEspera = 0;

bool envioMqtt = false;

unsigned long ultimoEnvio = 0;
unsigned long intervaloEnvio = 3000;

unsigned long msgRecebida;

void setup()
{
  Serial.begin(9600);

  inicializarIR();

  conectaWiFi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callBack);
}

void loop()
{
  checkWiFi();
  if (!client.connected())
    mqttConnect();

  client.loop();

  JsonDocument doc;
  String mensagem;



}

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

  // Lógica de IR
  tratarMensagemIR(doc);
}

void mqttConnect(void)
{
  while (!client.connected())
  {
    Serial.println("Conectando ao MQTT...");

    if (client.connect(mqtt_id))
    {
      Serial.println("Conectado com sucesso");
      client.subscribe(mqtt_topic_sub);
    }
    else
    {
      Serial.print("Falha, rc = ");

      Serial.println(client.state());

      Serial.println("Tentando novamente em 5 segundos...");

      delay(5000);
    }
  }
}
