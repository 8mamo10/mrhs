#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <string.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "8mamo10"
#define AIO_KEY "aio_xxxxxxxxxx"
#define READ_TIMEOUT 5000

// Wi-Fi
const char* fname = "/wifi.csv";
File fp;
char ssid[32];
char pass[32];

void SetwifiSD(const char *file){
  unsigned int cnt = 0;
  char data[64];
  char *str;

  fp = SD.open(fname, FILE_READ);
  while(fp.available()){
    data[cnt++] = fp.read();
  }
  Serial.print("data: ");
  Serial.print(data);
  Serial.print("\n");

  strtok(data,",");
  str = strtok(NULL,"\r"); // CR
  strncpy(&ssid[0], str, strlen(str));

  strtok(NULL,",");
  str = strtok(NULL,"\r"); // CR
  strncpy(&pass[0], str, strlen(str));

  M5.Lcd.printf("WIFI-SSID: %s\n",ssid);
  M5.Lcd.printf("WIFI-PASS: %s\n",pass);
  M5.Lcd.println("Connecting...");

  Serial.printf("SSID = %s\n",ssid);
  Serial.printf("PASS = %s\n",pass);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  M5.Lcd.print("IP: ");
  M5.Lcd.println(WiFi.localIP());
  Serial.printf("IP: ");
  Serial.println(WiFi.localIP());
  fp.close();
}

// adafruit
const char MQTT_SERVER[]   PROGMEM = AIO_SERVER;
const char MQTT_USERNAME[] PROGMEM = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM = AIO_KEY;
const char MRHS_FEED[]     PROGMEM = AIO_USERNAME "/feeds/mrhs";

//Adafruit_MQTT_Client mqtt(&ctx, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);
//Adafruit_MQTT_Subscribe onAirIndicator = Adafruit_MQTT_Subscribe(&mqtt, MRHS_FEED);

void setup() {
  M5.begin();
  M5.Lcd.setTextSize(2);

  SetwifiSD(fname);
}

void loop() {
  // put your main code here, to run repeatedly:
}
