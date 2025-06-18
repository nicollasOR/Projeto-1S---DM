/*#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

// Definição do struct para ar-condicionado
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

bool modulo = false; // false = AC, true = projetor

void callback(char *topic, byte *payload, unsigned int length) // Função de callback para receber mensagens MQTT
{
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
void setup()
{
    Serial.begin(115200);
}
void loop()
{
}
*/