
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <Bounce2.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "internet.h"
#include <ir_Fujitsu.h>

//  Wi‑Fi e MQTT
const char *WIFI_SSID = "SALA 09";
const char *WIFI_PASS = "info@134";
const char *mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char *mqtt_id = "esp32-senai134-julia";
const char *mqtt_topic_sub = "senai134/mesa05/projetoArProjPub";

WiFiClient espClient;
PubSubClient client(espClient);

struct Air {
  bool ligado = false;
  int velocidade = 0;
  float temperatura = 0.0;
};

struct Projetor {
  bool ligado = false;
  bool congelado = false;
};

struct AirState {
  int temperatura = 24;
  int velocidade = 1;
  int unidade = 1;
  bool controlarTemp = true;
};

struct ProjState {
  int unidade = 1;
  bool congelado = false;
};

Air acs[4];
Projetor projetor;
AirState air;
ProjState proj;
bool modulo = false;
int selectedDevice = 0;

//  Pinos e IR
#define IR_LED_AC1 4
#define IR_LED_AC2 5
#define IR_LED_PROJ 18
#define BTN_AC1 22
#define BTN_AC2 21
#define BTN_PROJ 23

const int botoes[] = {12, 13, 14, 15, 5, 19, 23};
Bounce btn[7];
Bounce btnAC1 = Bounce();
Bounce btnAC2 = Bounce();
Bounce btnProj = Bounce();

bool ac1On = false;
bool ac2On = false;
bool ac3On = false;
bool ac4On = false;

bool projOn = false;
unsigned long projPressedAt = 0;
bool projHeld = false;

LiquidCrystal_I2C lcd(0x27, 16, 4);
IRsend irsend(IR_LED_AC1);
IRsend irsendProj(IR_LED_PROJ);
IRFujitsuAC ac1(IR_LED_AC1);
IRFujitsuAC ac2(IR_LED_AC2);

//  Códigos projetor para IR
const uint16_t RAW_POWER[] PROGMEM = {
  9062, 4382, 668, 1580, 668, 458, 670, 456, 670, 454, 646, 478, 672, 456,
  670, 456, 668, 1578, 674, 1576, 672, 1576, 674, 452, 646, 480, 670, 456,
  666, 462, 668, 456, 668, 458, 672, 454, 670, 456, 670, 456, 670, 456,
  672, 1576, 674, 1574, 674, 1576, 672, 1576, 672, 1576, 672, 1578, 670,
  1578, 670, 1580, 674, 452, 668, 458, 668, 458, 668, 456, 668, 43768,
  9088, 4358, 668, 1580, 668, 1580, 666, 460, 668, 458, 668, 460, 666, 458,
  668, 458, 666, 1582, 666, 1582, 668, 458, 666, 1582, 666, 460, 666, 1582,
  668, 460, 668, 1578, 668, 460, 664, 462, 664, 462, 664, 462, 662, 462,
  664, 1584, 664, 466, 658, 492, 634, 1586, 664, 1584, 664, 1586, 662, 1584,
  662, 1586, 660, 494, 632, 1588, 660, 1588, 660, 494, 632
};

const uint16_t RAW_FREEZE[] PROGMEM = {
  9080, 4364, 644, 1604, 644, 482, 642, 482, 642, 484, 642, 484,
  668, 456, 668, 460, 640, 1606, 672, 1578, 668, 1580, 642, 484,
  668, 460, 666, 460, 668, 458, 642, 484, 668, 458, 644, 482,
  670, 456, 668, 458, 668, 458, 642, 1606, 668, 1580, 670, 1580,
  642, 1606, 670, 1580, 670, 1578, 668, 1582, 666, 1582, 668,
  458, 670, 458, 668, 460, 668, 458, 666, 43782, 9086, 4362, 666,
  1580, 666, 1582, 666, 460, 664, 460, 666, 460, 668, 458, 666,
  462, 662, 1584, 666, 1584, 666, 462, 664, 1584, 666, 462, 664,
  1584, 662, 464, 664, 1584, 664, 464, 662, 464, 662, 1586,
  662, 464, 660, 468, 658, 1588, 662, 464, 660, 466, 660, 1588,
  662, 1588, 658, 496, 630, 1590, 658, 1590, 658, 498, 628,
  1592, 656, 1590, 658, 498, 628
};

//  Funções 
void iniciarAC(IRFujitsuAC &ac) {
  ac.begin();
  ac.setModel(ARRAH2E);
  ac.setId(0);
}

void configurarAC(IRFujitsuAC &ac, uint8_t temp, uint8_t modo, uint8_t fan) {
  ac.setTemp(temp);
  ac.setMode(modo);
  ac.setFanSpeed(fan);
  ac.setSwing(kFujitsuAcSwingOff);
  ac.setPower(true);
  ac.send();
}

void desligarAC(IRFujitsuAC &ac) {
  ac.setPower(false);
  ac.send();
}

void ligarProjetor() {
  irsendProj.sendRaw(RAW_POWER, sizeof(RAW_POWER) / sizeof(RAW_POWER[0]), 38);
  Serial.println(F("Projetor: POWER enviado"));
}

void congelarProjetor() {
  irsendProj.sendRaw(RAW_FREEZE, sizeof(RAW_FREEZE) / sizeof(RAW_FREEZE[0]), 38);
  Serial.println(F("Projetor: FREEZE enviado"));
}

