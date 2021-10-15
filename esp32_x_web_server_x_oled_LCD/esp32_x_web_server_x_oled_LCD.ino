#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include "ESPAsyncWebServer.h"
#include <Adafruit_Sensor.h>



#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define DHTPIN 15
#define DHTTYPE DHT11
 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
//------------------Servidor Web en puerto 80---------------------

WiFiServer server(80);

//---------------------Credenciales de WiFi-----------------------

const char* ssid     = "MALUPONTON";
const char* password = "Paulette20";

//---------------------VARIABLES GLOBALES-------------------------
int contconexion = 0;

String header; // Variable para guardar el HTTP request

String estadoSalida = "off";

const int verde = 33;
const int blanco =32;
const int rojo =27;
const int amarillo =26;

//------------------------CODIGO HTML------------------------------
String pagina = "<!DOCTYPE html>"
"<html>"

"<head>"
    "<meta charset='UTF-8'>"
    "<meta http-equiv='X-UA-Compatible' content='IE=edge'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<title>Servidor Web ESP32</title>"
    "<link rel='preconnect' href='https://fonts.googleapis.com'>"
    "<link rel='preconnect' href='https://fonts.gstatic.com' crossorigin>"
    "<link href='https://fonts.googleapis.com/css2?family=Open+Sans:ital,wght@0,400;1,700&display=swap' rel='stylesheet'>"
    "<style>"
        ".button{"
            "border:none;"
            "outline: none;"
            "padding: 10px 15px;"
            "cursor: pointer;"
            "border-radius: 5px;"
            "color: white;"
            "width: 150px;"
            "transition: 0.2s;"
        "}"
        ".verde{"
            "background-color: chartreuse;"
        "}"
        ".rojo{"
            "background-color: crimson;"
        "}"
        ".blanco{"
            "background-color: darkgrey;"
        "}"
        ".amarillo{"
            "background-color: yellow;"
            "color: black;"
    
        "}"
        "body{"
            "color: white;"
            "background: rgb(2,0,36);"
            "background: linear-gradient(180deg, rgba(45,81,228,1) 50%, rgba(0,212,255,1) 100%);"
            "display: flex;"
            "flex-direction: column;"
            "justify-content: center;"
            "width: 100vw;"
            "align-items: center;"
            "font-family: 'Open Sans', sans-serif;"
            "font-weight: 700;"
            "font-style: italic;"
        "}"
    "</style>"
"</head>"

"<body>"

    "<h1>Servidor Web ESP32</h1>"

    "<h2>Foco Verde</h2>"
    "<p><a href='/onV'><button class='button verde'>ON</button></a></p>"
    "<p><a href='/offV'><button class='button verde'>OFF</button></a></p>"
    "<div></div>"
    "<h2>Foco Blanco</h2>"
    "<p><a href='/onB'><button class='button blanco'>ON</button></a></p>"
    "<p><a href='/offB'><button class='button blanco'>OFF</button></a></p>"
    "<div></div>"
    "<h2>Foco Rojo</h2>"
    "<p><a href='/onR'><button class='button rojo'>ON</button></a></p>"
    "<p><a href='/offR'><button class='button rojo'>OFF</button></a></p>"
    "<div></div>"
    "<h2>Foco Amarillo</h2>"
    "<p><a href='/onA'><button class='button amarillo'>ON</button></a></p>"
    "<p><a href='/offA'><button class='button amarillo'>OFF</button></a></p>"

"</body>"

"</html>";


DHT dht(DHTPIN, DHTTYPE);

