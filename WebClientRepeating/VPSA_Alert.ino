#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

EthernetClient client;
EthernetServer webServer(80);

IPAddress hudsonIp(10,201,1,11);
IPAddress shieldIp(192,168,100,50);

unsigned long lastConnectionTime = 0;
unsigned long lastConnectionTimeBuzzer = 0;
boolean lastConnected = false;
String jsonRetorno = "";
// Intervalo de consulta
const unsigned long postingInterval = 225L*1000L;
String cor = "blue";

const String STATUS_TESTES_INTEGRACAO_QUEBRADOS = "red";
const String STATUS_TESTES_UNITARIOS_QUEBRADOS = "yellow";
const int BUZZER_PIN =  9;
const int GIROFLEX_PIN = 8;
const int COMPRAS = 0;
const int ESTOQUE = 1;
const int PDV = 2;
const int VPSA = 3;

String projetos[] = { 
  "compras-java-integration-test", "estoque-java-integration-test", "offlinemanager", "vpsa-java-integration-test" };
int bipes[] = { 
  1, 2, 3, 4 };
int indexProjetoAtual = -1;
int retryCount = 0;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);  
  pinMode(GIROFLEX_PIN, OUTPUT); 

  //Serial.begin(115200);
  delay(1000);
  Ethernet.begin(mac, shieldIp);
  webServer.begin();
  //Serial.print("My IP address: ");
  //Serial.println(Ethernet.localIP());
  tone(BUZZER_PIN, 440, 500);
}

void loop() {
  checkProjects();

  listenIncomingClients();
}

void checkProjects() {
  if (client.available()) {
    char c = client.read();
    jsonRetorno += c;
  }

  if (!client.connected() && lastConnected) {
    int posicaoSeparador = jsonRetorno.indexOf(":\"");

    cor = jsonRetorno.substring(jsonRetorno.indexOf("\"}", posicaoSeparador), posicaoSeparador + 2);

    if(cor == STATUS_TESTES_INTEGRACAO_QUEBRADOS || cor == STATUS_TESTES_UNITARIOS_QUEBRADOS)
    {
      tocarAlarme();
    }

    //Serial.println(cor);
    //Serial.println("disconnecting.");
    client.stop();
    jsonRetorno = "";
  }

  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    if (indexProjetoAtual == VPSA) {
      indexProjetoAtual = COMPRAS;
    } 
    else {
      indexProjetoAtual++;
    }
    httpRequest();
  }

  lastConnected = client.connected();
}

void listenIncomingClients() {
  // listen for incoming clients
  EthernetClient client = webServer.available();
  if (client) {
    //Serial.println("new client");
    
    while(client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n') {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head><title>Berduino: VPSA Alert</title></head>");
          client.println("<body>");
  
          client.println("<h1>Berduino: VPSA Alert</h1>");
          switch(indexProjetoAtual) {
          case COMPRAS: 
            {
              client.print("<h3>Prox. Projeto: COMPRAS</h3>");
            } 
            break;
          case ESTOQUE: 
            {
              client.print("<h3>Prox. Projeto: ESTOQUE</h3>");
            } 
            break;
          case PDV: 
            {
              client.print("<h3>Prox. Projeto: PDV</h3>");
            } 
            break;
          case VPSA: 
            {
              client.print("<h3>Prox. Projeto: VPSA</h3>");
            } 
            break;
          }
          client.println("<p>");
          client.print("Leitura em: ");
          client.print((postingInterval - (millis() - lastConnectionTime)) / 1000L);
          client.println(" s.</p>");
          client.println("<p>O 'Berduino' alerta quando os testes do COMPRAS-JAVA (1 bipe), ESTOQUE-JAVA-INTEGRATION-TEST (2 bipes), OFFLINEMANAGER (3 bipes) e VPSA-JAVA-INTEGRATION-TEST (4 bipes) falharam no Jenkins!</p>");
          client.println("</body>");
          client.println("</html>");
          
          // give the web browser time to receive the data
          delay(1);
          // close the connection:
          client.stop();
          //Serial.println("client disconnected");
        }
      }
    }
  }
}

void tocarAlarme() {
  digitalWrite(GIROFLEX_PIN, HIGH);

  const int delayNota = 500;
  const int delayPausa = delayNota * 2;
  const int nota = 440;

  for (int intervalo = 0; intervalo < 3; intervalo++) {
    for (int vezes = 0; vezes < bipes[indexProjetoAtual]; vezes++) {
      tone(BUZZER_PIN, nota, delayNota);
      delay(delayPausa);
    }
    delay(4000);
  }

  digitalWrite(GIROFLEX_PIN, LOW);
}

void httpRequest() {
  if (client.connect(hudsonIp, 9080)) {
    //Serial.println("connecting...");

    client.println("GET /hudson/job/" + projetos[indexProjetoAtual] + "/api/json?tree=color HTTP/1.0");
    client.println();

    //Serial.println();
    //Serial.println("Projeto: " + projetos[indexProjetoAtual]);
    lastConnectionTime = millis();
  } 
  else {
    //Serial.println("connection failed");
    //Serial.println("disconnecting.");
    client.stop();

    if (retryCount < 3) {
      retryCount++;
      httpRequest(); 
    } 
    else {
      retryCount = 0;  
    }
  }
}
