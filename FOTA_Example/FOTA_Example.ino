/* ----------------------------------------------------------------------------

FOTA_Example.ino : Firmware Over The Air Example using FOTA.ino

'Firmware Over The Air' allows you to upload sketches remotely.
 ... No need to connect to your computer when you have an upgrade

Enter the right values in the 'Credentials' section below.
Compile and watch the Serial Monitor to see the url you can browse to.

---------------------------------------------------------------------------- */

#ifdef ESP8266
  #include <ESPAsyncTCP.h>
#elif defined (ESP32)
  #include <AsyncTCP.h>
  #include <Update.h>
#else
  #error Platform not supported  
#endif
#include <ESPAsyncWebServer.h>

// ----- Credentials

String WiFi_ssid = "SSID";
String WiFi_password = "PASSWORD";
String FOTA_User = "a";
String FOTA_Password = "a";

// ----- Home page

const char indexhtml[] PROGMEM = R"rawliteral(<!DOCTYPE HTML>
<html lang="en">
<head>
<meta name='viewport' content='width=device-width, height=device-height, initial-scale=1, user-scalable=no'>
<style>
  .rnd_btn {
   background-color: lightgrey;
   border-radius: 50%;
   border-width: 3px;
   border-color: gold;
   color: blue;
   width: 100px;
   height: 50px;
   text-align: center;
  }
</style>
<title>FOTA Example</title>
<body style='background-color: #E6E6FA;'>
<center>
  <h1>FOTA Example</h1>
  Firmware Over The Air<br><br>
  <a href='/FOTA/FOTA'>
   <button class="rnd_btn">FOTA</button>
  </a>
</center>
</body>
</html>
)rawliteral";

// ----- Web page not found

void notFound(AsyncWebServerRequest *request) {
  Serial.println("Client: "
                  + request->client()->remoteIP().toString()
                  + " " + request->url());
  request->send(404, "text/plain", "Not found");
}

// ----- Web Server

AsyncWebServer WebServer(80);

// ----- WiFi

bool initWiFi() {
  Serial.println("\nConnect to WiFi ...");
  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.persistent(false);  // do not store WiFi credentials in flash
  WiFi.mode(WIFI_STA);
  WiFi.begin(WiFi_ssid, WiFi_password);
  int count = 0;
  while ((WiFi.status() != WL_CONNECTED) && (count <= 50)) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(200);
    count = count + 1;
  }
  digitalWrite(LED_BUILTIN, 1);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No WiFi ...");
    return false;
  }
  return true;
}
// ----- Setup

void setup() {

  Serial.begin(115200);
  delay(5000);
  Serial.println("\n\nStartup begin...");

  initWiFi();

  Serial.println("\nWebserver Setup...");
 
  WebServer.onNotFound(notFound);

  WebServer.on("/", HTTP_GET,  [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", indexhtml) ;
  } );

  FOTAConfigureWebServer(WebServer,FOTA_User, FOTA_Password);

  WebServer.begin();

  Serial.println("\nStartup complete...");

  Serial.println("\nBrowse to ... http://"+WiFi.localIP().toString());

}

// ----- Loop

void loop() {

}
