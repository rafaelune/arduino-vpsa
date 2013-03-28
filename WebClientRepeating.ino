
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

EthernetClient client;

IPAddress server(10,201,1,11);

unsigned long lastConnectionTime = 0;
boolean lastConnected = false;
String jsonRetorno = "";
const unsigned long postingInterval = 10*1000;

void setup() {
  Serial.begin(9600);
  delay(1000);
  Ethernet.begin(mac);
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  if (client.available()) {
    char c = client.read();
    jsonRetorno += c;
  }

  if (!client.connected() && lastConnected) {
    Serial.println();
    
    int posicaoSeparador = jsonRetorno.indexOf(":\"");

    String cor = jsonRetorno.substring(jsonRetorno.indexOf("\"}", posicaoSeparador), posicaoSeparador + 2);
    
    if(cor != "blue")
    {
      Serial.println("Ta com erro");    
    }
    else
    {
      Serial.println("Ta OK");
    }
    
    Serial.println(cor);
    Serial.println("disconnecting.");
    client.stop();
    jsonRetorno = "";
  }

  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    httpRequest();
  }

  lastConnected = client.connected();
}

void httpRequest() {

  if (client.connect(server, 9080)) {
    Serial.println("connecting...");

    client.println("GET /hudson/job/commons/api/json?tree=color HTTP/1.0");
    client.println();

    lastConnectionTime = millis();
  } 
  else {
    Serial.println("connection failed");
    Serial.println("disconnecting.");
    client.stop();
  }
}
