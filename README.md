# mrhs
- Monitoring Running Hangout Status
- M5Stack + MQTT

# MQTT broker
- Using io.adafruit this time.
- Sign up to https://io.adafruit.com/
- Create Feeds `mrhs`
  - https://io.adafruit.com/[Username]/feeds/mrhs
- Create Dashboard `mrhs`
  - https://io.adafruit.com/[Username]/dashboards/mrhs

# MQTT client
- Using MQTT.fx
  - http://mqttfx.jensd.de/
- Download and install it
- Open the app and add a connection
  - Profile Name: `mrhs`
  - Profile Type: `MQTT Broker`
  - Broker Address: `io.adafruit.com`
  - Broken Port: `8883`
  - Client ID: `MQTT_FX_Client` (anything is ok)
  - User Credentials
    - User Name: [Username] of adafruit
    - Password: [Active Key] of adafruit
      - Can get from Afafruit IO Key
  - SSL/TLS
    - Check Enable SSL/TLS
    - Protocol: `TLSv1.2`
- Connect
- Publish
  - Input topic: `[Username]/feeds/mrhs`
  - Input any string and push `Publish`
- Subscribe
  - Input topic: `[Username]/feeds/mrhs`
  - Push `Subscribe`

# references
- https://learn.adafruit.com/naming-things-in-adafruit-io/naming-and-accessing-feeds-with-the-mqtt-api
- https://io.adafruit.com/api/docs/mqtt.html#adafruit-io-mqtt-api
