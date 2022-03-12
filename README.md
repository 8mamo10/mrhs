# mrhs

- Monitoring Running Hangout Status
- M5Stack + MQTT

## Deploy MRHS agent

```
$ cd agent
$ go build -o path/to/mrhsagent mrhs.go
$ sudo ln -s path/to/mrhsagent /usr/local/bin/mrhsagent
$ sudo ln -s path/to/mrhs.service /lib/systemd/system/mrhs.service
$ sudo ln -s path/to/.calendar.json /etc/mrhs/.calendar.json
$ sudo ln -s path/to/.adafruit.json /etc/mrhs/.adafruit.json
$ sudo ln -s path/to/.credential.json /etc/mrhs/.credential.json
$ sudo systemctl restart mrhs.service
```

## MRHS cli(deprecated)

- Update `username` and `key` on `script/.mrhs.json`
- Set up cron at an appropriate frequency

```
# Every minute
*   * * * * ${path/to/mrhs_agent}/mrhs_cli.sh

# Every 5minutes
*/5 * * * * ${path/to/mrhs_agent}/mrhs_cli.sh
```

## Notes

### MQTT broker

- Using io.adafruit this time.
- Sign up to https://io.adafruit.com/
- Create Feeds `mrhs`
  - https://io.adafruit.com/[Username]/feeds/mrhs
- Create Dashboard `mrhs`
  - https://io.adafruit.com/[Username]/dashboards/mrhs

### MQTT client

- Using MQTT.fx
  - http://mqttfx.jensd.de/
- Download and install it
- Open the app and add a connection
  - Profile Name: `mrhs`
  - Profile Type: `MQTT Broker`
  - Broker Address: `io.adafruit.com`
  - Broker Port: `8883`
  - Client ID: `MQTT_FX_Client`
    - Anything is ok
    - Do `Generate` is also good
  - User Credentials
    - User Name: `[Username] of adafruit`
    - Password: `[Active Key] of adafruit`
      - Can get from Afafruit IO Key screen
  - SSL/TLS
    - Check `Enable SSL/TLS`
    - Protocol: `TLSv1.2`
- Connect
- Publish
  - Input topic: `[Username]/feeds/mrhs`
  - Input any string and push `Publish`
- Subscribe
  - Input topic: `[Username]/feeds/mrhs`
  - Push `Subscribe`

### M5Stack

- https://github.com/8mamo10/m5stack

### Adafruit_MQTT

- Sketch -> Include Library -> Manage Libraries
  - Adafruit MQTT Library (vresion was 1.3.0 when I did)

### mosquitto

- https://mosquitto.org/
- `brew install mosquitto`

```
$ /usr/local/sbin/mosquitto -c /usr/local/etc/mosquitto/mosquitto.conf
1600654250: mosquitto version 1.6.12 starting
1600654250: Config loaded from /usr/local/etc/mosquitto/mosquitto.conf.
1600654250: Opening ipv6 listen socket on port 1883.
1600654250: Opening ipv4 listen socket on port 1883.
1600654250: mosquitto version 1.6.12 running
1600654260: New connection from ::1 on port 1883.
1600654260: New client connected from ::1 as SUB (p2, c1, k60).
1600654278: New connection from ::1 on port 1883.
1600654278: New client connected from ::1 as PUB (p2, c1, k60).
1600654278: Client PUB disconnected.
1600654282: Client SUB disconnected.
^C1600654290: mosquitto version 1.6.12 terminating
```

```
$ mosquitto_sub -t test -i SUB
Hello
```

```
$ mosquitto_pub -t test -m Hello -i PUB
```

```
$ mosquitto_sub -d -h test.mosquitto.org -p 8883 -t "#" --tls-version tlsv1.2 --cafile mosquitto.org.crt
```

```
$ mosquitto_sub -d -h io.adafruit.com -p 1883 -t ${username}/feeds/mrhs -u ${username} -P ${key}
```

```
$ mosquitto_pub -d -h io.adafruit.com -p 1883 -t ${username}/feeds/mrhs -u ${username} -P ${key} -m 1
```

```
$ mosquitto_pub -L mqtt://${username}:${key}@io.adafruit.com/${username}/feeds/mrhs -s
```

```
$ mosquitto_sub -d -h io.adafruit.com -p 8883 -t ${username}/feeds/mrhs -u ${username} -P ${key} --tls-version tlsv1.2 --cafile /etc/ssl/cert.pem
```

### References

- https://learn.adafruit.com/naming-things-in-adafruit-io/naming-and-accessing-feeds-with-the-mqtt-api
- https://learn.adafruit.com/mqtt-adafruit-io-and-you/overview
- https://io.adafruit.com/api/docs/mqtt.html#adafruit-io-mqtt-api
- https://io.adafruit.com/api/docs/mqtt.html#mqtt-connection-details
