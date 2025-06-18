#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <Bounce2.h>
#include <LiquidCrystal_I2C.h>
#include <stdio.h>
#include <string.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "internet.h"

// ────────────────────────────────
//  Configurações Wi‑Fi
// ────────────────────────────────
const char *WIFI_SSID = "SALA 09";
const char *WIFI_PASS = "info@134";

// ────────────────────────────────
//  Configurações MQTT
// ────────────────────────────────
WiFiClient espClient;
PubSubClient client(espClient);

const char *mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char *mqtt_id = "esp32-senai134-julia";
const char *mqtt_topic_sub = "senai134/mesa05/projetoArProjPub";
// Note: nesta versão, o dispositivo apenas recebe mensagens MQTT.

// ────────────────────────────────
//  Funções Wi‑Fi simples (não bloqueantes)
// ────────────────────────────────
void conectaWiFi()
{
    if (WiFi.status() == WL_CONNECTED)
        return;
    Serial.println("Conectando ao WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000)
    {
        delay(100);
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("WiFi conectado!");
    }
    else
    {
        Serial.println("Falha ao conectar ao WiFi.");
    }
}

void checkWiFi()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        conectaWiFi();
    }
}

bool modulo = false; // false = AC, true = projetor
//
struct Air
{
    bool ligado = false;     // Estado do AC (ligado/desligado)
    int velocidade = 0;      // Velocidade do ventilador (0-3)
    float temperatura = 0.0; // Temperatura configurada (em graus Celsius)
};

// Array para 4 aparelhos AC
Air acs[4]; // Inicializa 4 ACs com valores padrão

// Projetor struct
struct Projetor
{
    bool ligado = false;    // Estado do projetor (ligado/desligado)
    bool congelado = false; // Estado de congelamento do projetor (ligado/desligado)
} projetor;

// ────────────────────────────────
//  ESTRUTURAS DE ESTADO
// ────────────────────────────────
struct AirState
{
    int temperatura = 24;
    int velocidade = 1;
    int unidade = 1;
    bool controlarTemp = true;
};

struct ProjState
{
    int unidade = 1;
    bool congelado = false;
};

AirState air;
ProjState proj;

// Dispositivo selecionado: 0 = AR, 1 = Projetor
int selectedDevice = 0;

// Variáveis para travamento e atualização
bool travado = false;
bool aguardandoTrava = false;
unsigned long tempoTrava = 0;

// ────────────────────────────────
//  CONFIGURAÇÕES DE HARDWARE
// ────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 4);

const uint16_t kIrLed = 4;
IRsend irsend(kIrLed);

const int botoes[] = {12, 13, 14, 15, 5, 19, 23};
Bounce btn[7];

// ────────────────────────────────
//  FUNÇÃO DE ATUALIZAÇÃO DO LCD
// ────────────────────────────────
void atualizarLCD()
{
    char arHeader[9];
    char pjHeader[9];
    char header[17];

    char seta_ar = (selectedDevice == 0) ? '>' : ' ';
    char unidadeAr[3];
    if (air.unidade == 0)
        strcpy(unidadeAr, "TD");
    else
        sprintf(unidadeAr, "%d", air.unidade);
    sprintf(arHeader, "%cAR:%s", seta_ar, unidadeAr);
    int len = strlen(arHeader);
    for (int i = len; i < 8; i++)
        arHeader[i] = ' ';
    arHeader[8] = '\0';

    char seta_pj = (selectedDevice == 1) ? '>' : ' ';
    char unidadePj[3];
    if (proj.unidade == 0)
        strcpy(unidadePj, "TD");
    else
        sprintf(unidadePj, "%d", proj.unidade);
    sprintf(pjHeader, "%cPJ:%s", seta_pj, unidadePj);
    len = strlen(pjHeader);
    for (int i = len; i < 8; i++)
        pjHeader[i] = ' ';
    pjHeader[8] = '\0';

    sprintf(header, "%-8s%-8s", arHeader, pjHeader);

    char rowTemp[17];
    char bullet = (selectedDevice == 0 && air.controlarTemp) ? 'o' : ' ';
    sprintf(rowTemp, "%cT:%02d", bullet, air.temperatura);
    len = strlen(rowTemp);
    for (int i = len; i < 16; i++)
        rowTemp[i] = ' ';
    rowTemp[16] = '\0';

    char rowVel[17];
    bullet = (selectedDevice == 0 && !air.controlarTemp) ? 'o' : ' ';
    sprintf(rowVel, "%cV:%d", bullet, air.velocidade);
    len = strlen(rowVel);
    for (int i = len; i < 16; i++)
        rowVel[i] = ' ';
    rowVel[16] = '\0';

    char rowFreeze[17];
    char freezeStatus[9];
    if (proj.congelado)
        strcpy(freezeStatus, "FRZ");
    else
        strcpy(freezeStatus, "----");
    len = strlen(freezeStatus);
    for (int i = len; i < 8; i++)
        freezeStatus[i] = ' ';
    freezeStatus[8] = '\0';
    sprintf(rowFreeze, "%-8s%-8s", " ", freezeStatus);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(header);
    lcd.setCursor(0, 1);
    lcd.print(rowTemp);
    lcd.setCursor(0, 2);
    lcd.print(rowVel);
    lcd.setCursor(0, 3);
    lcd.print(rowFreeze);
}

