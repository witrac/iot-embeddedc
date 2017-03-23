#!/bin/bash
echo "Backing up lib directory"
rm -rf lib_bk
mv lib lib_bk
mkdir tmp
mkdir lib
mkdir lib/mbedtls
cd tmp
echo "Downloading paho mqtt embedded-c v1.0.0 source ...."
curl -LO https://github.com/eclipse/paho.mqtt.embedded-c/archive/v1.0.0.tar.gz
tar -xvf v1.0.0.tar.gz
echo "Downloading cJSON master.zip ...."
curl -LO https://github.com/DaveGamble/cJSON/archive/master.zip
unzip master.zip
echo "Downloading mbedTLS-2.4.1 source ...."
curl -LO https://github.com/ARMmbed/mbedtls/archive/mbedtls-2.4.1.tar.gz
tar -xvf mbedtls-2.4.1.tar.gz
cd paho.mqtt.embedded-c-1.0.0/MQTTClient-C/src/
mv MQTTClient.h MQTTClient_old.h
sed -e 's/""/"iotf_network_tls_wrapper.h"/' MQTTClient_old.h > MQTTClient.h
cd -
echo "Copying the necessary source files to lib"
cp paho.mqtt.embedded-c-1.0.0/MQTTPacket/src/* ../lib/
cp paho.mqtt.embedded-c-1.0.0/MQTTClient-C/src/MQTTClient.* ../lib/
cp mbedtls-mbedtls-2.4.1/include/mbedtls/*.h ../lib/mbedtls
cp mbedtls-mbedtls-2.4.1/library/*.c ../lib/
cp cJSON-master/cJSON.* ../lib/
cp ../lib_bk/CMakeLists.txt ../lib/
cd ..
echo "Removing the temporary files"
rm -rf ./tmp/
rm -rf ./lib_bk/
echo "Finished Setup"
