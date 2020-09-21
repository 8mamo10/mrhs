#!/bin/sh
username=xxxxxxxxxx
key=yyyyyyyyyy

/usr/sbin/lsof | /usr/bin/grep VDC | /usr/bin/wc -l | /usr/local/bin/mosquitto_pub -L mqtt://${username}:${key}@io.adafruit.com/${username}/feeds/mrhs -s