//---------------------------SETUP--------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("");
  dht.begin();
  pinMode(verde, OUTPUT); 
  pinMode(blanco, OUTPUT); 
  pinMode(rojo, OUTPUT); 
  pinMode(amarillo, OUTPUT); 
  digitalWrite(verde, LOW);
  digitalWrite(blanco, LOW);
  digitalWrite(rojo, LOW);
  digitalWrite(amarillo, LOW);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);

  display.println("Conectando...");
  display.display();
  
  
  // Conexión WIFI
  WiFi.begin(ssid, password);
  //Cuenta hasta 50 si no se puede conectar lo cancela
  while (WiFi.status() != WL_CONNECTED and contconexion <50) { 
    ++contconexion;
    delay(500);
    Serial.print(".");
  }
  if (contconexion <50) {
      
      Serial.println("");
      Serial.println("WiFi conectado");
      display.clearDisplay();
      display.println(WiFi.localIP());
      display.display(); 
      server.begin(); // iniciamos el servidor
  }
  else { 
      Serial.println("");
      Serial.println("Error de conexion");
  }
}

//----------------------------LOOP----------------------------------

void loop(){
  float h = dht.readHumidity();
  // Leemos la temperatura en grados centígrados (por defecto)
  float t = dht.readTemperature();
  // Leemos la temperatura en grados Fahreheit
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Error obteniendo los datos del sensor DHT11");
    return;
  }
  
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("Temperatura: ");
  display.setCursor(0, 20);
  display.println(t);
  display.setCursor(40, 20);
  display.println("Grados celsius");
  display.setCursor(55, 40);
  display.setTextSize(1);
  display.println("IP: ");
  display.setCursor(20, 50);
  display.println(WiFi.localIP());
  display.display();
  WiFiClient client = server.available();   // Escucha a los clientes entrantes

  if (client) {                             // Si se conecta un nuevo cliente
    Serial.println("New Client.");          // 
    String currentLine = "";                //
    while (client.connected()) {            // loop mientras el cliente está conectado
      if (client.available()) {             // si hay bytes para leer desde el cliente
        char c = client.read();             // lee un byte
        Serial.write(c);                    // imprime ese byte en el monitor serial
        header += c;
        if (c == '\n') {                    // si el byte es un caracter de salto de linea
          // si la nueva linea está en blanco significa que es el fin del 
          // HTTP request del cliente, entonces respondemos:
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // enciende y apaga el GPIO
            if (header.indexOf("GET /onV") >= 0) {
              Serial.println("GPIO onV");
              estadoSalida = "on";
              digitalWrite(verde, HIGH);
            } else if (header.indexOf("GET /offV") >= 0) {
              Serial.println("GPIO offV");
              estadoSalida = "off";
              digitalWrite(verde, LOW);
            } else if (header.indexOf("GET /onB") >= 0) {
              Serial.println("GPIO onB");
              estadoSalida = "on";
              digitalWrite(blanco, HIGH);
            } else if (header.indexOf("GET /offB") >= 0) {
              Serial.println("GPIO off");
              estadoSalida = "offB";
              digitalWrite(blanco, LOW);
            } else if (header.indexOf("GET /onR") >= 0) {
              Serial.println("GPIO onR");
              estadoSalida = "on";
              digitalWrite(rojo, HIGH);
            } else if (header.indexOf("GET /offR") >= 0) {
              Serial.println("GPIO offR");
              estadoSalida = "off";
              digitalWrite(rojo, LOW);
            } else if (header.indexOf("GET /onA") >= 0) {
              Serial.println("GPIO onA");
              estadoSalida = "on";
              digitalWrite(amarillo, HIGH);
            } else if (header.indexOf("GET /offA") >= 0) {
              Serial.println("GPIO off");
              estadoSalida = "offA";
              digitalWrite(amarillo, LOW);
            }
            
            
            // Muestra la página web
            client.println(pagina);
            
            // la respuesta HTTP temina con una linea en blanco
            client.println();
            break;
          } else { // si tenemos una nueva linea limpiamos currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // si C es distinto al caracter de retorno de carro
          currentLine += c;      // lo agrega al final de currentLine
        }
      }
    }
    // Limpiamos la variable header
    header = "";
    // Cerramos la conexión
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
