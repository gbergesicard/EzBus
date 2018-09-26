#include <string>               // Strings
#include <ESP8266WiFi.h>        // WiFi driver
#include <ESP8266WebServer.h>   // WebServer management
#include <DNSServer.h>          // DNS management
#include "ArduinoJson.h"        // JSON parser
#include <FS.h>                 // File system management
#include <SPI.h>                // SPI management for RFID reader
#include <MFRC522.h>            // RFID reader management
#include <LiquidCrystal_I2C.h>  // LCD screen management
#include <Ticker.h>             // LCD backlight timeout management

#define REV "REV0001"

// LED pins
#define LED_PIN_RED   0 
#define LED_PIN_GREEN 2

// MFRC522 pins
#define RST_PIN 16 // D0
#define SS_PIN 15 // D8

#define BACKLIGHT_TIMER 5 // 5 seconds

int debug = 1;
File fsUploadFile;              // a File object to temporarily store the received file
ESP8266WebServer server(80);    // Web server on port 80 
WiFiClient client;              // wifi client 
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display
Ticker backlightTimer;
IPAddress apIP(192, 168, 1, 1);
// for the captive network
const byte DNS_PORT = 53;
DNSServer dnsServer;            // dns server for captive wifi
int captiveNetwork = 0;
StaticJsonBuffer<20480> jsonBuffer;
String jsonFileName="/EZBus.json";
String cssFileName="/EZBus.css";
String css ="";
JsonObject* root;
char meta[] = "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";

String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)
void handleFileUpload();                // upload a new file to the SPIFFS

// Generate the server page
void root_Page() {
  traceChln("Serving Root Page");
  String s="";
  // Generate the html root page
  s = "<!DOCTYPE HTML>";
  s += meta;
  s += "<html>";
  s += "<head><style>"+css+"</style><title>EZBus</title></head>"; 
  s += "<body>";
  s +="<h1>EzBus</h1> ";
  s +="<a href=\"/settings\">Settings</a><br>";
  s +="<a href=\"/travel\">Voyage</a><br>";
  s += "</body>";
  s += "</html>";
  server.send( 200 , "text/html", s);
}
void settings_Page() {
  traceChln("Serving settings Page");
  String s="";
  // Generate the html root page
  s = "<!DOCTYPE HTML>";
  s += meta;
  s += "<html>";
  s += "<head><style>"+css+"</style><title>EZBus</title></head>"; 
  s += "<body>";
  s +="<h1>EzBus Settings</h1> ";
  s +="<a href=\"/up\">Upload file</a><br>";
  s +="<br><br><a href=\"/\">Home</a><br>";
  s += "</body>";
  s += "</html>";
  server.send( 200 , "text/html", s);
}
void travel_Page() {
  traceChln("Serving travel Page");
  String s="";
  // Generate the html root page
  s = "<!DOCTYPE HTML>";
  s += meta;
  s += "<html>";
  s += "<head><style>"+css+"</style><title>EZBus</title></head>"; 
  s += "<body>";
  s +="<h1>EzBus travel</h1> ";
  s +="<a href=\"/passengers\">Liste voyageurs</a><br>";
  s +="<a href=\"/\">Liste voyageurs pr&eacute;sents &agrave; l&#039;&eacute;tape</a><br>";
  s +="<a href=\"/\">Liste voyageurs manquants &agrave; l&#039;&eacute;tape</a><br>";
  s +="<br><br><a href=\"/\">Home</a><br>";
  s += "</body>";
  s += "</html>";
  server.send( 200 , "text/html", s);
}

void upload_Page(){
  traceChln("Serving Upload Page");
  String s="";
  // build page
  s = "<!DOCTYPE html>";
  s += meta;
  s += "<html>";
  s += "<head><style>"+css+"</style><title>EZBus</title></head>"; 
  s += "<body>";
  s += "<h1>EzBus File Upload</h1>";
  s += "<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">";
  s += "    <input type=\"file\" name=\"name\">";
  s += "    <input class=\"button\" type=\"submit\" value=\"Upload\">";
  s += "</form>";
  s +="<br><br><a href=\"/\">Home</a><br>";
  s += "</body>";
  s += "</html>";
  // Send page
  server.send( 200 , "text/html", s);
}
void passenger_Page() {
  traceChln("Serving travel Page");
  int arraySize =  (*root)["Passengers"].size();
  traceCh("Nb Passengers :");
  traceChln(String(arraySize));
  String s="";
  // Generate the html root page
  s = "<!DOCTYPE HTML>";
  s += meta;
  s += "<html>";
  s += "<head><style>"+css+"</style><title>EZBus</title></head>"; 
  s += "<body>";
  s +="<h1>Liste des Voyageurs</h1> ";
  s +="<table>";
  s +="<h2>Nombre de voyageurs total : "+String(arraySize)+"</h2>";
  s +="<tr>";
  s +="  <th>Nom</th>";
  s +="  <th>Num&eacute;ro badge</th>";
  s +="</tr>";
  // loop on the liste elements 
  for(short wni = 0;wni <arraySize;wni++){
    s +="<tr>";
    const char* Name = (*root)["Passengers"][wni]["Name"];
    const char* Tag = (*root)["Passengers"][wni]["Tag"];
    s +="<td>";
    s +=Name;
    s +="</td>";
    s +="<td>";
    s +=Tag;
    s +="</td>";
    s +="</tr>";
  }
  s +="</table>";
  s +="<br><br><a href=\"/\">Home</a><br>";
  s += "</body>";
  s += "</html>";
  server.send( 200 , "text/html", s);
}


/*
 * Callback for backlight saver management
 */
