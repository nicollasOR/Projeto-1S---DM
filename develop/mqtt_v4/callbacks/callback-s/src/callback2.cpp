/*#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "main.cpp"
String acName = "ac1";

JsonObject AC1 = doc[]
*/
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>
// #include "main.cpp"

void callBack(char *topic, byte *payload, unsigned int length)
{
    Serial.printf("mensagem recebida em %s: ", topic);

    String msgDeserialize = "";
    for (unsigned int i = 0; i < length; i++)
    {
        msgDeserialize += (char)payload[i];
    }

    Serial.println(msgDeserialize);

    // Declarar doc com tamanho adequado
    DynamicJsonDocument docDeserialize(1024);

    DeserializationError erro = deserializeJson(docDeserialize, msgDeserialize);
    if (erro)
    {
        Serial.println("deu merda no JSON");
        Serial.println(erro.c_str());
        return;
    }

    // Itera pelos pares (exemplo: ac1, ac2, ac3...)
    for (JsonPair valorChave : docDeserialize.as<JsonObject>())
    {
        const char *ac_id = valorChave.key().c_str(); // ex: "ac1"
        JsonObject acObj = valorChave.value().as<JsonObject>();

        // Pega os valores do subobjeto
        bool status = acObj["on/off"];
        int temperatura = acObj["temperatura"];
        int velocidade = acObj["velocidade"];

        Serial.printf("Controle do %s: %s, Temp: %d, Vel: %d\n", ac_id, status ? "Liga" : "Desliga", temperatura, velocidade);

        // Se quiser manipular/alterar os dados, faça aqui
        // Por exemplo, inverter status:
        // acObj["on/off"] = !status;
    }

    // Se quiser, por exemplo, atualizar algum objeto separadamente
    // Exemplo: atualizar "ac1"
    if (!docDeserialize["ac1"].isNull())
    {
        JsonObject AC1 = docDeserialize["ac1"].to<JsonObject>(); // cria ou acessa
        AC1["on/off"] = true;       // liga
        AC1["temperatura"] = 24;
        AC1["velocidade"] = 3;
    }

    // Da mesma forma para ac2, ac3, ac4
    if (!docDeserialize["ac2"].isNull())
    {
        JsonObject AC2 = docDeserialize["ac2"].to<JsonObject>();
        AC2["on/off"] = false;
        AC2["temperatura"] = 22;
        AC2["velocidade"] = 1;
    }
    if (!docDeserialize["ac3"].isNull())
    {
        JsonObject AC3 = docDeserialize["ac3"].to<JsonObject>();
        AC3["on/off"] = true;
        AC3["temperatura"] = 23;
        AC3["velocidade"] = 2;
    }
    if (!docDeserialize["ac4"].isNull())
    {
        JsonObject AC4 = docDeserialize["ac4"].to<JsonObject>();
        AC4["on/off"] = false;
        AC4["temperatura"] = 21;
        AC4["velocidade"] = 1;
    }

    // Se quiser enviar o JSON atualizado de volta pelo MQTT,
    // basta serializar e publicar aqui.
    // Exemplo:
    /*
    String output;
    serializeJson(docDeserialize, output);
    client.publish("topico/atualizado", output.c_str());
    */
}
