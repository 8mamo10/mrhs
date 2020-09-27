#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <string.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 8883
#define AIO_FEEDS "/feeds/mrhs"
#define READ_TIMEOUT 5000

// Wi-Fi
const char* fname = "/config.csv";

// config
File fp;
char ssid[32];
char pass[32];
char username[64];
char key[64];

// adafruit
char feed[64];

void SetwifiSD(const char *file){
  unsigned int cnt = 0;
  char data[256];
  char *str;

  fp = SD.open(fname, FILE_READ);
  while(fp.available()){
    data[cnt++] = fp.read();
  }
  Serial.print("data:");
  Serial.print("\n-----\n");
  Serial.print(data);
  Serial.print("\n-----\n");

  strtok(data,",");
  str = strtok(NULL,"\r");
  strncpy(&ssid[0], str, strlen(str));

  strtok(NULL,",");
  str = strtok(NULL,"\r");
  strncpy(&pass[0], str, strlen(str));

  strtok(NULL,",");
  str = strtok(NULL,"\r");
  strncpy(&username[0], str, strlen(str));

  strtok(NULL,",");
  str = strtok(NULL,"\r");
  strncpy(&key[0], str, strlen(str));

  M5.Lcd.printf("SSID:%s\n", ssid);
  M5.Lcd.printf("PASS:%s\n", pass);
  M5.Lcd.println("Connecting...");

  Serial.printf("SSID:%s\n", ssid);
  Serial.printf("PASS:%s\n", pass);
  Serial.printf("USERNAME:%s\n", username);
  Serial.printf("KEY:%s\n", key);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.print("\n");

  M5.Lcd.print("IP: ");
  M5.Lcd.println(WiFi.localIP());
  Serial.printf("IP: ");
  Serial.println(WiFi.localIP());
  fp.close();
}

WiFiClientSecure client;
Adafruit_MQTT_Client *mqtt;
Adafruit_MQTT_Subscribe *onMeetingIndicator;

void showOnMeeting(uint16_t bgColor) {
  M5.Lcd.fillScreen(bgColor);
  M5.Lcd.setCursor(40, 90);
  M5.Lcd.setTextSize(7);
  M5.Lcd.println(F("ON MEETING"));
}

void showOk() {
  M5.Lcd.fillScreen(TFT_GREEN);
  // 320 * 240
  M5.Lcd.fillEllipse(160, 120, 100, 100, TFT_WHITE);
  M5.Lcd.fillEllipse(160, 120, 75, 75, TFT_GREEN);
}

void showNg() {
  M5.Lcd.fillScreen(TFT_RED);
  // 320 * 240
  //M5.Lcd.fillRect(60, 110, 200, 20, TFT_WHITE);
  //M5.Lcd.fillRect(150, 20, 20, 200, TFT_WHITE);
  for (int i=0; i <= 10; i++) {
    M5.Lcd.drawLine(60+i, 20-i, 260+i, 220-i, TFT_WHITE);
    M5.Lcd.drawLine(60-i, 20+i, 260-i, 220+i, TFT_WHITE);
    M5.Lcd.drawLine(60+i, 220+i, 260+i, 20+i, TFT_WHITE);
    M5.Lcd.drawLine(60-i, 220-i, 260-i, 20-i, TFT_WHITE);
  }
}

void MQTT_connect() {
  int8_t ret;
  if (mqtt->connected()) {
    return;
  }

  Serial.println(F("Connecting to MQTT... "));
  while ((ret = mqtt->connect()) != 0) {
    Serial.println(F(mqtt->connectErrorString(ret)));
    Serial.println(F("Retrying MQTT connection in 5 seconds ..."));
    mqtt->disconnect();
    delay(5000);
  }
  Serial.println(F("MQTT connected!"));
}

void setup() {
  Serial.begin(115200);
  M5.begin();
  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);

  SetwifiSD(fname);

  M5.Lcd.println(F("Ok"));

  mqtt = new Adafruit_MQTT_Client(&client, AIO_SERVER, AIO_SERVERPORT, username, username, key);
  String mrhs_feed = String(username) + AIO_FEEDS;
  // mrhs_feed itself is not suitable for passing to Adafruit_MQTT_Subscribe
  // since it exists on this scope, so store it to persistent char array.
  memmove(feed, mrhs_feed.c_str(), mrhs_feed.length());
  onMeetingIndicator = new Adafruit_MQTT_Subscribe(mqtt, feed);
  mqtt->subscribe(onMeetingIndicator);

  delay(2000);
  showOk();
}

void loop() {
  M5.update();

  MQTT_connect();
  Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt->readSubscription(READ_TIMEOUT))) {
    if (subscription == onMeetingIndicator) {
      String lastread = String((char*)onMeetingIndicator->lastread);
      lastread.trim();
      if (lastread == "") {
        continue;
      }
      // Two processes are already in use before the camera was actually used.
      // (avconfere and google chrome)
      if (lastread.toInt() <= 2) {
        showOk();
      } else {
        showNg();
      }
    }
  }
}
