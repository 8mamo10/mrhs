#!/bin/sh
username=xxxxxxxxxx
key=yyyyyyyyyy

/usr/sbin/lsof | /usr/bin/grep VDC | /usr/bin/wc -l | /usr/local/bin/mosquitto_pub -h io.adafruit.com -p 8883 -t ${username}/feeds/mrhs -u ${username} -P ${key} --tls-version tlsv1.2 --cafile /etc/ssl/cert.pem -s
