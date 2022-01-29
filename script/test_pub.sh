#!/bin/sh

USERNAME=`cat .mrhs.json | jq -r '.username'`
KEY=`cat .mrhs.json | jq -r '.key'`

#echo $USERNAME
#echo $KEY

#/usr/local/bin/mosquitto_pub -h io.adafruit.com -p 8883 -t ${USERNAME}/feeds/mrhs -u ${USERNAME} -P ${KEY} --tls-version tlsv1.2 --cafile /etc/ssl/cert.pem -s
echo $1 | /usr/local/bin/mosquitto_pub -h io.adafruit.com -p 8883 -t ${USERNAME}/feeds/mrhs -u ${USERNAME} -P ${KEY} --tls-version tlsv1.2 --cafile /etc/ssl/cert.pem -s
