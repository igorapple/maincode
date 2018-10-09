#include <ESP8266WiFi.h> //provides us the methods to connect to a WiFi network.
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <FS.h>

ESP8266WebServer server;
int LED = 2;
char* ssid = "DOMEK";
char* password = "Kondominium12";

int ledpin = 2; // D1(gpio5)
int button = 4; //D2(gpio4)
int buttonState=0;

int deviceid=4;
int connectedid[10];
int commands[10];
String devicepurpose="nadajnik";

String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

void setup()
{
  pinMode(4, INPUT);
  SPIFFS.begin();
  pinMode(2, OUTPUT);
  WiFi.begin(ssid,password);
  Serial.begin(115200);
  while(WiFi.status()!=WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/index.html", SPIFFS, "/index.html");
  server.serveStatic("/konfiguruj.html", SPIFFS, "/konfiguruj.html");
  server.serveStatic("/komendy_plytki.html", SPIFFS, "/komendy_plytki.html");
  server.serveStatic("/steruj.html", SPIFFS, "/steruj.html");
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.serveStatic("/config.json", SPIFFS, "/config.json");
  server.on("/setid", setid);
  server.on("/setcommand", setcommand);
  server.on("/deletecommand", deletecommand);
  server.on("/deleteid", deleteid);
  server.on("/loadconfig", loadconfig);
  server.on("/saveconfig", saveconfig);
  server.on("/show", show);
  server.on("/showconfigfile", showconfigfile);
  
  server.on("/data", []() {
    StaticJsonBuffer<300> jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
    json["myid"] = deviceid;
    json["purpose"] = devicepurpose;
    JsonArray& idki = json.createNestedArray("id");
    JsonArray& komendy = json.createNestedArray("commands");
    for (int i = 0; i < 10; i++) {
       int y=connectedid[i];
       int x=commands[i];
       idki.add(y);
       komendy.add(x);
    }
    
    char jsonchar[300];
    json.printTo(jsonchar); //print to char array, takes more memory but sends in one piece
    server.send(200, "application/json", jsonchar);
  });
  
  server.begin();
  loadconfig();
    
  Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {    
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
  Serial.printf("\n");
  
}

void loop(){
  server.handleClient();
   buttonState=digitalRead(button); // put your main code here, to run repeatedly:
 if (buttonState == 1)
 {
 digitalWrite(ledpin, HIGH); 
 delay(200);
 }
 if (buttonState==0)
 {
 digitalWrite(ledpin, LOW); 
 delay(200);
 }
}

void loadconfig(){
  File conf = SPIFFS.open("/config.json", "r");
  if (!conf){
    Serial.println("file open failed");
  }
  size_t size = conf.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
  }
  
  std::unique_ptr<char[]> buf(new char[size]);
  conf.readBytes(buf.get(), size);
  
  StaticJsonBuffer<300> jBuffer;
  JsonObject& jObject = jBuffer.parseObject(buf.get());

  if (!jObject.success()) {
    Serial.println("Failed to parse config file");
  }
  
  for (int i = 0; i < 10; i++) { //Iterate through results 
    connectedid[i] = jObject["id"][i];  //Implicit cast 
    Serial.print(connectedid[i]);
  }
  Serial.println("");
  for (int i = 0; i < 10; i++) { //Iterate through results 
    commands[i] = jObject["commands"][i];  //Implicit cast 
    Serial.print(commands[i]);
  }
  Serial.println("");
  conf.close();
}

void saveconfig(){
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  json["myid"]=deviceid;
  json["purpose"]=devicepurpose;
  
  JsonArray& idki = json.createNestedArray("id");
  JsonArray& komendy = json.createNestedArray("commands");
  for (int i = 0; i < 10; i++) {
       int y=connectedid[i];
       int x=commands[i];
       idki.add(y);
       komendy.add(x);
  }

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
  }
  
  json.printTo(configFile);
  Serial.println("zapisano config.json");  
  configFile.close();
}

void showconfigfile(){
  File conf = SPIFFS.open("/config.json", "r");
  if (!conf){
    Serial.println("file open failed");
  }
  size_t size = conf.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
  }
  
  std::unique_ptr<char[]> buf(new char[size]);
  conf.readBytes(buf.get(), size);
  int tab1[10];
  int tab2[10];
  StaticJsonBuffer<300> jBuffer;
  JsonObject& jObject = jBuffer.parseObject(buf.get());

  if (!jObject.success()) {
    Serial.println("Failed to parse config file");
  }
  
  for (int i = 0; i < 10; i++) { //Iterate through results 
    tab1[i] = jObject["id"][i];
  }
  Serial.println("");
  for (int i = 0; i < 10; i++) { //Iterate through results 
    tab2[i]=(jObject["commands"][i]);
  }
  conf.close();
}

void show(){
  Serial.println(deviceid);
  for (int i = 0; i < 10; i++) { //Iterate through results 
    Serial.print(connectedid[i]);
  }
  Serial.println("");
  for (int i = 0; i < 10; i++) { //Iterate through results 
    Serial.print(commands[i]);
  }
  Serial.println("");
  Serial.println(devicepurpose);
}

//dodawanie id oraz komend
int addidf(int a){
  for (int i=0; i<10; i++){
    if (connectedid[i]==a){
      break;
    }
    else if (connectedid[i]==0){
      connectedid[i]=a;
      break;
    }
  }
  show();
  saveconfig();
}

int addcommandf(int a){
  for (int i=0; i<10; i++){
    if (commands[i]==a){
      break;
    }
    if (commands[i]==0){
      commands[i]=a;
      break;
    }
  }
  saveconfig();
}

//usuwanie id oraz komend
int deleteidf(int a){
  for (int i=0; i<10; i++){
    if (connectedid[i]==a){
      connectedid[i]=0;
    }
  }
  saveconfig();
}

int deletecommandf(int a){
  for (int i=0; i<10; i++){
    if (commands[i]==a){
      commands[i]=0;
    }
  }
  saveconfig();
}

//otrzymywanie informacji z webservera
void setid(){
  String data = server.arg("plain");
  StaticJsonBuffer<200> jBuffer;
  JsonObject& jObject = jBuffer.parseObject(data);
  int id = jObject["id"];
  server.send(204,"");
  addidf(id);
}
void deleteid(){
  String data = server.arg("plain");
  StaticJsonBuffer<200> jBuffer;
  JsonObject& jObject = jBuffer.parseObject(data);
  int id = jObject["id"];
  server.send(204,"");
  deleteidf(id);
}
void setcommand(){
  String data = server.arg("plain");
  StaticJsonBuffer<200> jBuffer;
  JsonObject& jObject = jBuffer.parseObject(data);
  int commands = jObject["commands"];
  server.send(204,"");
  addcommandf(commands);
}
void deletecommand(){
  String data = server.arg("plain");
  StaticJsonBuffer<200> jBuffer;
  JsonObject& jObject = jBuffer.parseObject(data);
  int commands = jObject["commands"];
  server.send(204,"");
  deletecommandf(commands);
}
