#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>

const char* ssid = "UC SumoBot 2";      // Replace with your desired AP SSID
const char* password = "12345678";  // Replace with your desired AP password

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create a WebSocket object

AsyncWebSocket ws("/ws");
// Set LED GPIO
const int ledPin1 = 14;
const int ledPin2 = 12;
const int ledPin3 = 13;

String message = "";
String sliderValue1 = "0";
String sliderValue2 = "0";
String sliderValue3 = "0";

boolean DirA = HIGH;
boolean DirB = HIGH;


int dutyCycle1;
int dutyCycle2;
int dutyCycle3;

int PWMA=5;//Right side 
int PWMB=4;//Left side 
int DA=0;//Right reverse 
int DB=2;//Left reverse 

int motorspeed = 500;

//Json Variable to Hold Slider Values
JSONVar sliderValues;


// Initialize WiFi
void initWiFi() {
  // WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid, password);
  // Serial.print("Connecting to WiFi ..");
  // while (WiFi.status() != WL_CONNECTED) {
  //   Serial.print('.');
  //   delay(1000);
  // }
  // Serial.println(WiFi.localIP());

    // Set the ESP8266 to AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.print("AP Mode: ");
  Serial.println(ssid);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}

void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else{
   Serial.println("LittleFS mounted successfully");
  }
}

//Get Slider Values
String getSliderValues(){
  sliderValues["sliderValue1"] = String(sliderValue1);
  sliderValues["sliderValue2"] = String(sliderValue2);
  sliderValues["sliderValue3"] = String(sliderValue3);

  String jsonString = JSON.stringify(sliderValues);
  return jsonString;
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    message = (char*)data;
    if (message.indexOf("1s") >= 0) {
      sliderValue1 = message.substring(2);
      dutyCycle1 = map(sliderValue1.toInt(), 0, 100, 0, motorspeed);
//      Serial.print(getSliderValues());
      

      
      if(dutyCycle1 < 0){
        dutyCycle1 = dutyCycle1 * -1;
        DirA = HIGH;
      } else {
        DirA = LOW;
      }

      digitalWrite(PWMA, dutyCycle1);
      digitalWrite(DA, DirA);
      Serial.println(dutyCycle1);
      Serial.println(DirA);

      notifyClients(getSliderValues());
      
    }
    if (message.indexOf("2s") >= 0) {
      sliderValue2 = message.substring(2);
      dutyCycle2 = map(sliderValue2.toInt(), 0, 100, 0, motorspeed);
//      Serial.print(getSliderValues());
      

      if(dutyCycle2 < 0){
        dutyCycle2 = dutyCycle2 * -1;
        DirB = LOW;
      } else {
        DirB = HIGH;
      }

      digitalWrite(PWMB, dutyCycle2);
      digitalWrite(DB, DirB);
      Serial.println(dutyCycle2);
      Serial.println(DirB);

      notifyClients(getSliderValues());

      
    }    
    if (strcmp((char*)data, "getValues") == 0) {
      notifyClients(getSliderValues());
    }
  }
}
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void notifyClients(String sliderValues) {
  ws.textAll(sliderValues);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  //Init motor shield
  pinMode(PWMA, OUTPUT); 
  pinMode(PWMB, OUTPUT); 
  pinMode(DA, OUTPUT); 
  pinMode(DB, OUTPUT); 
  
  //init file system untuk webnya
  initFS();

  //init wifi
  initWiFi();

  //init websocket
   initWebSocket();
  
  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });
  
  server.serveStatic("/", LittleFS, "/");

  // Start server
  server.begin();

}

void loop() {
  
  

  ws.cleanupClients();

}
