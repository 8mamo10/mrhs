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
const char *fname = "/config.csv";

// io.adafruit.com root CA
const char *adafruitio_root_ca =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n"
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n"
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n"
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n"
    "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n"
    "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n"
    "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n"
    "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n"
    "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n"
    "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n"
    "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n"
    "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n"
    "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n"
    "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n"
    "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n"
    "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n"
    "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n"
    "-----END CERTIFICATE-----\n";

// config
File fp;
char ssid[32];
char pass[32];
char username[64];
char key[64];

// adafruit
char feed[64];

// for debug console
String debugMsg;
bool isDebug = false;
bool isOk = true;

void SetupWifi(const char *file)
{
  unsigned int cnt = 0;
  char data[256];
  char *str;

  fp = SD.open(fname, FILE_READ);
  while (fp.available())
  {
    data[cnt++] = fp.read();
  }
  Serial.print("data:");
  Serial.print("\n-----\n");
  Serial.print(data);
  Serial.print("\n-----\n");

  strtok(data, ",");
  str = strtok(NULL, "\r");
  strncpy(&ssid[0], str, strlen(str));

  strtok(NULL, ",");
  str = strtok(NULL, "\r");
  strncpy(&pass[0], str, strlen(str));

  strtok(NULL, ",");
  str = strtok(NULL, "\r");
  strncpy(&username[0], str, strlen(str));

  strtok(NULL, ",");
  str = strtok(NULL, "\r");
  strncpy(&key[0], str, strlen(str));

  M5.Lcd.printf("SSID:%s\n", ssid);
  M5.Lcd.printf("PASS:%s\n", pass);
  M5.Lcd.println("Connecting...");

  // Serial.printf("SSID:%s\n", ssid);
  // Serial.printf("PASS:%s\n", pass);
  // Serial.printf("USERNAME:%s\n", username);
  // Serial.printf("KEY:%s\n", key);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    // Serial.print(".");
  }
  // Serial.print("\n");

  M5.Lcd.print("IP: ");
  M5.Lcd.println(WiFi.localIP());
  // Serial.printf("IP: ");
  // Serial.println(WiFi.localIP());
  fp.close();

  debugMsg += "SSID:";
  debugMsg += ssid;
  debugMsg += "\n";
  debugMsg += "PASS:";
  debugMsg += pass;
  debugMsg += "\n";
  debugMsg += "IP:";
  debugMsg += ipToString(WiFi.localIP());
  debugMsg += "\n";
  debugMsg += "USERNAME:";
  debugMsg += username;
  debugMsg += "\n";
  debugMsg += "KEY:";
  debugMsg += key;
  debugMsg += "\n";
  Serial.println(debugMsg);
}

String ipToString(uint32_t ip)
{
  String result = "";

  result += String((ip & 0xFF), 10);
  result += ".";
  result += String((ip & 0xFF00) >> 8, 10);
  result += ".";
  result += String((ip & 0xFF0000) >> 16, 10);
  result += ".";
  result += String((ip & 0xFF000000) >> 24, 10);

  return result;
}

WiFiClientSecure client;
Adafruit_MQTT_Client *mqtt;
Adafruit_MQTT_Subscribe *onMeetingIndicator;

void showOnMeeting(uint16_t bgColor)
{
  M5.Lcd.fillScreen(bgColor);
  M5.Lcd.setCursor(40, 90);
  M5.Lcd.setTextSize(7);
  M5.Lcd.println(F("ON MEETING"));
}

void showOkOrNg()
{
  if (isOk)
  {
    showOk();
  }
  else
  {
    showNg();
  }
}

void showOk()
{
  if (isDebug)
  {
    showDebug();
  }
  else
  {
    M5.Lcd.fillScreen(TFT_GREEN);
    // 320 * 240
    M5.Lcd.fillEllipse(160, 120, 100, 100, TFT_WHITE);
    M5.Lcd.fillEllipse(160, 120, 75, 75, TFT_GREEN);
  }
  isOk = true;
}

void showNg()
{
  if (isDebug)
  {
    showDebug();
  }
  else
  {
    M5.Lcd.fillScreen(TFT_RED);
    // 320 * 240
    // M5.Lcd.fillRect(60, 110, 200, 20, TFT_WHITE);
    // M5.Lcd.fillRect(150, 20, 20, 200, TFT_WHITE);
    for (int i = 0; i <= 10; i++)
    {
      M5.Lcd.drawLine(60 + i, 20 - i, 260 + i, 220 - i, TFT_WHITE);
      M5.Lcd.drawLine(60 - i, 20 + i, 260 - i, 220 + i, TFT_WHITE);
      M5.Lcd.drawLine(60 + i, 220 + i, 260 + i, 20 + i, TFT_WHITE);
      M5.Lcd.drawLine(60 - i, 220 - i, 260 - i, 20 - i, TFT_WHITE);
    }
  }
  isOk = false;
}

void showDebug()
{
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println(F(debugMsg.c_str()));
}

void MQTT_connect()
{
  int8_t ret;
  if (mqtt->connected())
  {
    return;
  }

  Serial.println(F("Connecting to MQTT... "));
  while ((ret = mqtt->connect()) != 0)
  {
    Serial.println(F(mqtt->connectErrorString(ret)));
    Serial.println(F("Retrying MQTT connection in 5 seconds ..."));
    mqtt->disconnect();
    delay(5000);
  }
  Serial.println(F("MQTT connected!"));
}

void setup()
{
  // Serial.begin(115200);
  M5.begin();
  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);

  SetupWifi(fname);

  M5.Lcd.println(F("Ok"));

  client.setCACert(adafruitio_root_ca);
  mqtt = new Adafruit_MQTT_Client(&client, AIO_SERVER, AIO_SERVERPORT, username, username, key);
  String mrhs_feed = String(username) + AIO_FEEDS;
  // mrhs_feed itself is not suitable for passing to Adafruit_MQTT_Subscribe
  // since this instance exists on this scope, so store it to persistent char array.
  memmove(feed, mrhs_feed.c_str(), mrhs_feed.length());
  onMeetingIndicator = new Adafruit_MQTT_Subscribe(mqtt, feed);
  mqtt->subscribe(onMeetingIndicator);

  delay(2000);
  showOk();
}

void loop()
{
  M5.update();
  MQTT_connect();
  Adafruit_MQTT_Subscribe *subscription;

  if (M5.BtnA.isPressed())
  {
    Serial.println("ButtonA pressed");
    isDebug = true;
  }
  else if (M5.BtnB.isPressed())
  {
    Serial.println("ButtonB pressed");
    isDebug = false;
  }

  showOkOrNg();

  // loop until here without MQTT update

  while ((subscription = mqtt->readSubscription(READ_TIMEOUT)))
  {
    if (subscription == onMeetingIndicator)
    {
      String lastread = String((char *)onMeetingIndicator->lastread);
      lastread.trim();
      Serial.printf("MQTT:%s\n", lastread.c_str());
      if (lastread == "")
      {
        continue;
      }
      // Two processes are already in use before the camera was actually used.
      // (avconfere and google chrome)
      if (lastread.toInt() <= 2)
      {
        isOk = true;
      }
      else
      {
        isOk = false;
      }
      showOkOrNg();
    }
  }
}
