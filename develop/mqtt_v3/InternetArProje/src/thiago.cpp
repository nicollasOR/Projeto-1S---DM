// #include <Arduino.h>
// #include <IRremoteESP8266.h>
// #include <ir_Fujitsu.h>

// constexpr uint16_t kIrLed   = 4;   // LED IR (via driver)
// constexpr uint8_t  kBtnBoot = 0;   // Botão BOOT na maioria dos ESP32-Dev

// IRFujitsuAC ac(kIrLed);            // Objecto “smart” para Fujitsu

// bool isOn         = false;         // Estado lógico do AC
// uint32_t debounce = 0;             // ms último accionamento

// //------------------------------------------------------------------
// // Constrói o frame “ligado” (Cool 24 °C, Fan Quiet)
// void buildOnFrame() {
//   ac.begin();                        // Reinicia estrutura interna
//   ac.setModel(ARRAH2E);              // Modelo = 1 (ARRAH2E)
//   ac.setId(0);                       // Id do comando (0 no teu dump)
//   ac.setMode(kFujitsuAcModeCool);    // Cool
//   ac.setTemp(24);                    // 24 °C
//   ac.setFanSpeed(kFujitsuAcFanQuiet);     // Ventoinha Quiet
//   ac.setSwing(kFujitsuAcSwingOff);
//   ac.setPower(true);                 // Liga
// }

// // Apenas troca o bit de Power para OFF mantendo o resto do estado
// void buildOffFrame() {
//   ac.setPower(false);
// }

// void setup() {
//   Serial.begin(115200);
//   pinMode(kBtnBoot, INPUT_PULLUP);

//   buildOffFrame();   // força quadro “OFF” ao arrancar
//   ac.send();
// }

// void loop() {
//   // Leitura simples do BOOT com debounce ~40 ms
//   if (!digitalRead(kBtnBoot) && millis() - debounce > 40) {
//     while (!digitalRead(kBtnBoot)) delay(1);  // espera libertar
//     debounce = millis();

//     isOn = !isOn;
//     if (isOn) buildOnFrame();
//     else      buildOffFrame();

//     ac.send();   // biblioteca gera e transmite os 128 bits
//     Serial.println(isOn ? F("AC ON") : F("AC OFF"));
//   }
// }

//! EPSON

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

constexpr uint8_t IR_LED_PIN = 4; // pino do LED infravermelho
IRsend irsend(IR_LED_PIN);        // instância global

// ───────────────────────────────── RAW ARRAYS ─────────────────────────────────
// Cole cada bloco "uint16_t rawData[135] = { … }" aqui, mudando o nome p/ RAW_XXX
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

const uint16_t RAW_SOURCE[] PROGMEM = {
    9044, 4384, 664, 1584, 638, 488, 662, 464, 662, 464, 666, 460, 638, 488, 652, 472, 662, 1586, 666, 1582, 664, 1584, 664, 462, 664, 462, 662, 464, 664, 462, 638, 488, 662, 464, 652, 472, 658, 468, 660, 466, 658, 468, 660, 1588, 658, 1590, 658, 1590, 658, 1592, 630, 1620, 628, 1620, 652, 1596, 652, 1598, 650, 504, 620, 506, 618, 508, 618, 508, 618, 43794, 9038, 4410, 616, 1628, 622, 1628, 622, 532, 592, 534, 594, 534, 592, 534, 592, 534, 594, 1628, 618, 1630, 620, 532, 592, 1630, 618, 534, 592, 1630, 618, 534, 592, 1630, 618, 534, 590, 534, 590, 510, 618, 1630, 618, 1630, 618, 508, 620, 506, 618, 508, 618, 1630, 622, 1628, 620, 1630, 620, 506, 620, 506, 620, 1628, 622, 1628, 622, 1628, 620, 504, 620};

const uint16_t RAW_HDMI[] PROGMEM = {
    /*  <<< cole aqui o rawData completo do botão HDMI >>>  */
};

const uint16_t RAW_COMPUTER[] PROGMEM = {
    /*  <<< cole aqui o rawData completo do botão COMPUTER >>>  */
};

const uint16_t RAW_USB[] PROGMEM = {
    /*  <<< cole aqui o rawData completo do botão USB >>>  */
};

const uint16_t RAW_LAN[] PROGMEM = {
    /*  <<< cole aqui o rawData completo do botão LAN >>>  */
};

