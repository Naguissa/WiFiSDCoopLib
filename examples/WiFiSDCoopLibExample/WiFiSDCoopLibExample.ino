/**
 * On SD card, connected to SPI1 by default, you can store following files:
 * www/__pre => Webpage header (html, title, body)
 * www/__post => Webpage footer  ( footer, /body and  /html)
 * files/<any file> => Can be accessed by /files/<filename>. WARNING: Binary files not supported.
 *
 * WiFi setup:
 *  You can change _setupESP() function to fit your AP. If not
 *  ESP will wake-up in AP mode creating an open network with SSID default an without password.
 *
 * Note you may need edit LED_PIN define to fit your board
 *
 * While configuring SD and WiFi LED is ON; once everything is configured LED goes OFF. Notice tht it goes OFF EVEN if configuration is not successfull.
 *
 * To archieve SD reloading you need to add:
 *
 * root.close();
 *
 * on SD.cpp file, function begin, around line 343, just before return sentence.
 *
 * @copyright Naguissa
 * @author Naguissa
 * @email naguissa.com@gmail.com
 * @version 1.0.0
 * @created 2015-06-13
 */



#include <Arduino.h>
#define SD_CS 10
#define LED_PIN 33

#include <SPI.h>
#include <SD.h>
#include <WiFiSDCoopLib.h>




bool SDConnected = false;

WiFiSDCoopLib ESP;



void _setupESP() {
    // To JOIN an AP as a STA device:
    // ESP.setMode('1');
    // ESP.setSSID("SSID");
    // ESP.setPass("PASSWORD");
    ESP.reinit();
}



void _web_header(const unsigned char ipd) {
if (SDConnected) {
    ESP.sendFileByIPD(ipd, "www/__pre");
  } else {
    ESP.sendDataByIPD(ipd, F("<html><head><title>WiFiSDCoopLib Example</title></head><body>"));
    ESP.sendDataByIPD(ipd, F("<h2>Warning</h2>"));
    ESP.sendDataByIPD(ipd, F("<p>SD card failed to start.</p>"));
    ESP.sendDataByIPD(ipd, F("<p><a href=\"/resetSD\">Insert SD and click here.</a></p><br>"));
  }
}

void _web_footer(const unsigned char ipd) {
  if (SDConnected) {
    ESP.sendFileByIPD(ipd, "www/__post");
  } else {
    ESP.sendDataByIPD(ipd, F("</body></html>"));
  }
}

void errorRoute(const String route, const unsigned char ipd) {
  _web_header(ipd);
  ESP.sendDataByIPD(ipd, F("<h2>404</h2>"));
  ESP.sendDataByIPD(ipd, F("<h3>Page not found</h3>"));
  ESP.sendDataByIPD(ipd, F("<p><a href=\"/\">Return home</a></p>"));
  _web_footer(ipd);

}

void indexRoute(const String route, const unsigned char ipd) {
  _web_header(ipd);
  ESP.sendDataByIPD(ipd, "<h1>Status</h1><div class=\"table2\"><div>SD card error</div><div>");
  ESP.sendDataByIPD(ipd, SDConnected ? "No" : "Yes");
  ESP.sendDataByIPD(ipd, "</div></div>");
  _web_footer(ipd);
}


void filesRoute(const String route, const unsigned char ipd) {
  char charbuff[route.length()];
  route.substring(1).toCharArray(charbuff, route.length());
  ESP.sendFileByIPD(ipd, charbuff);
}

void resetSDRoute(const String route, const unsigned char ipd) {
  _setupSD();
  _web_header(ipd);
  ESP.sendDataByIPD(ipd, F("<h1>Reset SD done</h1>"));
  ESP.sendDataByIPD(ipd, F("<p><a href=\"/\">Return home</a></h1>"));
  _web_footer(ipd);
}

void _setupSD() {
  SDConnected = SD.begin(SD_CS);
}


void setup() {

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  pinMode(SD_CS, OUTPUT);
  delay(4000);
  _setupSD();
  _setupESP();
  ESP.attachRoute("/files/", filesRoute, 1);
  ESP.attachRoute("/resetSD", resetSDRoute, 0);
  ESP.attachRoute("/", indexRoute, 0);
  ESP.attachRoute("", errorRoute, 4);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  ESP.wifiLoop();
}