// ────────────────────────────────
//  RECEBIMENTO E TRATAMENTO DE COMANDOS MQTT
// ────────────────────────────────
void tratarMensagemIR(JsonDocument &doc)
{

    // Atualiza os parâmetros do AR, se presentes
    if (!doc["unidade"].isNull())
    {
        air.unidade = doc["unidade"];
    }
    if (!doc["temperatura"].isNull())
    {
        air.temperatura = doc["temperatura"];
    }
    if (!doc["velocidade"].isNull())
    {
        air.velocidade = doc["velocidade"];
    }

    // Para o projetor, altera o status de congelamento e a unidade, se presentes
    if (!doc["freeze"].isNull())
    {
        const char *frz = doc["freeze"];
        proj.congelado = (strcmp(frz, "on") == 0);
    }
    if (!doc["pjunid"].isNull())
    {
        proj.unidade = doc["pjunid"];
    }

    atualizarLCD();


}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    //   Serial.printf("Mensagem recebida no tópico %s: ", topic);
    //   DynamicJsonDocument doc(512);
    //   DeserializationError error = deserializeJson(doc, payload, length);
    //   if (error) {
    //     Serial.print("Erro ao deserializar: ");
    //     Serial.println(error.c_str());
    //     return;
    //   }
    //   serializeJsonPretty(doc, Serial);
    //   Serial.println();
    //   tratarMensagemIR(doc);
    Serial.printf("Mensagem recebida no tópico: %s\n", topic); // Exibe o tópico da mensagem recebida

    String mensagemRecebida;
    for (unsigned int i = 0; i < length; i++)
    {
        mensagemRecebida += (char)payload[i];
    }
    Serial.println("Payload: " + mensagemRecebida);

    JsonDocument doc;                                                   // Cria um documento JSON para armazenar os dados recebidos
    DeserializationError erro = deserializeJson(doc, mensagemRecebida); // Deserializa a mensagem JSON recebida
    if (erro)
    {
        Serial.print("Erro no JSON: "); // Exibe erro se a deserialização falhar
        Serial.println(erro.c_str());   // Exibe a mensagem de erro
        return;
    }

    if (!modulo) // AC
    {            // AC
        // Para cada AC esperado: ac1, ac2, ac3, ac4
        for (int i = 0; i < 4; i++)
        {
            String acKey = "ac" + String(i + 1);
            if (!doc[acKey].isNull())
            {
                JsonObject acObj = doc[acKey].as<JsonObject>();
                acs[i].ligado = acObj["ligado"] | acs[i].ligado;                // Atualiza o estado do AC
                acs[i].velocidade = acObj["velocidade"] | acs[i].velocidade;    // Atualiza a velocidade do ventilador
                acs[i].temperatura = acObj["temperatura"] | acs[i].temperatura; // Atualiza a temperatura configurada
            }
        }
    }
    else
    {                                                               // Projetor
        projetor.ligado = doc["ligado"] | projetor.ligado;          // Atualiza o estado do projetor
        projetor.congelado = doc["congelado"] | projetor.congelado; // Atualiza o estado de congelamento do projetor
    }

    Serial.println("=== Dados atualizados via MQTT ===");
    if (!modulo) // Se for AC
    {
        for (int i = 0; i < 4; i++)
        {
            Serial.printf("AC%d - Ligado: %s, Velocidade: %d, Temperatura: %.1f, Unidade: %s\n",
                          i + 1,
                          acs[i].ligado ? "Sim" : "Não",
                          acs[i].velocidade,
                          acs[i].temperatura); // Exibe os dados de cada AC
        }
    }
    else // Se for Projetor
    {
        Serial.printf("Projetor - Ligado: %s, Congelado: %s\n",
                      projetor.ligado ? "Sim" : "Não",
                      projetor.congelado ? "Sim" : "Não"); // Exibe os dados do projetor
    }
}

