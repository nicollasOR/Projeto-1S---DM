#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Fujitsu.h>
#include <Bounce2.h>

// === ALIAS MAIS CURTOS PARA CONSTANTES DO FUJITSU ===
#define MODE_AUTO kFujitsuAcModeAuto
#define MODE_COOL kFujitsuAcModeCool
#define MODE_FAN kFujitsuAcModeFan

#define FAN_AUTO kFujitsuAcFanAuto
#define FAN_QUIET kFujitsuAcFanQuiet
#define FAN_LOW kFujitsuAcFanLow
#define FAN_MED kFujitsuAcFanMed
#define FAN_HIGH kFujitsuAcFanHigh

#define SWING_OFF kFujitsuAcSwingOff
#define SWING_VERT kFujitsuAcSwingV
#define SWING_HORIZ kFujitsuAcSwingH
#define SWING_BOTH kFujitsuAcSwingBoth

// === DEFINIÇÕES DE PINOS ===
constexpr uint8_t IR_LED_AC1 = 4;
constexpr uint8_t IR_LED_AC2 = 5;
constexpr uint8_t IR_LED_PROJ = 18;

constexpr uint8_t BTN_AC1 = 23;
constexpr uint8_t BTN_PROJ = 22;

bool ac1On = false;
bool projOn = false;
unsigned long projPressedAt = 0;
bool projHeld = false;

// === INSTANCIAS ===
IRFujitsuAC ac1(IR_LED_AC1);
IRFujitsuAC ac2(IR_LED_AC2);
IRsend irsendProj(IR_LED_PROJ);

Bounce btnAC1 = Bounce();
Bounce btnProj = Bounce();

// === RAW POWER E FREEZE DO PROJETOR ===
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
    662, 1586, 660, 494, 632, 1588, 660, 1588, 660, 494, 632};

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
    1592, 656, 1590, 658, 498, 628};

// === FUNÇÕES DO AR CONDICIONADO ===
void iniciarAC(IRFujitsuAC &ac)
{
  ac.begin();
  ac.setModel(ARRAH2E);
  ac.setId(0);
}

void configurarAC(IRFujitsuAC &ac, uint8_t temp, uint8_t modo, uint8_t fan)
{
  ac.setTemp(temp);
  ac.setMode(modo);
  ac.setFanSpeed(fan);
  ac.setSwing(SWING_OFF);
  ac.setPower(true);
  ac.send();
}

void desligarAC(IRFujitsuAC &ac)
{
  ac.setPower(false);
  ac.send();
}

// === FUNÇÕES DO PROJETOR ===
void ligarProjetor()
{
  irsendProj.sendRaw(RAW_POWER, sizeof(RAW_POWER) / 2, 38);
  Serial.println(F("Projetor: POWER enviado"));
}

void congelarProjetor()
{
  irsendProj.sendRaw(RAW_FREEZE, sizeof(RAW_FREEZE) / 2, 38);
  Serial.println(F("Projetor: FREEZE enviado"));
}

void setup()
{
  Serial.begin(115200);

  irsendProj.begin();
  iniciarAC(ac1);
  iniciarAC(ac2);

  btnAC1.attach(BTN_AC1, INPUT_PULLUP);
  btnAC1.interval(25);
  btnProj.attach(BTN_PROJ, INPUT_PULLUP);
  btnProj.interval(25);

  Serial.println(F("Sistema IR inicializado"));
}

void loop()
{
  btnAC1.update();
  btnProj.update();

  // Controle AC1 com botão
  if (btnAC1.fell())
  {
    ac1On = !ac1On;
    if (ac1On)
      configurarAC(ac1, 24, MODE_COOL, FAN_QUIET);
    else
      desligarAC(ac1);
    Serial.println(ac1On ? F("AC1: LIGADO") : F("AC1: DESLIGADO"));
  }

  // Controle Projetor

  if (btnProj.previousDuration() < 3000)
  {
    if (btnProj.fell())
      ligarProjetor(); // comando de ligar funciona como toggle
  }
  else if (!btnProj.read() && !projHeld)
  {
    congelarProjetor();
    projHeld = true;
  }

  if (btnProj.rose())
    projHeld = false;
}