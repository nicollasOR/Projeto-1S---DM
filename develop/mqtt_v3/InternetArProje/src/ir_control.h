#ifndef IR_CONTROL_H
#define IR_CONTROL_H

#include <ArduinoJson.h>

void inicializarIR();
void tratarMensagemIR(JsonDocument &doc);

#endif