// ────────────────────────────────
//  CONEXÃO MQTT (não bloqueante)
// ────────────────────────────────
unsigned long lastMqttAttempt = 0;
void mqttConnect()
{
    if (client.connected())
        return;
    if (millis() - lastMqttAttempt < 5000)
        return; // tenta novamente somente a cada 5s
    lastMqttAttempt = millis();
    Serial.println("Conectando ao MQTT...");
    if (client.connect(mqtt_id))
    {
        Serial.println("Conectado ao MQTT!");
        client.subscribe(mqtt_topic_sub);
    }
    else
    {
        Serial.print("Falha, rc=");
        Serial.println(client.state());
    }
}

// ────────────────────────────────
//  PROCESSAMENTO DOS BOTÕES
// ────────────────────────────────
void processarBotoes()
{
    unsigned long agora = millis();
    for (int i = 0; i < 7; i++)
    {
        btn[i].update();
    }

    // Botão 6: travamento (pressione por 5 segundos)
    if (btn[6].read() == LOW)
    {
        if (!aguardandoTrava)
        {
            tempoTrava = agora;
            aguardandoTrava = true;
        }
        else if (agora - tempoTrava >= 5000)
        {
            travado = !travado;
            aguardandoTrava = false;
            atualizarLCD();
            return;
        }
    }
    else
    {
        aguardandoTrava = false;
    }

    if (travado)
        return;

    if (btn[0].fell())
    {
        selectedDevice = (selectedDevice == 0) ? 1 : 0;
        atualizarLCD();
        return;
    }

    if (selectedDevice == 0)
    { // Controle para AR
        if (btn[1].fell())
        {
            air.controlarTemp = !air.controlarTemp;
        }
        if (btn[2].fell())
        {
            if (air.controlarTemp && air.temperatura < 30)
                air.temperatura++;
            else if (!air.controlarTemp && air.velocidade < 3)
                air.velocidade++;
        }
        if (btn[3].fell())
        {
            if (air.controlarTemp && air.temperatura > 18)
                air.temperatura--;
            else if (!air.controlarTemp && air.velocidade > 1)
                air.velocidade--;
        }
        if (btn[4].fell())
        {
            air.unidade = (air.unidade + 1) % 5;
        }
    }
    else
    { // Controle para Projetor
        if (btn[4].fell())
        {
            proj.unidade = (proj.unidade + 1) % 3;
        }
        if (btn[5].fell())
        {
            proj.congelado = !proj.congelado;
        }
    }
    atualizarLCD();
}

// ────────────────────────────────
//  SETUP
// ────────────────────────────────
void setup()
{
    Serial.begin(115200);

    // Conecta ao Wi‑Fi
    conectaWiFi();

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(mqttCallback);

    irsend.begin();

    lcd.init();
    lcd.backlight();
    atualizarLCD();

    for (int i = 0; i < 7; i++)
    {
        pinMode(botoes[i], INPUT_PULLUP);
        btn[i].attach(botoes[i]);
        btn[i].interval(25);
    }
}

// ────────────────────────────────
//  LOOP
// ────────────────────────────────
void loop()
{
    processarBotoes();
    checkWiFi();

    mqttConnect();
    client.loop();

    // Nesta versão, não há publicação de mensagens – o dispositivo apenas recebe comandos MQTT
}