const uint16_t RAW_MENU[] PROGMEM = {
    9074, 4384, 644, 1574, 646, 480, 668, 458, 646, 478, 648, 476, 646, 480,
    670, 456, 670, 1578, 672, 1578, 670, 1578, 674, 452, 670, 454, 670, 456,
    670, 456, 670, 456, 668, 458, 670, 456, 670, 456, 672, 456, 668, 458,
    668, 1578, 670, 1580, 668, 1578, 672, 1578, 668, 1580, 670, 1580, 668,
    1580, 666, 1582, 666, 460, 668, 458, 666, 458, 670, 458, 666, 43782,
    9086, 4362, 664, 1584, 666, 1582, 668, 460, 664, 462, 662, 464, 662, 462,
    664, 462, 662, 1584, 664, 1586, 664, 462, 662, 1586, 662, 466, 660,
    1588, 662, 494, 632, 1588, 662, 494, 632, 494, 630, 1588, 660, 496, 632,
    1588, 660, 1590, 658, 496, 632, 494, 630, 1590, 658, 1590, 658, 496, 630,
    1590, 658, 496, 630, 498, 628, 1592, 656, 1594, 656, 498, 626};

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
// … repita para ↑ ↓ ← → ENTER VOL+ VOL– MUTE FREEZE AUTO ESC …
// ───────────────────────────────────────────────────────────────

// Estrutura que liga hash → rawArray → tamanho
struct BtnMap
{
  uint32_t hash;
  const uint16_t *raw;
  uint16_t len;
};

// Tabela de 18 entradas (complete à medida que colar os RAWs)
const BtnMap btnMap[] PROGMEM = {
    {0xDB996389, RAW_POWER, sizeof(RAW_POWER) / 2},
    {0xF51CD7AA, RAW_SOURCE, sizeof(RAW_SOURCE) / 2},
    {0x2990B795, RAW_HDMI, sizeof(RAW_HDMI) / 2},
    {0xE8C9A33E, RAW_COMPUTER, sizeof(RAW_COMPUTER) / 2},
    {0xC6172201, RAW_USB, sizeof(RAW_USB) / 2},
    {0xB5FEFF3D, RAW_LAN, sizeof(RAW_LAN) / 2},
    {0x74DC1858, RAW_MENU, sizeof(RAW_MENU) / 2},
    {0x3D54F20F, RAW_FREEZE, sizeof(RAW_FREEZE) / 2},

};
constexpr uint8_t BTN_CNT = sizeof(btnMap) / sizeof(btnMap[0]);

// ───────────────────────── UTIL ─────────────────────────
void sendByIndex(uint8_t idx)
{
  if (idx >= BTN_CNT)
    return;
  BtnMap entry;
  memcpy_P(&entry, &btnMap[idx], sizeof(BtnMap));

  irsend.sendRaw(entry.raw, entry.len, 38); // 38 kHz
  Serial.printf("» Enviado botão %u  (hash 0x%08X, %u pulsos)\n",
                idx + 1, entry.hash, entry.len);
}

// ───────────────────────── SETUP ─────────────────────────
void setup()
{
  Serial.begin(115200);
  while (!Serial)
    delay(10);
  irsend.begin();

  Serial.println(F("\n=== Epson IR Transmitter ==="));
  for (uint8_t i = 0; i < BTN_CNT; i++)
  {
    Serial.printf("  [%2u]  enviar botão #%u\n", i + 1, i + 1);
  }
  Serial.println(F("\nDigite o número e <Enter>."));
}

// ───────────────────────── LOOP ──────────────────────────
void loop()
{
  if (Serial.available())
  {
    int c = Serial.parseInt(); // lê número
    if (c >= 1 && c <= BTN_CNT)
    {
      sendByIndex(c - 1);
    }
    else
    {
      Serial.println(F("Valor inválido."));
    }
  }
}

//! COLETOR DE RAWS

// /*  Epson - coletor de RAWs  ────────────────────────────────

// #include <Arduino.h>
// #include <assert.h>
// #include <IRrecv.h>
// #include <IRremoteESP8266.h>
// #include <IRac.h>
// #include <IRtext.h>
// #include <IRutils.h>

// #ifdef ARDUINO_ESP32C3_DEV
// const uint16_t kRecvPin = 10;  // 14 on a ESP32-C3 causes a boot loop.
// #else  // ARDUINO_ESP32C3_DEV
// const uint16_t kRecvPin = 15;
// #endif  // ARDUINO_ESP32C3_DEV

