
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

EthernetClient client;

IPAddress server(10,201,1,11);

unsigned long lastConnectionTime = 0;
unsigned long lastConnectionTimeBuzzer = 0;
boolean lastConnected = false;
String jsonRetorno = "";
const unsigned long postingInterval = 30*1000;
String cor = "blue";

const int buzzerPin =  9;
const int giroflexPin = 8;

const int COMPRAS = 0;
const int ESTOQUE = 1;
const int PDV = 2;
const int VPSA = 3;

String projetos[] = { "compras-java", "estoque-java-integration-test", "offlinemanager", "vpsa-java-integration-test" };
int bipes[] = { 1, 2, 3, 4 };
int indexProjetoAtual = -1;
int retryCount = 0;

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
    int posicaoSeparador = jsonRetorno.indexOf(":\"");

    cor = jsonRetorno.substring(jsonRetorno.indexOf("\"}", posicaoSeparador), posicaoSeparador + 2);
    
    if(cor != "blue")
    {
      Serial.println("Ta com erro");
      tocarAlarme();
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
    if (indexProjetoAtual == VPSA) {
      indexProjetoAtual = COMPRAS;
    } else {
      indexProjetoAtual++;
    }
    httpRequest();
  }
  
  lastConnected = client.connected();
}

void tocarAlarme() {
    digitalWrite(giroflexPin, HIGH);
  
    const int delayNota = 500;
    const int delayPausa = delayNota * 2;
    const int nota = 261;
    
    for (int intervalo = 0; intervalo < 3; intervalo++) {
      for (int vezes = 0; vezes < bipes[indexProjetoAtual]; vezes++) {
        tone(buzzerPin, nota, delayNota);
        delay(delayPausa);
      }
      delay(4000);
    }
    
    digitalWrite(giroflexPin, LOW);
}

void httpRequest() {
  if (client.connect(server, 9080)) {
    Serial.println("connecting...");

    client.println("GET /hudson/job/" + projetos[indexProjetoAtual] + "/api/json?tree=color HTTP/1.0");
    client.println();

    Serial.println();
    Serial.println("Projeto: " + projetos[indexProjetoAtual]);
    lastConnectionTime = millis();
  } 
  else {
    Serial.println("connection failed");
    Serial.println("disconnecting.");
    client.stop();
    
    if (retryCount < 3) {
      retryCount++;
      httpRequest(); 
    } else {
      retryCount = 0;  
    }
  }
}
