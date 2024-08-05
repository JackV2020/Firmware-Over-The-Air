/* ----------------------------------------------------------------------------

FOTA.ino : 'Firmware Over The Air' upload sketches remotely to ESP8266/ESP32. 

  For your main app :

    Use the library manager to install some libraries. 
    Install 'ESP Async WebServer' by Me-No-Dev.
    For the ESP8266 install 'ESPAsyncTCP' by dvarrelInstall.
    For the ESP32 install 'Async TCP' by Me-No_Dev (Mathieu Carbou).
    Note the spaces in the names 'Async TCP' and 'ESP Async WebServer'.

  After creating your webserver named MyWebServer with :
    AsyncWebServer MyWebServer(80);
  call:
    FOTAConfigureWebServer(MyWebServer,User,Password); 
  and you can browse to http://'ipadress'/FOTA/FOTA

  For Me :

    This FOTA works for Wemos D1 mini Pro which is ESP8266 based.
    I only have Wemos D1 mini Pro so can not develop and test for 
    other devices based on ESP8266, ESP etc.
    Did you get this running for another type ? Please let me know.

    https://github.com/JackV2020/Firmware-Over-The-Air

  Thanks to Henk Holdijk for his feedback concerning ESP32

---------------------------------------------------------------------------- */

//#include "FOTA_Basic.h"
//#include "FOTA.h"
#include "FOTA_Cloud.h"

// =====  Configure web server

void FOTAConfigureWebServer(AsyncWebServer &server, String &username, String &password) {

  Serial.println("\nConfiguring FOTA ...");

  FOTA_user = username;
  FOTA_password = password;

  // ----- The 'Update Firmware Over The air' web page

  server.on("/FOTA/FOTA", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!FOTA_isAuthenticated(request)) {
      return;
    }
    request->send(200, "text/html", FOTA_html);
  });

  // ----- Perform the firmware upload

  server.on(
    "/FOTA/upload", HTTP_POST,
  // This is part 1 and is executed after Part 2 below (upload) finishes
  [](AsyncWebServerRequest *request) {
    
    if (!FOTA_isAuthenticated(request)) {
      return;
    }
    Serial.println("FOTA Close Connection");
    AsyncWebServerResponse *response = request->beginResponse((Update.hasError()) ? 400 : 200, "text/plain", (Update.hasError()) ? FOTA_update_error.c_str() : "Update oke");
    response->addHeader("Connection", "close");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
    if (Update.hasError()) {
      Serial.println("FOTA Update Error");
    } else {
      Serial.println("FOTA Update Oke");
    }
  },
  // This is part 2 and is the actual upload and is executed before Part 1 above
  [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!FOTA_isAuthenticated(request)) {
      return;
    }
    if (!index) {
      // Initialise when we receive the first data frame
      FOTA_bytes_written = 0;
      FOTA_update_error = "";

#ifdef ESP8266
        Update.runAsync(true);
        if (!Update.begin(((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000))) {
        Serial.println("FOTA Start Not Ok");
        FOTA_update_error = Update.getErrorString(); 
        FOTA_update_error.concat("\n");
        } else {
          Serial.println("FOTA Start Oke");
        }
#elif defined (ESP32)
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
        Serial.println("FOTA Start Not Ok");
        FOTA_update_error = Update.errorString();
        FOTA_update_error.concat("\n");
        } else {
          Serial.println("FOTA Start Oke");
        }
#endif
    }

    if (len) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      if (Update.write(data, len) != len) {
        return request->send(400, "text/plain", "Failed to save update");
      }
      FOTA_bytes_written += len;
      Serial.printf_P(PSTR("\nFOTA Saving data len: %d ; total: %d"), len, FOTA_bytes_written);
    }

    if (final) {
      Serial.println("\nFOTA bytes written: " + String(FOTA_bytes_written));
      digitalWrite(LED_BUILTIN,1);
      if (!Update.end(true)) {  // true sets the size to the amount of saved bytes
#ifdef ESP8266
          FOTA_update_error = Update.getErrorString();
#elif defined (ESP32)
          FOTA_update_error = Update.errorString();
#endif
        FOTA_update_error.concat("\n");
      }
    } else {
      return;
    }
  });

  // ----- Serve the reboot button on the FOTA web page

  WebServer.on("/FOTA/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!FOTA_isAuthenticated(request)) {
      return;
    }
    Serial.println("Reboot");
    ESP.restart();
  });

}

// ===== Check authentication

bool FOTA_isAuthenticated(AsyncWebServerRequest *request) {
  if (!request->authenticate(FOTA_user.c_str(), FOTA_password.c_str())) {
    request->requestAuthentication();
    return false;
  }
  return true;
}

// ----------------------------------------------------------------------------
