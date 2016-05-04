#!/bin/bash
echo "Cleaning lib"
rm -rf lib
mkdir tmp
mkdir lib
cd tmp
echo "downloading paho embedded-C...."
curl -LO http://git.eclipse.org/c/paho/org.eclipse.paho.mqtt.embedded-c.git/snapshot/org.eclipse.paho.mqtt.embedded-c-1.0.0.tar.gz
tar -xvf org.eclipse.paho.mqtt.embedded-c-1.0.0.tar.gz
echo "downloading cJSON...."
curl -LO https://github.com/DaveGamble/cJSON/archive/master.zip
unzip master.zip
echo "downloading iot-embeddedc...."
curl -LO https://github.com/ibm-watson-iot/iot-embeddedc/archive/master.zip
unzip master.zip
cd org.eclipse.paho.mqtt.embedded-c-1.0.0/MQTTClient-C/src/
sed -i 's/""/"MQTTLinux.h"/g' MQTTClient.h
cd -
echo "copying the necessary files to lib"
cp org.eclipse.paho.mqtt.embedded-c-1.0.0/MQTTPacket/src/* ../lib/
cp org.eclipse.paho.mqtt.embedded-c-1.0.0/MQTTClient-C/src/MQTTClient.* ../lib/
cp org.eclipse.paho.mqtt.embedded-c-1.0.0/MQTTClient-C/src/linux/MQTTLinux.* ../lib/
cp cJSON-master/cJSON.* ../lib/
cd ..
echo "removing the temporary files"
rm -rf ./tmp/
echo "finished setup"

