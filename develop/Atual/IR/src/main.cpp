
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
// #include "LCD.h"

//  Wi‑Fi e MQTT

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
  bool ligar = false;
  int temperatura = 24;
  int velocidade = 1;
  int unidade = 1;
  bool controlarTemp = true;
};

struct ProjState {
  bool ligar = 0;
  int unidade = 1;
  bool congelado = false;
};

Air acs[4];
Projetor projetor;
AirState air;
ProjState proj;
bool modulo = false;
bool atualizou = false;
int selectedDevice = 0;

// //! VariaveisJussa 
// bool reuniaoAtiva = false;        // Indica se uma reunião está ativa
// bool reuniaoAnterior = false;     // Armazena o estado anterior da reunião
// bool mensagemAtiva = false;       // Indica se uma mensagem foi recebida
// unsigned long tempoAnterior = 0;  // Guarda o tempo da última mudança no LED
// bool estadoLed = false;           // Estado atual do LED (ligado/desligado)

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

#define temperaturaPadrao 

void atualizarLCD() {
  // lcd.clear();
  // lcd.setCursor(0, 0);
  // lcd.print("AC1: ");
  // lcd.print(ac1On ? "LIGADO" : "DESLIGADO");

  // lcd.setCursor(0, 1);
  // lcd.print("Temp: ");
  // lcd.print(ac1.getTemp());

  // lcd.setCursor(0, 2);
  // lcd.print("AC2: ");
  // lcd.print(ac2On ? "LIGADO" : "DESLIGADO");

  // lcd.setCursor(0, 3);
  // lcd.print("Temp: ");
  // lcd.print(ac2.getTemp());

  // lcd.setCursor(0, 4);
  // lcd.print("Proj: ");
  // lcd.print(projOn ? "LIGADO" : "DESLIGADO");

  // lcd.setCursor(0, 5);
  // lcd.print("Congelado: ");
  // lcd.print(projetor.congelado ? "SIM" : "NAO");
  // Linha 1 - Cabeçalho
  char arHeader[9];
  char pjHeader[9];
  char header[17];

  // AR Header (colunas 0-7): mostra "AR:" + unidade; a seta que indica seleção do dispositivo
  char seta_ar = (selectedDevice == 0) ? '>' : ' ';
  char unidadeAr[3];
  if (air.unidade == 0)
  {
    strcpy(unidadeAr, "TD");
  }
  else
  {
    sprintf(unidadeAr, "%d", air.unidade);
  }
  sprintf(arHeader, "%cAR:%s", seta_ar, unidadeAr);
  int len = strlen(arHeader);
  for (int i = len; i < 8; i++)
  {
    arHeader[i] = ' ';
  }
  arHeader[8] = '\0';

  // PJ Header (colunas 8-15): mostra "PJ:" + unidade; seta se selecionado
  char seta_pj = (selectedDevice == 1) ? '>' : ' ';
  char unidadePj[3];
  if (proj.unidade == 0)
  {
    strcpy(unidadePj, "TD");
  }
  else
  {
    sprintf(unidadePj, "%d", proj.unidade);
  }
  sprintf(pjHeader, "%cPJ:%s", seta_pj, unidadePj);
  len = strlen(pjHeader);
  for (int i = len; i < 8; i++)
  {
    pjHeader[i] = ' ';
  }
  pjHeader[8] = '\0';

  sprintf(header, "%-8s%-8s", arHeader, pjHeader);

  // Linha 2: Temperatura (apenas AR)
  // Se o AR estiver selecionado e o controle ativo for a temperatura, usa bolinha ('o');
  // caso contrário, o primeiro caractere é espaço.
  char rowTemp[17];
  char bullet = (selectedDevice == 0 && air.controlarTemp) ? 'o' : ' ';
  sprintf(rowTemp, "%cT:%02d", bullet, air.temperatura);
  len = strlen(rowTemp);
  for (int i = len; i < 16; i++)
  {
    rowTemp[i] = ' ';
  }
  rowTemp[16] = '\0';

  // Linha 3: Velocidade (apenas AR)
  // Se o AR estiver selecionado e o controle ativo for velocidade, usa bolinha ('o');
  char rowVel[17];
  bullet = (selectedDevice == 0 && !air.controlarTemp) ? 'o' : ' ';
  sprintf(rowVel, "%cV:%d", bullet, air.velocidade);
  len = strlen(rowVel);
  for (int i = len; i < 16; i++)
  {
    rowVel[i] = ' ';
  }
  rowVel[16] = '\0';

  // Linha 4: Status de congelamento do Projetor, exibido apenas na área da direita (colunas 8-15)
  char rowFreeze[17];
  // Preenche a metade esquerda com espaços
  char freezeStatus[9];
  if (proj.congelado)
  {
    strcpy(freezeStatus, "FRZ");
  }
  else
  {
    strcpy(freezeStatus, "----");
  }
  len = strlen(freezeStatus);
  for (int i = len; i < 8; i++)
  {
    freezeStatus[i] = ' ';
  }
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


// * Funções 
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

void mqttCallback(char *topic, byte *payload, unsigned int length) 
{
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);

  String mensagem;
  for (unsigned int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }
 
  Serial.print("Conteúdo: ");
  Serial.println(mensagem);

  // Interpretar a mensagem JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, mensagem);
  

  
  if (error) {
    Serial.print(F("Falha ao interpretar JSON: "));
    Serial.println(error.f_str());
    return;
  }

  JsonObject JsonAR = doc["ac"];
  JsonObject JsonProj = doc["proj"];
  if(!JsonAR.isNull())
  {
    if(!JsonAR["liga"].isNull())
    {
  configurarAC(ac1, 24, kFujitsuAcModeCool, kFujitsuAcFanQuiet); 
  ac1On = JsonAR["liga"];
 Serial.println(ac1On ? "AC1: LIGADO" : "AC1: DESLIGADO");
  }

   if(!JsonAR["temp"].isNull() && !JsonAR["vel"].isNull())
   {
    if(ac1On)
    {
      
     air.velocidade = JsonAR["vel"].as<int>();
     air.temperatura = JsonAR["temp"].as<int>();
    configurarAC(ac1, air.temperatura, kFujitsuAcModeCool, air.velocidade);
    Serial.print("Temperatura ajustada para: ");
    Serial.println(air.temperatura);

      Serial.print("velocidade ajustada para: ");
    Serial.println(air.velocidade);
    }
   }

   }
  if(!JsonProj.isNull())
  {
    if(!JsonProj["ligar"].isNull())
    {
     projOn = JsonProj["ligar"];
      Serial.println(projOn ? "proj: LIGADO" : "proj: DESLIGADO");
    }
   if(!JsonProj["congelar"].isNull())
   {
    projetor.congelado = JsonProj["congelar"];
      Serial.println(projetor.ligado ? "proj: congelado" : "proj: N congelado");
   }

  } 
   atualizarLCD();
  atualizou = true;
  }

 

  // //! Jussa
  //  if (!doc["reuniao"].isNull())
  // {
  //   reuniaoAtiva = doc["reuniao"]; // Atualiza estado de reunião se presente na mensagem
  // }

  // if (!doc["mensagem"].isNull())
  // {
  //   mensagem = doc["mensagem"].as<String>(); // Lê o conteúdo da mensagem
  //   mensagemAtiva = true; // Ativa o estado de mensagem
  // }
  


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
  btnAC2.attach(BTN_AC2, INPUT_PULLUP);
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
    Serial.println(ac2On ? F("AC2: LIGADO") : F("AC2: DESLIGADO"));
  }
/*
if (btnProj.fell()) {
  projPressedAt = millis();
}

if (btnProj.read() == LOW && !projHeld && millis() - projPressedAt > 3000) {
  congelarProjetor();
  projHeld = true;
}

if (btnProj.rose()) {
  projHeld = false;
  if (millis() - projPressedAt <= 3000) {
    ligarProjetor();
  }
}


*/
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