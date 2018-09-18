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
  String s="";
  Tnetwork = WiFi.scanNetworks(); // Scan for total networks available
  IPAddress ip = WiFi.softAPIP(); // Get ESP8266 IP Adress
  if (debug == 1){
    Serial.println("Serving Page");
  }
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
