#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "main.cpp"

void callBack(char *topic, byte *payload, unsigned int length)
{
  Serial.printf("mensagem recebida em %s: ", topic);

  String msgDeserialize = "";

  for (unsigned int i = 0; i < length; i++)
  {
    msgDeserialize += (char)payload[i];
  }

  Serial.println("JSON recebe 👍");

  JsonDocument docDeserialize;

  DeserializationError erro = deserializeJson(docDeserialize, msgDeserialize);

  if (erro)
  {
    Serial.println("deu merda no JSON");
    Serial.println(erro.c_str());

    return;
  }

  // agora, a logica de controle

  for (JsonPair valorChave : docDeserialize.as<JsonObject>()) // pareia o Json com o objeto criado do deserializeDoc
  {

    const char *ac_id = valorChave.key().c_str(); // Pega a chave do par atual(ar-condicionado), que é o nome do ar-condicionado
    int status = valorChave.value()["on/off"];    // valorchave.value = ac_2 = on ou off

    Serial.printf("Controle do %s: %s\n", ac_id, status ? "Liga" : "Desliga"); // imprime na serial se está desligado ou ligado.
  }
  // JsonObject AC1 = doc.createNestedObject("ac1");
  // JsonObject AC2 = doc.createNestedObject("ac2");
  // JsonObject AC3 = doc.createNestedObject("ac3");
  // JsonObject AC3 = doc.createNestedObject("ac4"); só pra armazenar msm

  if (!docDeserialize["ac1"].isNull())
  {
    AC1["on/off"] = docDeserialize["on/off"];
    AC1["temperatura"] = docDeserialize["temperatura"];
    AC1["velocidade"] = docDeserialize["velocidade"];
  }

  if (!docDeserialize["ac2"].isNull())
  {
    AC2["on/off"] = docDeserialize["on/off"];
    AC2["temperatura"] = docDeserialize["temperatura"];
    AC2["velocidade"] = docDeserialize["velocidade"];
  }
  if (!docDeserialize["ac3"].isNull())
  {
    AC3["on/off"] = docDeserialize["on/off"];
    AC3["temperatura"] = docDeserialize["temperatura"];
    AC3["velocidade"] = docDeserialize["velocidade"];
  }

  if (!docDeserialize["ac4"].isNull())
  {
    AC4["on/off"] = docDeserialize["on/off"];
    AC4["temperatura"] = docDeserialize["temperatura"];
    AC4["velocidade"] = docDeserialize["velocidade"];
  }
}
