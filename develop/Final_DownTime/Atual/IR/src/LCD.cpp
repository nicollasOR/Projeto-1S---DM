// #include <Arduino.h>
// #include <IRremoteESP8266.h>
// #include <IRsend.h>
// #include <Bounce2.h>
// #include <LiquidCrystal_I2C.h>
// #include <stdio.h>
// #include <string.h>
// #include <ArduinoJson.h>
// #include <PubSubClient.h>
// #include <WiFi.h>
// #include "internet.h"

// WiFiClient espClient;
// PubSubClient client(espClient);


// const char *mqtt_server = "broker.hivemq.com"; // endereco do broker, pode ser um ip, nds
// const int mqtt_port = 1883;                    // porta de comunicacao sem seguranca
// const char *mqtt_id = "esp32-senai134-ju";     // id = nome do dispositivo
// // TODO const char *mqtt_id = "senaideck-1234";
// // const char *mqtt_topic_sub = "senaideck-1234/mesa05/"; //o esp esta ouvindo este topico,
// // const char *mqtt_topic_pub = "senai134/mesa05/projetoAProjPub"; //o esp publica mensagens

// const char *mqtt_topic_sub = "senai134/mesa05/projetoArProjSub"; // o esp esta ouvindo este topico,
// const char *mqtt_topic_pub = "senai134/mesa05/projetoArProjPub"; // o esp publica mensagens

// void mqttConnect(void);
// void tratarMensagemIR(JsonDocument &doc);
// // ==============================
// // CONFIGURAÇÕES DE HARDWARE
// // ==============================

// // Inicializa o LCD I2C para 16 colunas e 4 linhas
// LiquidCrystal_I2C lcd(0x27, 16, 4);

// // IR LED (mantido para futuras funções)
// const uint16_t kIrLed = 4;
// IRsend irsend(kIrLed);

// // Configuração dos botões (8 botões com pull-up interno)
// const int botoes[] = {12, 13, 14, 15, 5, 19, 23, 18};
// Bounce btn[8];

// //* ESTRUTURAS DE ESTADO

// struct variaveisAR
// {
//     bool ligar = 0;
//     int temperatura = 24;      // faixa: 18 a 30
//     int velocidade = 1;        // faixa: 1 a 3
//     int unidade = 1;           // valores: 1,2,3,4 e 0 para 'Todos'
//     bool controlarTemp = true; // true: controla temperatura; false: controla velocidade
// };

// bool estadoAnteriorAr = false;
// bool estadoAnteriorProj = false;

// struct variaveisProj
// {
//     bool ligar = 0;
//     int unidade = 1; // valores: 1,2 e 0 para 'Todos'
//     bool congelado = false;
// };

// variaveisAR air;
// variaveisProj proj;

// // Dispositivo selecionado para ajuste: 0 = AR, 1 = Projetor
// int selectedDevice = 0;
// variaveisAR ultimoEstadoAr;

// // Variaveis de travamento
// bool travado = false;
// bool aguardandoTrava = false;
// unsigned long tempoTrava = 0;
// bool atualizou = false;

// // 
// // FUNÇÃO DE ATUALIZAÇÃO DO LCD
// // 

// void atualizarLCD()
// {
//     // Linha 1 - Cabeçalho
//     char arHeader[9];
//     char pjHeader[9];
//     char header[17];

//     // AR Header (colunas 0-7): mostra "AR:" + unidade; a seta que indica seleção do dispositivo
//     char seta_ar = (selectedDevice == 0) ? '>' : ' ';
//     char unidadeAr[3];
//     if (air.unidade == 0)
//         strcpy(unidadeAr, "TD");
//     else
//         sprintf(unidadeAr, "%d", air.unidade);
//     sprintf(arHeader, "%cAR:%s", seta_ar, unidadeAr);
//     int len = strlen(arHeader);
//     for (int i = len; i < 8; i++)
//         arHeader[i] = ' ';
//     arHeader[8] = '\0';

//     // PJ Header (colunas 8-15): mostra "PJ:" + unidade; seta se selecionado
//     char seta_pj = (selectedDevice == 1) ? '>' : ' ';
//     char unidadePj[3];
//     if (proj.unidade == 0)
//         strcpy(unidadePj, "TD");
//     else
//         sprintf(unidadePj, "%d", proj.unidade);
//     sprintf(pjHeader, "%cPJ:%s", seta_pj, unidadePj);
//     len = strlen(pjHeader);
//     for (int i = len; i < 8; i++)
//         pjHeader[i] = ' ';
//     pjHeader[8] = '\0';

//     sprintf(header, "%-8s%-8s", arHeader, pjHeader);

//     // Linha 2: Temperatura (apenas AR)
//     // Se o AR estiver selecionado e o controle ativo for a temperatura, usa bolinha ('o');
//     // caso contrário, o primeiro caractere é espaço.
//     char rowTemp[17];
//     char bullet = (selectedDevice == 0 && air.controlarTemp) ? 'o' : ' ';
//     sprintf(rowTemp, "%cT:%02d", bullet, air.temperatura);
//     len = strlen(rowTemp);
//     for (int i = len; i < 16; i++)
//         rowTemp[i] = ' ';
//     rowTemp[16] = '\0';

//     // Linha 3: Velocidade (apenas AR)
//     // Se o AR estiver selecionado e o controle ativo for velocidade, usa bolinha ('o');
//     char rowVel[17];
//     bullet = (selectedDevice == 0 && !air.controlarTemp) ? 'o' : ' ';
//     sprintf(rowVel, "%cV:%d", bullet, air.velocidade);
//     len = strlen(rowVel);
//     for (int i = len; i < 16; i++)
//         rowVel[i] = ' ';
//     rowVel[16] = '\0';

//     // Linha 4: Status de congelamento do Projetor, exibido apenas na área da direita (colunas 8-15)
//     char rowFreeze[17];
//     // Preenche a metade esquerda com espaços
//     char freezeStatus[9];
//     if (proj.congelado)
//         strcpy(freezeStatus, "FRZ");
//     else
//         strcpy(freezeStatus, "----");
//     len = strlen(freezeStatus);
//     for (int i = len; i < 8; i++)
//         freezeStatus[i] = ' ';
//     freezeStatus[8] = '\0';
//     sprintf(rowFreeze, "%-8s%-8s", " ", freezeStatus);

//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print(header);
//     lcd.setCursor(0, 1);
//     lcd.print(rowTemp);
//     lcd.setCursor(0, 2);
//     lcd.print(rowVel);
//     lcd.setCursor(0, 3);
//     lcd.print(rowFreeze);
// }
