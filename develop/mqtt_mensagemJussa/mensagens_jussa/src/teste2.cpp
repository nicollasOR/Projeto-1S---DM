#include <Arduino.h>              // Biblioteca principal do Arduino
#include <WiFi.h>                 // Biblioteca para conexão Wi-Fi
#include <PubSubClient.h>         // Biblioteca para comunicação MQTT
#include <ArduinoJson.h>          // Biblioteca para manipulação de JSON
#include <Bounce2.h>              // Biblioteca para fazer debounce do botão
#include "internet.h"             // Arquivo com a função conectaWiFi()

// ====== CONFIGURAÇÕES WI-FI / MQTT ======
const char *mqttServer = "broker.hivemq.com";             // Endereço do broker MQTT
const int mqttPort = 1883;                                // Porta padrão do MQTT
const char *mqtt_id = "esp32-senai134-sala09-10";               // ID único do dispositivo no MQTT
const char *mqtt_topic_sub = "notificacoes";              // Tópico para receber mensagens
const char *mqtt_topic_pub = "notificacoes/estado";       // Tópico para enviar mensagens

// ====== HARDWARE ======
constexpr uint8_t pinLed = 2;      // Pino do LED integrado (GPIO 2)
constexpr uint8_t botaoMsg = 0;    // Pino do botão (GPIO 0)

// ====== OBJETOS ======
WiFiClient espClient;             // Cliente Wi-Fi
PubSubClient client(espClient);   // Cliente MQTT usando Wi-Fi
Bounce botao = Bounce();          // Objeto para tratamento de botão com debounce
JsonDocument doc;                 // Documento JSON para armazenar dados recebidos

// ====== ESTADOS ======
bool reuniaoAtiva = false;        // Indica se uma reunião está ativa
bool reuniaoAnterior = false;     // Armazena o estado anterior da reunião
bool mensagemAtiva = false;       // Indica se uma mensagem foi recebida
unsigned long tempoAnterior = 0;  // Guarda o tempo da última mudança no LED
bool estadoLed = false;           // Estado atual do LED (ligado/desligado)

// ====== Função para conectar ao broker MQTT ======
void mqttConnect()
{
  while (!client.connected()) // Enquanto não estiver conectado ao MQTT
  {
    Serial.print("Conectando ao MQTT...");
    if (client.connect(mqtt_id)) // Tenta conectar com o ID do dispositivo
    {
      Serial.println("Conectado com sucesso!");
      client.subscribe(mqtt_topic_sub); // Inscreve no tópico para receber mensagens
    }
    else
    {
      Serial.print("Falha, rc=");
      Serial.println(client.state()); // Mostra o erro da conexão
      Serial.println("tentando novamente em 5 segundos");
      delay(5000); // Aguarda 5 segundos para tentar novamente
    }
  }
}

// ====== Função chamada quando uma mensagem MQTT chega ======
void mqttCallback(char *topic, byte *payLoad, unsigned int length)
{
  Serial.printf("Mensagem recebida em %s: ", topic);

  String mensagem = "";
  for (unsigned int i = 0; i < length; i++)
  {
    mensagem += (char)payLoad[i]; // Constrói a string da mensagem recebida
  }
  Serial.println(mensagem);

  JsonDocument doc;
  deserializeJson(doc, mensagem); // Converte a string em objeto JSON

  if (!doc["reuniao"].isNull())
  {
    reuniaoAtiva = doc["reuniao"]; // Atualiza estado de reunião se presente na mensagem
  }

  if (!doc["mensagem"].isNull())
  {
    mensagem = doc["mensagem"].as<String>(); // Lê o conteúdo da mensagem
    mensagemAtiva = true; // Ativa o estado de mensagem
  }
}

// ====== Envia mensagem confirmando que a mensagem foi lida ======
void enviarMensagemLida()
{
  JsonDocument resp;
  resp["id"] = (mqtt_id); // Adiciona o ID do dispositivo
  resp["resposta"] = "mensagem lida"; // Mensagem de confirmação

  char buffer[64];
  serializeJson(resp, buffer); // Converte JSON em string
  client.publish(mqtt_topic_pub, buffer); // Publica no tópico MQTT
  Serial.println("Resposta enviada: mensagem lida");
}

// ====== Configuração inicial ======
void setup()
{
  Serial.begin(9600); // Inicializa comunicação serial
  pinMode(pinLed, OUTPUT); // Define o LED como saída
  pinMode(botaoMsg, INPUT_PULLUP); // Define o botão com pull-up interno

  botao.attach(botaoMsg); // Conecta o botão ao objeto Bounce
  botao.interval(25);     // Define intervalo de debounce

  conectaWiFi(); // Conecta à rede Wi-Fi (implementada em internet.h)
  client.setServer(mqttServer, mqttPort); // Configura o servidor MQTT
  client.setCallback(mqttCallback); // Define a função para mensagens recebidas

  Serial.println("Sistema iniciado");
}

// ====== Loop principal do programa ======
void loop()
{
  // === Reconecta ao MQTT se estiver desconectado ===
  if (!client.connected())
  {
    mqttConnect();
  }

  client.loop(); // Mantém o cliente MQTT ativo
  botao.update(); // Atualiza o estado do botão

  // === Se reunião estiver ativa, pisca o LED ===
  if (reuniaoAtiva)
  {
    if (millis() - tempoAnterior >= 500) // Pisca a cada 500ms
    {
      estadoLed = !estadoLed;
      digitalWrite(pinLed, estadoLed);
      tempoAnterior = millis();
    }

    if (reuniaoAtiva && reuniaoAnterior == 0)
    {
      Serial.println("Reunião!"); // Exibe mensagem uma vez ao iniciar reunião
      reuniaoAnterior = reuniaoAtiva;
    }
  }
  // === Se recebeu uma mensagem, mantém LED aceso ===
  else if (mensagemAtiva)
  {
    digitalWrite(pinLed, HIGH);
  }
  // === Caso contrário, LED fica apagado ===
  else
  {
    digitalWrite(pinLed, LOW);
  }

  // === Quando o botão for pressionado ===
  if (botao.fell()) // Detecta borda de descida (botão pressionado)
  {
    digitalWrite(pinLed, LOW); // Apaga o LED
    enviarMensagemLida();      // Envia confirmação por MQTT
    reuniaoAtiva = false;      // Reseta estado de reunião
    mensagemAtiva = false;     // Reseta estado de mensagem
  }
}
