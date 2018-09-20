#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <string>
#include <DNSServer.h>
#include "ArduinoJson.h"
#include <FS.h> // file system management

#define REV "REV0001"

int debug = 1;
File fsUploadFile;              // a File object to temporarily store the received file
ESP8266WebServer server(80);    // Web server on port 80 
WiFiClient client;              // wifi client 
IPAddress apIP(192, 168, 1, 1);
// for the captive network
const byte DNS_PORT = 53;
DNSServer dnsServer;            // dns server for captive wifi
int captiveNetwork = 0;
DynamicJsonBuffer jsonBuffer(10240);
String Gjson="";
String jsonFileName="/EZBus.json";
JsonObject& root = jsonBuffer.parseObject(Gjson);

String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)
void handleFileUpload();                // upload a new file to the SPIFFS

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

void UploadPage(){
  traceChln("Serving Upload Page");
  String s="";
  // build page
  s = "<!DOCTYPE html>";
  s += "<html>";
  s += "<body>";
  s += "<h1>EzBus File Upload</h1>";
  s += "<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">";
  s += "    <input type=\"file\" name=\"name\">";
  s += "    <input class=\"button\" type=\"submit\" value=\"Upload\">";
  s += "</form>";
  s += "</body>";
  s += "</html>";
  // Send page
  server.send( 200 , "text/html", s);
}

void setup() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP("Wifi device setup");
    delay(100); //Stable AP
    if(debug == 1){
      Serial.begin(115200); //Set Baud Rate
      Serial.print("IP address:\t");
      Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer
    }

    // initilize file system
    SPIFFS.begin();

    if (isFileExists(jsonFileName)){
      UpdateJson(jsonFileName,&root);
    }
    dnsServer.start(DNS_PORT, "*", apIP);
    server.onNotFound(D_AP_SER_Page);
    server.on("/",D_AP_SER_Page);
    server.on("/upload", HTTP_POST,                       // if the client posts to the upload page
      [](){ server.send(200); },                          // Send status 200 (OK) to tell the client we are ready to receive
      handleFileUpload                                    // Receive and save the file
    );
    server.on("/upload", HTTP_GET, []() {                 // if the client requests the upload page
    if (!handleFileRead("/EZBus.json"))                // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
    });
    server.on("/up",UploadPage);
    
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

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
}

void handleFileUpload(){ // upload a new file to the SPIFFS
  traceChln("uploading a file !");
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    jsonFileName = filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location","/success.html");      // Redirect the client to the success page
      server.send(303);
      UpdateJson(jsonFileName,&root);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

void UpdateJson(String fileName,JsonObject* Proot){
  if(fileName == ""){
    return;
  }
  if (debug == 1){
    Serial.println("UpdateJson load file");
    Serial.println(fileName);
  }
  File dataFile = SPIFFS.open(fileName, "r");   //open file (path has been set elsewhere and works)
  String json = dataFile.readString();                    // read data to 'json' variable
  dataFile.close();                                       // close file
  if (debug == 1){
    Serial.println(json);
  }
  Proot = (JsonObject*) jsonBuffer.parseObject(json);
  char chTempo[30];
  int arraySize =  (*Proot)["Liste"].size();

  chTempo[0] ='\0';
  String s="";
  
  sprintf(chTempo,"Nb liste :%d",arraySize);
  traceChln(chTempo);
}

bool isFileExists(String filename){
  return(SPIFFS.exists(filename));
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
