#!/bin/sh

USERNAME=`cat .mrhs.json | jq -r '.username'`
KEY=`cat .mrhs.json | jq -r '.key'`

/usr/sbin/lsof | /usr/bin/grep VDC | /usr/bin/wc -l | /usr/local/bin/mosquitto_pub -h io.adafruit.com -p 8883 -t ${USERNAME}/feeds/mrhs -u ${USERNAME} -P ${KEY} --tls-version tlsv1.2 --cafile /etc/ssl/cert.pem -s