// const uint32_t kBaudRate = 115200;

// // As this program is a special purpose capture/decoder, let us use a larger
// // than normal buffer so we can handle Air Conditioner remote codes.
// const uint16_t kCaptureBufferSize = 1024;

// #if DECODE_AC
// // Some A/C units have gaps in their protocols of ~40ms. e.g. Kelvinator
// // A value this large may swallow repeats of some protocols
// const uint8_t kTimeout = 50;
// #else   // DECODE_AC
// // Suits most messages, while not swallowing many repeats.
// const uint8_t kTimeout = 15;
// #endif  // DECODE_AC

// const uint16_t kMinUnknownSize = 12;

// const uint8_t kTolerancePercentage = kTolerance;  // kTolerance is normally 25%

// // Legacy (No longer supported!)
// //
// // Change to `true` if you miss/need the old "Raw Timing[]" display.
// #define LEGACY_TIMING_INFO false
// // ==================== end of TUNEABLE PARAMETERS ====================

// // Use turn on the save buffer feature for more complete capture coverage.
// IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
// decode_results results;  // Somewhere to store the results

// // This section of code runs only once at start-up.
// void setup() {
// #if defined(ESP8266)
//   Serial.begin(kBaudRate, SERIAL_8N1, SERIAL_TX_ONLY);
// #else  // ESP8266
//   Serial.begin(kBaudRate, SERIAL_8N1);
// #endif  // ESP8266
//   while (!Serial)  // Wait for the serial connection to be establised.
//     delay(50);
//   // Perform a low level sanity checks that the compiler performs bit field
//   // packing as we expect and Endianness is as we expect.
//   assert(irutils::lowLevelSanityCheck() == 0);

//   Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);
// #if DECODE_HASH
//   // Ignore messages with less than minimum on or off pulses.
//   irrecv.setUnknownThreshold(kMinUnknownSize);
// #endif  // DECODE_HASH
//   irrecv.setTolerance(kTolerancePercentage);  // Override the default tolerance.
//   irrecv.enableIRIn();  // Start the receiver
// }

// // The repeating section of the code
// void loop() {
//   // Check if the IR code has been received.
//   if (irrecv.decode(&results)) {
//     // Display a crude timestamp.
//     uint32_t now = millis();
//     Serial.printf(D_STR_TIMESTAMP " : %06u.%03u\n", now / 1000, now % 1000);
//     // Check if we got an IR message that was to big for our capture buffer.
//     if (results.overflow)
//       Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);
//     // Display the library version the message was captured with.
//     Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_STR "\n");
//     // Display the tolerance percentage if it has been change from the default.
//     if (kTolerancePercentage != kTolerance)
//       Serial.printf(D_STR_TOLERANCE " : %d%%\n", kTolerancePercentage);
//     // Display the basic output of what we found.
//     Serial.print(resultToHumanReadableBasic(&results));
//     // Display any extra A/C info if we have it.
//     String description = IRAcUtils::resultAcToString(&results);
//     if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
//     yield();  // Feed the WDT as the text output can take a while to print.
// #if LEGACY_TIMING_INFO
//     // Output legacy RAW timing info of the result.
//     Serial.println(resultToTimingInfo(&results));
//     yield();  // Feed the WDT (again)
// #endif  // LEGACY_TIMING_INFO
//     // Output the results as source code
//     Serial.println(resultToSourceCode(&results));
//     Serial.println();    // Blank line between entries
//     yield();             // Feed the WDT (again)
//   }
// }

//! LEITOR DE HASH

// /**********************************************************************
//  * Epson – IR Button Mapper  (ignora o segundo quadro automático)
//  *  · Receptor  : GPIO 15
//  *  · Baudrate  : 115 200
//  *********************************************************************/







// #include <Arduino.h>
// #include <IRremoteESP8266.h>
// #include <IRrecv.h>
// #include <IRutils.h>

// const uint16_t PIN_RECV       = 15;
// const uint16_t CAPTURE_BUFFER = 1024;
// const uint8_t  TIMEOUT_RX_MS  = 15;          // 15 ms ≃ NEC
// IRrecv         ir(PIN_RECV, CAPTURE_BUFFER, TIMEOUT_RX_MS, true);
// decode_results res;

// // ---------- FNV-1a hash de um raw ----------
// uint32_t hashFNV1a(const volatile uint16_t *d, uint16_t n)
// {
//   const uint32_t FNV_PRIME = 16777619UL;
//   uint32_t h = 2166136261UL;
//   while (n--) { h ^= *d++; h *= FNV_PRIME; }
//   return h;
// }

