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
#define AIO_USERNAME "8mamo10"
#define AIO_KEY "aio_xxxxxxxxxx"
#define READ_TIMEOUT 5000

// Wi-Fi
const char* fname = "/config.csv";

File fp;
char ssid[32];
char pass[32];

void SetwifiSD(const char *file){
  unsigned int cnt = 0;
  char data[256];
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

  M5.Lcd.printf("SSID:%s\n",ssid);
  M5.Lcd.printf("PASS:%s\n",pass);
  M5.Lcd.println("Connecting...");

  Serial.printf("SSID:%s\n",ssid);
  Serial.printf("PASS:%s\n",pass);

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

WiFiClientSecure client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe onMeetingIndicator = Adafruit_MQTT_Subscribe(&mqtt, MRHS_FEED);

void setup() {
  Serial.begin(115200);
  M5.begin();
  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);

  SetwifiSD(fname);

  M5.Lcd.println(F("Ok"));

  mqtt.subscribe(&onMeetingIndicator);

  delay(2000);
}

void loop() {
  M5.update();

  MQTT_connect();
  Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(READ_TIMEOUT))) {
    if (subscription == &onMeetingIndicator) {
      String lastread = String((char*)onMeetingIndicator.lastread);
      lastread.trim();
      if (lastread == "") {
        continue;
      }
      if (lastread == "0") {
        showOk();
      } else {
        showNg();
      }
    }
  }
}

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
  if (mqtt.connected()) {
    return;
  }

  M5.Lcd.setTextSize(2);
  //M5.Lcd.setCursor(0,0);
  M5.Lcd.print(F("Connecting to MQTT... "));

  while ((ret = mqtt.connect()) != 0) {
    M5.Lcd.println(F(mqtt.connectErrorString(ret)));
    M5.Lcd.println(F("Retrying MQTT connection in 5 seconds ..."));
    mqtt.disconnect();
    delay(5000);
  }

  M5.Lcd.println("MQTT connected!");
}
