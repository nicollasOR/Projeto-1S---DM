#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "codgiuliano.cpp"
#include <Bounce2.h>
#include "thiago.cpp"



// Declarações globais
bool estadoProj1 = false; // Estado do projetor 1
bool estadoProj2 = false; // Estado do projetor 2
bool estadoAC4 = false;   // Estado do ar-condicionado 4
bool estadoAC3 = false;   // Estado do ar-condicionado 3
bool estadoAC2 = false;   // Estado do ar-condicionado 2
bool estadoAC1 = false;   // Estado do ar-condicionado 1

// Estrutura ou classe para o projetor
struct Projetor
{
    bool congelado = false; // Estado` de congelamento
};

// Instância global do projetor
Projetor proj;

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.printf("mensagem recebida em %s: ", topic);

    String mensagem = "";
    for (unsigned int i = 0; i < length; i++)
    {
        mensagem += (char)payload[i];
    }
    Serial.println(mensagem);

    JsonDocument doc;
    DeserializationError erro = deserializeJson(doc, mensagem);
    if (erro)
    {
        Serial.print("Erro ao decodificar JSON: ");
        Serial.println(erro.f_str());
        return;
    }

    String topico = String(topic);

    if (topico == "ac1")
    {
        if (!doc["ac1"].isNull())
            estadoAC1 = doc["ac1"] == "on";
        if (!doc["vel"].isNull())
            air.velocidade = doc["vel"];
        if (!doc["temp"].isNull())
            air.controlarTemp = doc["temp"];
    }
    else if (topico == "ac2")
    {
        if (!doc["ac2"].isNull())
            estadoAC2 = doc["ac2"] == "on";
        if (!doc["vel"].isNull())
            air.velocidade = doc["vel"];
        if (!doc["temp"].isNull())
            air.controlarTemp = doc["temp"];
    }
    else if (topico == "ac3")
    {
        if (!doc["ac3"].isNull())
            estadoAC3 = doc["ac3"] == "on";
        if (!doc["vel"].isNull())
            air.velocidade = doc["vel"];
        if (!doc["temp"].isNull())
            air.controlarTemp = doc["temp"];
    }
    else if (topico == "ac4")
    {
        if (!doc["ac4"].isNull())
            estadoAC4 = doc["ac4"] == "on";
        if (!doc["vel"].isNull())
            air.velocidade = doc["vel"];
        if (!doc["temp"].isNull())
            air.controlarTemp = doc["temp"];
    }
    else if (topico == "proj1")
    {
        if (!doc["proj1"].isNull())
            estadoProj1 = doc["proj1"] == "on";
        if (!doc["freeze"].isNull())
            proj.congelado = doc["freeze"] == "on";
    }
    else if (topico == "proj2")
    {
        if (!doc["proj2"].isNull())
            estadoProj2 = doc["proj2"] == "on";
        if (!doc["freeze2"].isNull())
            proj.congelado = doc["freeze2"] == "on";
    }
}
