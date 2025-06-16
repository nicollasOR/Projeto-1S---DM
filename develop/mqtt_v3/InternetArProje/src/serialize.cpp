// codigo para juntar com o do giuliano depois
// codigo serialize json = enviar dados para o mqtt

void serialize()
{
  JsonDocument doc;

    // Subdocumento para ac1
  JsonObject AC1 = doc.createNestedObject("ac1");
  AC1["ac1"] = estadoAC1 ? "on" : "off";
  AC1["vel"] = air.velocidade;
  AC1["temp"] = air.temperatura;
  doc["unid"] = air.unidade;

  // Subdocumento para ac2
  JsonObject AC2 = doc.createNestedObject("ac2");
  AC2["ac2"] = estadoAC2 ? "on" : "off";
  AC2["vel"] = air.velocidade;
  AC2["temp"] = air.temperatura;
  doc["unid"] = air.unidade;

  // Subdocumento para projetor
  JsonObject Proj = doc.createNestedObject("proj");
  Proj["proj"] = estadoProj ? "on" : "off";
  Proj["freeze"] = proj.congelado ? "on" : "off";
  doc["pjunid"] = proj.unidade;
  

  String mensagem;
  serializeJson(doc, mensagem); // Envia para a serial

  client.publish(mqtt_topic_pub, mensagem.c_str());
  
  Serial.print("Enviando dados MQTT: ");
  Serial.println(mensagem);
  
}
