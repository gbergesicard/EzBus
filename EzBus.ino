#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <string>
#include <DNSServer.h>
#include "ArduinoJson.h"

#define REV "REV0001"

int debug = 1;
ESP8266WebServer server(80);//Specify port 
WiFiClient client;
IPAddress apIP(192, 168, 1, 1);
// for the captive network
const byte DNS_PORT = 53;
DNSServer dnsServer;
int captiveNetwork = 0;
DynamicJsonBuffer jsonBuffer(10240);
char json[]="{\"Liste\":[{\"Nom\":\"FAVRE-FELIXALEXIS\",\"Tag\":\"04708EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX1\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX2\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX3\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX4\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX5\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX6\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX7\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX8\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX9\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX10\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX11\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX12\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX13\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX14\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX15\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX16\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX17\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX18\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX19\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX20\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX21\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX22\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX23\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX24\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX25\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX26\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX27\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX28\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX29\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX30\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX31\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX32\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX33\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX34\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX35\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX36\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX37\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX38\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX39\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX40\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX41\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX42\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX43\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX44\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX45\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX46\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX47\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX48\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX49\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX50\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX51\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX52\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX53\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX54\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX55\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX56\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX57\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX58\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX59\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX60\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX61\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX62\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]},{\"Nom\":\"DUPONTALEX63\",\"Tag\":\"046C8EE2AC5C80\",\"Incidents\":0,\"MP\":[17,50],\"ME\":[17,50]}]}";
JsonObject& root = jsonBuffer.parseObject(json);

void dumpEEPROM(){
  char output[10];
  Serial.println("dumpEEPROM start");
  for(int i =0;i<70;i++){
    snprintf(output,10,"%x ",EEPROM.read(i));
    Serial.print(output);
  }
  Serial.println("\ndumpEEPROM stop");
}

// Clear Eeprom
void ClearEeprom(){
  if (debug == 1){
    Serial.println("Clearing Eeprom");
  }
  for (int i = 0; i < 500; ++i) { EEPROM.write(i, 0); }
  if (debug == 1){
    Serial.println("Clearing Eeprom end");
  }
}


void resetSettings(){
  if (debug == 1){
    Serial.println("");
    Serial.println("Reset EEPROM");  
    Serial.println("Rebooting ESP");
  }
  ClearEeprom();//First Clear Eeprom
  server.send(200, "text/plain", "Reseting settings, ESP will reboot soon");
  delay(100);
  EEPROM.commit();
  ESP.restart();
}

// Generate the server page
void D_AP_SER_Page() {
  int Tnetwork=0,i=0,len=0;
  char chTempo[30];
  int arraySize =  root["Liste"].size();

  chTempo[0] ='\0';
  String s="";
  Tnetwork = WiFi.scanNetworks(); // Scan for total networks available
  IPAddress ip = WiFi.softAPIP(); // Get ESP8266 IP Adress
  
  sprintf(chTempo,"Nb liste :%d",arraySize);
  traceChln("Serving Page");
  traceChln(chTempo);
  // Generate the html setting page
  s = "\n\r\n<!DOCTYPE HTML>\r\n<html><h1>EzBus</h1> ";
  s += "<p>";
  s += "</html>\r\n\r\n";
  server.send( 200 , "text/html", s);
}

void setup() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP("Wifi device setup");
    delay(100); //Stable AP
    if(debug == 1){
      Serial.begin(115200); //Set Baud Rate
    }
 
    dnsServer.start(DNS_PORT, "*", apIP);
    server.onNotFound(D_AP_SER_Page);
    server.on("/",D_AP_SER_Page); 
    server.begin();
    // captive network is active 
    captiveNetwork = 1;
}

void loop() {

  if (captiveNetwork == 1){
    dnsServer.processNextRequest();
  }
  server.handleClient();
}

void traceChln(char* chTrace){
  if (debug == 1){
    Serial.println(chTrace);
  }
}
void traceCh(char* chTrace){
  if (debug == 1){
    Serial.print(chTrace);
  }
}
