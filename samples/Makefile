all: sampleDevice helloWorld sampleGateway sampleDeviceManagement sampleDMAction sampleDMFirmwareActions

sampleDevice: sampleDevice.c
	$(CC) sampleDevice.c -I ./../src/ -I ./../lib ./../src/iotfclient.c ./../lib/MQTTClient.c ./../lib/MQTTLinux.c ./../lib/MQTTFormat.c  ./../lib/MQTTPacket.c ./../lib/MQTTDeserializePublish.c ./../lib/MQTTConnectClient.c ./../lib/MQTTSubscribeClient.c ./../lib/MQTTSerializePublish.c ./../lib/MQTTConnectServer.c ./../lib/MQTTSubscribeServer.c ./../lib/MQTTUnsubscribeServer.c ./../lib/MQTTUnsubscribeClient.c -o $@
	strip $@

helloWorld: helloWorld.c
	$(CC) helloWorld.c -I ./../src/ -I ./../lib ./../src/iotfclient.c ./../lib/MQTTClient.c ./../lib/MQTTLinux.c ./../lib/MQTTFormat.c  ./../lib/MQTTPacket.c ./../lib/MQTTDeserializePublish.c ./../lib/MQTTConnectClient.c ./../lib/MQTTSubscribeClient.c ./../lib/MQTTSerializePublish.c ./../lib/MQTTConnectServer.c ./../lib/MQTTSubscribeServer.c ./../lib/MQTTUnsubscribeServer.c ./../lib/MQTTUnsubscribeClient.c -o $@
	strip $@

sampleGateway: sampleGateway.c ./../src/gatewayclient.c
	$(CC) sampleGateway.c -I ./../src/ -I ./../lib ./../src/gatewayclient.c ./../lib/MQTTClient.c ./../lib/MQTTLinux.c ./../lib/MQTTFormat.c  ./../lib/MQTTPacket.c ./../lib/MQTTDeserializePublish.c ./../lib/MQTTConnectClient.c ./../lib/MQTTSubscribeClient.c ./../lib/MQTTSerializePublish.c ./../lib/MQTTConnectServer.c ./../lib/MQTTSubscribeServer.c ./../lib/MQTTUnsubscribeServer.c ./../lib/MQTTUnsubscribeClient.c -o $@
	strip $@

sampleDeviceManagement: sampleDeviceManagement.c ./../src/devicemanagementclient.c ./../src/iotfclient.c
	$(CC) sampleDeviceManagement.c -I ./../src/ -I ./../lib ./../src/iotfclient.c ./../lib/cJSON.c ./../src/devicemanagementclient.c ./../lib/MQTTClient.c ./../lib/MQTTLinux.c ./../lib/MQTTFormat.c  ./../lib/MQTTPacket.c ./../lib/MQTTDeserializePublish.c ./../lib/MQTTConnectClient.c ./../lib/MQTTSubscribeClient.c ./../lib/MQTTSerializePublish.c ./../lib/MQTTConnectServer.c ./../lib/MQTTSubscribeServer.c ./../lib/MQTTUnsubscribeServer.c ./../lib/MQTTUnsubscribeClient.c -o $@ -lm
	strip $@

sampleDMAction: sampleDMActions.c ./../src/devicemanagementclient.c ./../src/iotfclient.c
	$(CC) sampleDMActions.c -I ./../src/ -I ./../lib ./../src/iotfclient.c ./../lib/cJSON.c ./../src/devicemanagementclient.c ./../lib/MQTTClient.c ./../lib/MQTTLinux.c ./../lib/MQTTFormat.c  ./../lib/MQTTPacket.c ./../lib/MQTTDeserializePublish.c ./../lib/MQTTConnectClient.c ./../lib/MQTTSubscribeClient.c ./../lib/MQTTSerializePublish.c ./../lib/MQTTConnectServer.c ./../lib/MQTTSubscribeServer.c ./../lib/MQTTUnsubscribeServer.c ./../lib/MQTTUnsubscribeClient.c -o $@ -lm
	strip $@

sampleDMFirmwareActions: sampleDMFirmwareActions.c ./../src/devicemanagementclient.c ./../src/iotfclient.c 
	$(CC) sampleDMFirmwareActions.c -I ./../src/ -I ./../lib ./../src/iotfclient.c ./../lib/cJSON.c ./../src/devicemanagementclient.c ./../lib/MQTTClient.c ./../lib/MQTTLinux.c ./../lib/MQTTFormat.c  ./../lib/MQTTPacket.c ./../lib/MQTTDeserializePublish.c ./../lib/MQTTConnectClient.c ./../lib/MQTTSubscribeClient.c ./../lib/MQTTSerializePublish.c ./../lib/MQTTConnectServer.c ./../lib/MQTTSubscribeServer.c ./../lib/MQTTUnsubscribeServer.c ./../lib/MQTTUnsubscribeClient.c -o $@ -lm
	strip $@

clean:
	rm -f sampleDevice helloWorld sampleGateway sampleDeviceManagement sampleDMAction sampleDMFirmwareActions