void callbackBacklight() {
  lcd.noBacklight();
  backlightTimer.detach();
}

void setup() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("EZBus");
  delay(100); //Stable AP
  if(debug == 1){
    Serial.begin(115200); //Set Baud Rate
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer
  }

  // Setting LED pins
  pinMode(LED_PIN_RED, OUTPUT);
  pinMode(LED_PIN_GREEN, OUTPUT);

  // Init the LCD
  lcd.begin();

  // Init SPI bus
  SPI.begin();

  // Init MFRC522 card
  mfrc522.PCD_Init();

  // initilize file system
  SPIFFS.begin();

  if (isFileExists(jsonFileName)){
    updateJson(jsonFileName,&root);
  }
  if (isFileExists(cssFileName)){
    loadCss(cssFileName);
  }
  dnsServer.start(DNS_PORT, "*", apIP);
  server.onNotFound(root_Page);
  server.on("/",root_Page);
  server.on("/up",upload_Page);
  server.on("/settings",settings_Page);
  server.on("/travel",travel_Page);
  server.on("/passengers",passenger_Page);
  
  server.on("/upload", HTTP_POST,                       // if the client posts to the upload page
    [](){ server.send(200); },                          // Send status 200 (OK) to tell the client we are ready to receive
    handleFileUpload                                    // Receive and save the file
  );
  server.on("/upload", HTTP_GET, []() {                 // if the client requests the upload page
  if (!handleFileRead(jsonFileName))                // send it if it exists
    server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });
  
  server.begin();
  // captive network is active 
  captiveNetwork = 1;

  // Setup end
  lcdPrint(0,0,"EZBus ready!");
  lcdPrint(1,0,REV);
}

void loop() {
  digitalWrite(LED_PIN_RED, LOW);
  digitalWrite(LED_PIN_GREEN, LOW);
  
  if (captiveNetwork == 1){
    dnsServer.processNextRequest();
  }
  server.handleClient();

  // Look for new cards
  if (mfrc522.PICC_IsNewCardPresent()) {
    digitalWrite(LED_PIN_RED, HIGH);
    
    // Select one of the cards
    if ( mfrc522.PICC_ReadCardSerial()) {
      clearLine(1);
      String tagId = getStringFromByteArray(mfrc522.uid.uidByte, mfrc522.uid.size);

      traceChln("Tag Id: " + tagId);
      lcdPrint(0,0,"Tag");
      lcdPrint(1,0,tagId);

      // TO DO: search the TagId in JSON
      /*if (existTag(tagId)) {
          digitalWrite(LED_PIN_RED, LOW);
          digitalWrite(LED_PIN_GREEN, HIGH);
          wait(1000);
        }
      }*/
    }
  }
}

String getStringFromByteArray(byte *buffer, byte bufferSize) {
  String myString = "";
  for (byte i = 0; i < bufferSize; i++) {
      myString += (buffer[i] < 0x10 ? "0" : "");
      myString += String(buffer[i], HEX);
  }
  return myString;
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
  traceChln("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    traceChln(String("\tSent file: ") + path);
    return true;
  }
  traceChln(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
}

void handleFileUpload(){ // upload a new file to the SPIFFS
  traceChln("uploading a file !");
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    jsonFileName = filename;
    traceCh("handleFileUpload Name: "); traceChln(filename);
    fsUploadFile = SPIFFS.open(filename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      traceCh("handleFileUpload Size: "); traceChln(String(upload.totalSize));
      traceChln("file name : "+upload.filename);
      server.sendHeader("Location","/success.html");      // Redirect the client to the success page
      server.send(303);
      updateJson(jsonFileName,&root);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

void updateJson(String fileName,JsonObject** Proot){
  traceChln("updateJson load file");
  traceChln(fileName);
  if(fileName == ""){
    return;
  }
  File dataFile = SPIFFS.open(fileName, "r");   //open file (path has been set elsewhere and works)
  String json = dataFile.readString();                    // read data to 'json' variable
  dataFile.close();                                       // close file
  traceChln(json);
  jsonBuffer.clear();
  *Proot =  &jsonBuffer.parseObject(json);
  if((**Proot).invalid()==JsonObject::invalid()){
    traceChln("Failed to parse Json");
  }
  int arraySize =  (**Proot)["Passengers"].size();
  String s=String(arraySize);
  traceChln("Nb liste : "+s);
}


void loadCss(String fileName){
    traceChln("loadCss load file");
    traceChln(fileName);
  if(fileName == ""){
    return;
  }
  File dataFile = SPIFFS.open(fileName, "r");   //open file (path has been set elsewhere and works)
  css = dataFile.readString();                  // read data to 'css' variable
  dataFile.close();                             // close file
  traceChln(css);
}

bool isFileExists(String filename){
  return(SPIFFS.exists(filename));
}

void traceChln(char* chTrace){
  if (debug == 1){
    Serial.println(chTrace);
  }
}
void traceChln(String chTrace){
  if (debug == 1){
    Serial.println(chTrace);
  }
}
void traceCh(char* chTrace){
  if (debug == 1){
    Serial.print(chTrace);
  }
}
void traceCh(String chTrace){
  if (debug == 1){
    Serial.print(chTrace);
  }
}

// LCD management
void lcdPrint(int line, int col, String text) {
  lcd.setCursor(col,line);
  lcd.print(text);
  lcd.backlight();

  backlightTimer.attach(BACKLIGHT_TIMER, callbackBacklight);
}

void lcdPrintPassager(String passenger){
  clearLine(1);
  lcdPrint(1,0,passenger);
}

void clearLine(int ligne){
  lcd.setCursor(0,ligne);
  lcd.print("                    ");
}