//  Funções de Wi-Fi e MQTT
void conectaWiFi() {
  Serial.print("Conectando-se ao Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado!");
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);

  String mensagem;
  for (unsigned int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }

  Serial.print("Conteúdo: ");
  Serial.println(mensagem);

  // Interpretar a mensagem JSON
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, mensagem);

  if (error) {
    Serial.print(F("Falha ao interpretar JSON: "));
    Serial.println(error.f_str());
    return;
  }

  const char* dispositivo = doc["dispositivo"];
  const char* acao = doc["acao"];

  if (strcmp(dispositivo, "ac1") == 0) {
    if (strcmp(acao, "ligar") == 0) {
      configurarAC(ac1, 24, kFujitsuAcModeCool, kFujitsuAcFanQuiet);
      ac1On = true;
      Serial.println(F("AC1: LIGADO"));
    } else if (strcmp(acao, "desligar") == 0) {
      desligarAC(ac1);
      ac1On = false;
      Serial.println(F("AC1: DESLIGADO"));
    }
  } 
  
  if (strcmp(dispositivo, "ac2") == 0) {
    if (strcmp(acao, "ligar") == 0) {
      configurarAC(ac2, 24, kFujitsuAcModeCool, kFujitsuAcFanQuiet);
      ac1On = true;
      Serial.println(F("AC2: LIGADO"));
    } else if (strcmp(acao, "desligar") == 0) {
      desligarAC(ac2);
      ac1On = false;
      Serial.println(F("AC2: DESLIGADO"));
    }
  
  } 
   if(strcmp(dispositivo, "ac3") == 0)
   {
    if (strcmp(acao, "ligar") == 0) {
      configurarAC(ac3, 24, kFujitsuAcModeCool, kFujitsuAcFanQuiet);
      ac3On = true;
      Serial.println(F("AC3: LIGADO"));
    } else if (strcmp(acao, "desligar") == 0) {
      desligarAC(ac3);
      ac3On = false;
      Serial.println(F("AC3: DESLIGADO"));
    }
  } 
    if (strcmp(dispositivo, "ac4") == 0)
     {
    if (strcmp(acao, "ligar") == 0) {
      configurarAC(ac4, 24, kFujitsuAcModeCool, kFujitsuAcFanQuiet);
      ac4On = true;
      Serial.println(F("AC4: LIGADO"));
    } else if (strcmp(acao, "desligar") == 0) {
      desligarAC(ac4);
      ac4On = false;
      Serial.println(F("AC4: DESLIGADO"));
    }
   }
  else if (strcmp(dispositivo, "proj") == 0) {
    if (strcmp(acao, "ligar") == 0) {
      ligarProjetor();
      projOn = true;
      Serial.println(F("Projetor: LIGADO"));
    } else if (strcmp(acao, "congelar") == 0) {
      congelarProjetor();
      projetor.congelado = true;
      Serial.println(F("Projetor: CONGELADO"));
    } else if (strcmp(acao, "descongelar") == 0) {
      projetor.congelado = false;
      Serial.println(F("Projetor: DESCONGELADO"));
    }
  }
}

void mqttConnect() {
  if (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect(mqtt_id)) {
      Serial.println("Conectado!");
      client.subscribe(mqtt_topic_sub);
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    conectaWiFi();
  }
}

void atualizarLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AC1: ");
  lcd.print(ac1On ? "LIGADO" : "DESLIGADO");

  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(ac1.getTemp());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AC2: ");
  lcd.print(ac1On ? "LIGADO" : "DESLIGADO");

  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(ac1.getTemp());

  lcd.setCursor(0, 2);
  lcd.print("Proj: ");
  lcd.print(projOn ? "LIGADO" : "DESLIGADO");

  lcd.setCursor(0, 3);
  lcd.print("Congelado: ");
  lcd.print(projetor.congelado ? "SIM" : "NAO");
}

void setup() {
  Serial.begin(115200);
  conectaWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
  irsend.begin();
  iniciarAC(ac1);
  iniciarAC(ac2);
  irsendProj.begin();

  lcd.init();
  lcd.backlight();
  atualizarLCD();

  for (int i = 0; i < 7; i++) {
    pinMode(botoes[i], INPUT_PULLUP);
    btn[i].attach(botoes[i]);
    btn[i].interval(25);
  }

  btnAC1.attach(BTN_AC1, INPUT_PULLUP);
  btnAC1.interval(25);
  btnAC2.attach(BTN_AC1, INPUT_PULLUP);
  btnAC2.interval(25);
  btnProj.attach(BTN_PROJ, INPUT_PULLUP);
  btnProj.interval(25);

  Serial.println(F("Sistema IR inicializado"));
}

void loop() {
  checkWiFi();

  if (!client.connected()) {
    mqttConnect();
  }

  client.loop();

  btnAC1.update();
  btnAC2.update();
  btnProj.update();

  if (btnAC1.fell()) {
    ac1On = !ac1On;
    if (ac1On)
      configurarAC(ac1, 24, kFujitsuAcModeCool, kFujitsuAcFanQuiet);
    else
      desligarAC(ac1);
    Serial.println(ac1On ? F("AC1: LIGADO") : F("AC1: DESLIGADO"));
  }

  if (btnAC2.fell()) {
    ac2On = !ac2On;
    if (ac2On)
      configurarAC(ac2, 24, kFujitsuAcModeCool, kFujitsuAcFanQuiet);
    else
      desligarAC(ac2);
    Serial.println(ac2On ? F("AC1: LIGADO") : F("AC1: DESLIGADO"));
  }


  if (btnProj.previousDuration() < 3000) {
    if (btnProj.fell())
      ligarProjetor();
  } else if (!btnProj.read() && !projHeld) {
    congelarProjetor();
    projHeld = true;
  }

  if (btnProj.rose())
    projHeld = false;
}
