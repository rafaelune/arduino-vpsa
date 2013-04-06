
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

EthernetClient client;

IPAddress server(10,201,1,11);

unsigned long lastConnectionTime = 0;
unsigned long lastConnectionTimeBuzzer = 0;
boolean lastConnected = false;
String jsonRetorno = "";
const unsigned long postingInterval = 10*1000;
String cor = "blue";

boolean tocarAlarme = false;

const int buzzerPin =  9;
const int giroflexPin = 8;

void setup() {
  pinMode(buzzerPin, OUTPUT);  
  pinMode(giroflexPin, OUTPUT); 
  
  Serial.begin(115200);
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

    cor = jsonRetorno.substring(jsonRetorno.indexOf("\"}", posicaoSeparador), posicaoSeparador + 2);
    
    if(cor != "blue")
    {
      Serial.println("Ta com erro");
      tocarAlarme = true;
    }
    else
    {
      Serial.println("Ta OK");
      tocarAlarme = false;
    }
    
    Serial.println(cor);
    Serial.println("disconnecting.");
    client.stop();
    jsonRetorno = "";
  }

  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    httpRequest();
  }
  
  if (tocarAlarme) {
    digitalWrite(giroflexPin, HIGH);
    const int delayNota = 1000;
    const int delayPausa = delayNota * 2;
    const int nota = 261;
    tone(buzzerPin, nota, delayNota);
    delay(delayPausa);
     
    tone(buzzerPin, nota, delayNota);
    delay(delayPausa);
    
    tone(buzzerPin, nota, delayNota);
    delay(delayPausa);
    
    tone(buzzerPin, nota, delayNota);
    
    tocarAlarme = false;
    digitalWrite(giroflexPin, LOW);
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