// // ---------- Lista de botões na ordem desejada ----------
// const char *const botoes[] PROGMEM = {
//   "POWER", "SOURCE SEARCH", "HDMI", "COMPUTER", "USB", "LAN",
//   "MENU", "↑", "↓", "←", "→", "ENTER",
//   "VOL +", "VOL –", "MUTE", "FREEZE", "AUTO", "ESC"
// };
// const uint8_t TOTAL = sizeof(botoes) / sizeof(botoes[0]);

// // tempo (ms) em que ficamos “surdos” depois de capturar o 1.º quadro
// const uint16_t DEAF_MS = 350;

// // --------------------------------------------------------------------
// void setup() {
//   Serial.begin(115200);
//   while (!Serial) delay(10);

//   ir.setUnknownThreshold(4);     // descarta ruídos muito curtos
//   ir.enableIRIn();               // receptor ON

//   Serial.println(F("\n=== Epson IR Button Mapper ==="));
// }

// // --------------------------------------------------------------------
// uint32_t capturar(uint8_t idx)
// {
//   Serial.printf("\n[%u/%u] Pressione **%s**...\n",
//                 idx + 1, TOTAL, botoes[idx]);

//   for (;;) {
//     if (!ir.decode(&res)) { delay(4); continue; }

//     if (res.repeat) { ir.resume(); continue; }        // descarta “repeat”

//     uint32_t h = hashFNV1a(res.rawbuf, res.rawlen);
//     Serial.printf("  ▸ hash = 0x%08lX  len=%u  ✅\n",
//                   (unsigned long)h, res.rawlen);
//     ir.resume();                                     // limpa o buffer

//     /* ignora qualquer coisa nos próximos DEAF_MS ms (segundo quadro) */
//     uint32_t t0 = millis();
//     while (millis() - t0 < DEAF_MS) {
//       if (ir.decode(&res)) ir.resume();
//       delay(2);
//     }
//     return h;
//   }
// }

// // --------------------------------------------------------------------
// void loop() {
//   for (uint8_t i = 0; i < TOTAL; ++i) (void)capturar(i);

//   Serial.println(F("\nMapeamento concluído — reinicie a placa para repetir."));
//   while (true) delay(1000);
// }

//! HASHS

// [1/18] Pressione **POWER**...
//   ▸ hash = 0xDB996389  len=68  ✅

// [2/18] Pressione **SOURCE SEARCH**...
//   ▸ hash = 0xF51CD7AA  len=68  ✅

// [3/18] Pressione **HDMI**...
//   ▸ hash = 0x2990B795  len=68  ✅

// [4/18] Pressione **COMPUTER**...
//   ▸ hash = 0xE8C9A33E  len=68  ✅

// [5/18] Pressione **USB**...
//   ▸ hash = 0xC6172201  len=68  ✅

// [6/18] Pressione **LAN**...
//   ▸ hash = 0xB5FEFF3D  len=68  ✅

// [7/18] Pressione **MENU**...
//   ▸ hash = 0x74DC1858  len=68  ✅

// [8/18] Pressione **↑**...
//   ▸ hash = 0xA6FBAE3B  len=68  ✅

// [9/18] Pressione **↓**...
//   ▸ hash = 0x3CF7FD37  len=68  ✅

// [10/18] Pressione **←**...
//   ▸ hash = 0xA876069F  len=68  ✅

// [11/18] Pressione **→**...
//   ▸ hash = 0x26EC9AA9  len=68  ✅

// [12/18] Pressione **ENTER**...
//   ▸ hash = 0x3F9F4649  len=68  ✅

// [13/18] Pressione **VOL +**...
//   ▸ hash = 0x67C6393A  len=68  ✅

// [14/18] Pressione **VOL –**...
//   ▸ hash = 0x2EEFE8E0  len=68  ✅

// [15/18] Pressione **MUTE**...
//   ▸ hash = 0xA4F6F079  len=68  ✅

// [16/18] Pressione **FREEZE**...
//   ▸ hash = 0x3D54F20F  len=68  ✅

// [17/18] Pressione **AUTO**...
//   ▸ hash = 0x2CAA8D78  len=68  ✅

// [18/18] Pressione **ESC**...
//   ▸ hash = 0x1AC0CEDC  len=68  ✅