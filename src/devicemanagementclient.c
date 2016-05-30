/****************************************
 * Copyright (c) 2016 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Hari hara prasad Viswanathan, - initial implementation and API implementation
 *    Hari prasada Reddy P
 *    Hari Prasada Reddy P          - Added implementation for Device actions support 
 *
 ****************************************/


#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <time.h>
#include "devicemanagementclient.h"
#include "cJSON.h"

commandCallback cb;
commandCallback cbReboot;
commandCallback cbFactoryReset;
actionCallback cbFirmwareDownload;
actionCallback cbFirmwareUpdate;

//util functions
void onMessage(MessageData* md);
void messageResponse(MessageData* md);
void messageUpdate(MessageData* md);
void messageObserve(MessageData* md);
void messageCancel(MessageData* md);
void messageForAction(MessageData* md, bool isReboot);
void generateUUID(char* uuid_str);
int publish(char* publishTopic, char* data);
void getMessageFromReturnCode(int rc, char* msg);
void messageFirmwareDownload(MessageData* md);
void messageFirmwareUpdate(MessageData* md);

const char* dmUpdate = "iotdm-1/device/update";
const char* dmObserve = "iotdm-1/observe";
const char* dmCancel = "iotdm-1/cancel";
const char* dmReboot = "iotdm-1/mgmt/initiate/device/reboot";
const char* dmFactoryReset = "iotdm-1/mgmt/initiate/device/factory_reset";
const char* dmFirmwareDownload = "iotdm-1/mgmt/initiate/firmware/download";
const char* dmFirmwareUpdate = "iotdm-1/mgmt/initiate/firmware/update";

static char currentRequestID[40];
volatile int interrupt = 0;

// Handle signal interrupt
/*void sigHandler(int signo) {
	printf("SigINT received.\n");
	interrupt = 1;
}*/

/*
* Function used to initialize the IBM Watson IoT client using the config file which is generated when you register your device
*
* @param configFilePath - File path to the configuration file
*
* @return int return code
* error codes
* CONFIG_FILE_ERROR -3 - Config file not present or not in right format
*/
int initialize_configfile_dm(char *configFilePath)
{
	int rc = -1;
	rc = initialize_configfile(&dmClient.deviceClient, configFilePath);
	return rc;
}

/**
* Function used to initialize the IBM Watson IoT client
*
* @param org - Your organization ID
*
* @param type - The type of your device
*
* @param id - The ID of your device
*
* @param auth-method - Method of authentication (the only value currently supported is â€œtokenâ€�)
*
* @param auth-token - API key token (required if auth-method is â€œtokenâ€�)
*
* @return int return code
*/
int initialize_dm(char *orgId, char *deviceType, char *deviceId, char *authmethod, char *authToken)
{
	int rc = -1;
	rc = initialize(&dmClient.deviceClient, orgId, deviceType, deviceId, authmethod, authToken);
	return rc;
}

/*
* Function used to initialize the IBM Watson IoT client
*
* @return int return code
*/
int connectiotf_dm()
{
	int rc = isConnected(&dmClient.deviceClient);
	if(rc){ 							//if connected return
		printf("Client is connected\n");
		return rc;
	}

	rc = connectiotf(&dmClient.deviceClient);
	return rc;
}

/*
* Function used to Publish events from the device to the IBM Watson IoT service
*
* @param eventType - Type of event to be published e.g status, gps
*
* @param eventFormat - Format of the event e.g json
*
* @param data - Payload of the event
*
* @param QoS - qos for the publish event. Supported values : QOS0, QOS1, QOS2
*
* @return int return code from the publish
*/

int publishEvent_dm(char *eventType, char *eventFormat, unsigned char* data, enum QoS qos)
{
	int rc = -1;
	rc = publishEvent(&dmClient.deviceClient, eventType, eventFormat, data, qos);
	return rc;
}

/*
* Function used to set the Command Callback function. This must be set if you to recieve commands.
*
* @param handler Function pointer to the commandCallback. Its signature - void (*commandCallback)(char* commandName, char* payload)
* 
*/
void setCommandHandler_dm(commandCallback handler)
{
	setCommandHandler(&dmClient.deviceClient,handler );//handler
	cb = handler;
}

/**
 * Register Callback function to managed request response
 *
 * @param handler Function pointer to the commandCallback. Its signature - void (*commandCallback)(char* Status, char* requestId,            void*       payload)
 *
*/

void setManagedHandler_dm(commandCallback handler)
{
	cb = handler;
}

/**
 * Register Callback function to Reboot request
 * 
 * @param handler Function pointer to the commandCallback. Its signature - void (*commandCallback)(char* Status, char* requestId,            void*       payload)
 *
*/

void setRebootHandler(commandCallback handler)
{
	cbReboot = handler;
}

/**
 * Register Callback function to Factory reset request
 *
 * @param handler Function pointer to the commandCallback. Its signature - void (*commandCallback)(char* Status, char* requestId,            void*       payload)
 *
*/

void setFactoryResetHandler(commandCallback handler)
{
	cbFactoryReset = handler;
}

/**
 * Register Callback function to Download Firmware
 *
 * @param handler Function pointer to the actionCallback. Its signature - void (*actionCallback)()
 *
*/

void setFirmwareDownloadHandler(actionCallback handler)
{
	cbFirmwareDownload = handler;
}

/**
 * Register Callback function to update Firmware
 *
 * @param handler Function pointer to the actionCallback. Its signature - void (*actionCallback)()
 *
*/

void setFirmwareUpdateHandler(actionCallback handler)
{
	cbFirmwareUpdate = handler;
}

/*
* Function used to subscribe to all commands. This function is by default called when in registered mode.
*
* @return int return code
*/
int subscribeCommands_dm()
{
	int rc = -1;

	rc = subscribeCommands(&dmClient.deviceClient);
	if(rc >=0){
		Client *c;
		c= &(dmClient.deviceClient.c);
		// Call back handles all the requests and responses received from the Watson IoT platform
		rc = MQTTSubscribe(c, "iotdm-1/#", QOS0, onMessage);
		printf("Actions subscription :%d\n",rc);
	}else{
		printf("iotf subscribe failed ");
	}
	return rc;
}

/*
* Function used to Yield for commands.
*
* @param time_ms - Time in milliseconds
*
* @return int return code
*/
int yield_dm(int time_ms)
{
	int rc = 0;
	rc = yield(&dmClient.deviceClient, time_ms);
	return rc;
}

/*
* Function used to check if the client is connected
*
* @return int return code
*/
int isConnected_dm()
{
	return isConnected(&dmClient.deviceClient);
}

/*
* Function used to disconnect from the IBM Watson IoT service
*
* @return int return code
*/

int disconnect_dm()
{
	int rc = 0;
	rc = disconnect(&dmClient.deviceClient);
	return rc;
}

/**
* <p>Send a device manage request to Watson IoT Platform</p>
* 
* <p>A Device uses this request to become a managed device. 
* It should be the first device management request sent by the 
* Device after connecting to the IBM Watson IoT Platform. 
* It would be usual for a device management agent to send this 
* whenever is starts or restarts.</p>
* 
* <p>This method connects the device to Watson IoT Platform connect if its not connected already</p>
* 
* @param lifetime The length of time in seconds within 
*        which the device must send another Manage device request.
*        if set to 0, the managed device will not become dormant. 
*        When set, the minimum supported setting is 3600 (1 hour).
* 
* @param supportFirmwareActions Tells whether the device supports firmware actions or not.
*        The device must add a firmware handler to handle the firmware requests.
* 
* @param supportDeviceActions Tells whether the device supports Device actions or not.
*        The device must add a Device action handler to handle the reboot and factory reset requests.
*
* @param reqId Function returns the reqId if the publish Manage request is successful.
* 
* @return
*/
void publishManageEvent(long lifetime, int supportFirmwareActions,
	int supportDeviceActions, char* reqId)
{
		char uuid_str[40];
		generateUUID(uuid_str);
		strcpy(currentRequestID,uuid_str);
		char* strPayload = "{\"d\": {\"metadata\":%s ,\"lifetime\":%ld ,\"supports\": {\"deviceActions\":%d,\"firmwareActions\":%d},\"deviceInfo\": {\"serialNumber\":\"%s\",\"manufacturer\":\"%s\",\"model\":\"%s\",\"deviceClass\":\"%s\",\"description\":\"%s\",\"fwVersion\":\"%s\",\"hwVersion\":\"%s\",\"descriptiveLocation\":\"%s\"}},\"reqId\": \"%s\"}" ;//cJSON_Print(jsonPayload);
		char payload[1500];
		sprintf(payload,strPayload,dmClient.DeviceData.metadata.metadata,lifetime, supportDeviceActions, supportFirmwareActions, dmClient.DeviceData.deviceInfo.serialNumber,dmClient.DeviceData.deviceInfo.manufacturer, dmClient.DeviceData.deviceInfo.model,dmClient.DeviceData.deviceInfo.deviceClass,dmClient.DeviceData.deviceInfo.description,dmClient.DeviceData.deviceInfo.fwVersion,dmClient.DeviceData.deviceInfo.hwVersion,dmClient.DeviceData.deviceInfo.descriptiveLocation,uuid_str);
		int rc = -1;
		rc = publish(MANAGE, payload);
		if(rc == SUCCESS){
			strcpy(reqId, uuid_str);
			printf("publish success\n");
		}
}

/**
 * Moves the device from managed state to unmanaged state
 *
 * A device uses this request when it no longer needs to be managed.
 * This means Watson IoT Platform will no longer send new device management requests
 * to this device and device management requests from this device will
 * be rejected apart from a Manage device request
 *
 * @param reqId Function returns the reqId if the Unmanage request is successful.
 */
void publishUnManageEvent(char* reqId)
{
	char uuid_str[40];
	int rc = -1;
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);
	char data[70];
	sprintf(data,"{\"reqId\":\"%s\"}",uuid_str);
	rc = publish(UNMANAGE, data);
	if(rc == SUCCESS)
		strcpy(reqId, uuid_str);
}

/*
 * Update the location of the device. This method converts the
 * date in the required format. The caller needs to pass the date in string in ISO8601 format.
 *
 * @param latitude	Latitude in decimal degrees using WGS84
 *
 * @param longitude Longitude in decimal degrees using WGS84
 *
 * @param elevation	Elevation in meters using WGS84
 *
 * @param measuredDateTime When the location information is retrieved
 *
 * @param accuracy	Accuracy of the position in meters
 *
 * @param reqId Function returns the reqId if the updateLocation request is successful.
 *
 * @return code indicating whether the update is successful or not
 *        (200 means success, otherwise unsuccessful)

 */
void updateLocation(double latitude, double longitude, double elevation, char* measuredDateTime, double accuracy, char* reqId)
{
	int rc = -1;

	char uuid_str[40];
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);

	char data[500];
	sprintf(data,"{\"d\":{\"longitude\":%f,\"latitude\":%f,\"elevation\":%f,\"measuredDateTime\":\"%s\",\"accuracy\":%f},\"reqId\":\"%s\"}", latitude, longitude, elevation, measuredDateTime, accuracy, uuid_str);

	rc = publish(UPDATE_LOCATION, data);
	if(rc == SUCCESS)
		strcpy(reqId, uuid_str);
}

/*
 * Update the location of the device. This method converts the
 * date in the required format. The caller needs to pass the date in string in ISO8601 format.
 *
 * @param latitude	Latitude in decimal degrees using WGS84
 *
 * @param longitude Longitude in decimal degrees using WGS84
 *
 * @param elevation	Elevation in meters using WGS84
 *
 * @param measuredDateTime When the location information is retrieved
 * *
 * @param updatedDateTime When the location information is updated
 *
 * @param accuracy	Accuracy of the position in meters
 *
 * @param reqId Function returns the reqId if the updateLocation request is successful.
 *
 * @return code indicating whether the update is successful or not
 *        (200 means success, otherwise unsuccessful)

 */
void updateLocationEx(double latitude, double longitude, double elevation, char* measuredDateTime,char* updatedDateTime, double accuracy, char* reqId)
{
	int rc = -1;
	char uuid_str[40];
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);
		
	char data[500];
	sprintf(data,"{\"d\":{\"longitude\":%f,\"latitude\":%f,\"elevation\":%f,\"measuredDateTime\":\"%s\",\"updatedDateTime\":\"%s\",\"accuracy\":%f},\"reqId\":\"%s\"}", latitude, longitude, elevation, updatedDateTime, updatedDateTime, accuracy, currentRequestID);

	rc = publish(UPDATE_LOCATION, data);
		
	if(rc == SUCCESS)
		strcpy(reqId, uuid_str);
}

/**
 * Adds the current errorcode to IBM Watson IoT Platform.
 *
 * @param errorCode The "errorCode" is a current device error code that
 * needs to be added to the Watson IoT Platform.
 *
 * @param reqId Function returns the reqId if the addErrorCode request is successful.
 *
 * @return code indicating whether the update is successful or not
 *        (200 means success, otherwise unsuccessful)
 */
void addErrorCode(int errNum, char* reqId)
{
	char uuid_str[40];
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);
	int rc = -1;
	char data[125];
	sprintf(data,"{\"d\":{\"errorCode\":%d},\"reqId\":\"%s\"}", errNum, uuid_str);

	rc = publish(CREATE_DIAG_ERRCODES, data);
	if(rc == SUCCESS)
		strcpy(reqId, uuid_str);
}

/**
 * Clear the Error Codes from IBM Watson IoT Platform for this device
 *
 * @param reqId Function returns the reqId if the clearErrorCodes request is successful.
 *
 * @return code indicating whether the clear operation is successful or not
 *        (200 means success, otherwise unsuccessful)
 */
void clearErrorCodes(char* reqId)
{
	char uuid_str[40];
	int rc = -1;
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);

	char data[125];
	sprintf(data,"{\"reqId\":\"%s\"}", uuid_str);

	rc = publish(CLEAR_DIAG_ERRCODES, data);
	if(rc == SUCCESS)
		strcpy(reqId, uuid_str);
}

/**
 * The Log message that needs to be added to the Watson IoT Platform.
 *
 * @param message The Log message that needs to be added to the Watson IoT Platform.
 *
 * @param timestamp The Log timestamp 
 *
 * @param data The optional diagnostic string data
 *
 * @param severity The Log severity
 *
 * @param reqId Function returns the reqId if the addLog request is successful.
 *
 * @return code indicating whether the update is successful or not
 *        (200 means success, otherwise unsuccessful)
 */
void addLog(char* message, char* data ,int severity, char* reqId)
{
	char uuid_str[40];
	int rc = -1;
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);
	time_t t = 0;
	char updatedDateTime[50];//"2016-03-01T07:07:56.323Z"
	strftime(updatedDateTime, sizeof(updatedDateTime), "%Y-%m-%dT%TZ", localtime(&t));
	char payload[125];
	sprintf(payload,"{\"d\":{\"message\":\"%s\",\"timestamp\":\"%s\",\"data\":\"%s\",\"severity\":%d},\"reqId\":\"%s\"}",message,updatedDateTime,data,severity, uuid_str);

	rc = publish(ADD_DIAG_LOG, payload );
	if(rc == SUCCESS)
		strcpy(reqId, uuid_str);
}

/**
 * Clear the Logs from IBM Watson IoT Platform for this device
 *
 * @param reqId Function returns the reqId if the clearLogs request is successful.
 *
 * @return code indicating whether the clear operation is successful or not
 *        (200 means success, otherwise unsuccessful)
 */
void clearLogs(char* reqId){
	char uuid_str[40];
	int rc = -1;
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);

	char data[125];
	sprintf(data,"{\"reqId\":\"%s\"}", uuid_str);

	rc = publish(CLEAR_DIAG_LOG, data);
	if(rc == SUCCESS)
		strcpy(reqId, uuid_str);
}

/**
 * Notifies the IBM Watson IoT Platform response for action
 *
 * @param reqId request Id of the request that is received from the IBM Watson IoT Platform
 *
 * @param state state of the request that is request received from the IBM Watson IoT Platform
 *
 * @return int return code
 *
 */
int changeState(int rc)
{
	char response[100];
	char msg[100] ;
	getMessageFromReturnCode(rc,msg);
	sprintf(response, "{\"rc\":\"%d\",\"message\":\"%s\",\"reqId\":\"%s\"}",rc,msg,currentRequestID);
	int res = publishActionResponse(RESPONSE, response);
	printf("Response for action:%d\n",res);
	return res;
}

/**
 * Update the firmware state while downloading firmware and
 * Notifies the IBM Watson IoT Platform with the updated state
 *
 * @param state Download state update received from the device
 *
 * @return int return code
 *
 */
int changeFirmwareState(int state) {
	char firmwareMsg[300];
	int rc = -1;
	if (dmClient.bObserve) {
		dmClient.DeviceData.mgmt.firmware.state = state;
		sprintf(firmwareMsg,
				"{\"d\":{\"fields\":[{\"field\" : \"mgmt.firmware\",\"value\":{\"state\":%d}}]}}",
				state);
		rc = publishActionResponse(NOTIFY, firmwareMsg);
		printf("Change changeFirmwareState called published with rc=%d\n", rc);
	} else
		printf(
				"Change changeFirmwareState called but the mgmt.firmware is not in observe state\n");
	return rc;
}

/**
 * Update the firmware update state while updating firmware and
 * Notifies the IBM Watson IoT Platform with the updated state
 *
 * @param state Update state received from the device while updating the Firmware
 *
 * @return int return code
 *
 */
int changeFirmwareUpdateState(int state) {
	char firmwareMsg[300];
	int rc = -1;
	if (dmClient.bObserve) {
		dmClient.DeviceData.mgmt.firmware.updateStatus = state;
		sprintf(firmwareMsg,
				"{\"d\":{\"fields\":[{\"field\" : \"mgmt.firmware\",\"value\":{\"state\":%d,\"updateStatus\":%d}}]}}",
				dmClient.DeviceData.mgmt.firmware.state,state);
		rc = publishActionResponse(NOTIFY, firmwareMsg);
		printf("Change changeFirmwareUpdateState called published with rc=%d\n",
				rc);
	} else
		printf(
				"Change changeFirmwareUpdateState called but the mgmt.firmware is not in observe state\n");
	return rc;
}

// Utility function to publish the message to Watson IoT
int publish(char* publishTopic, char* data)
{
	int rc = -1;
	MQTTMessage pub;
	printf("Topic ( %s) payload (%s)\n", publishTopic,data);
	pub.qos = 1;
	pub.retained = '0';
	pub.payload = data;
	pub.payloadlen = strlen(data);
	//signal(SIGINT, sigHandler);
	//signal(SIGTERM, sigHandler);
	interrupt =0;
	while(!interrupt)
		{
			rc = MQTTPublish(&dmClient.deviceClient.c, publishTopic , &pub);
			if(rc == SUCCESS) {
				rc = yield(&dmClient.deviceClient, 100);
			}
			if(!interrupt)
				sleep(2);
		}
	return rc;
}

//Publish actions response to IoTF platform
int publishActionResponse(char* publishTopic, char* data)
{
	int rc = -1;
	MQTTMessage pub;
//	printf("Topic ( %s) payload (%s)\n", publishTopic,data);
	pub.qos = 1;
	pub.retained = '0';
	pub.payload = data;
	pub.payloadlen = strlen(data);

	rc = MQTTPublish(&dmClient.deviceClient.c, publishTopic , &pub);
	if(rc == SUCCESS) {
		rc = yield(&dmClient.deviceClient, 100);
	}
	return rc;
}

//Utility for LocationUpdate Handler
void updateLocationHandler(double latitude, double longitude, double elevation, char* measuredDateTime,char* updatedDateTime, double accuracy)
{
        int rc = -1;
        char data[500];
        sprintf(data,"{\"d\":{\"longitude\":%f,\"latitude\":%f,\"elevation\":%f,\"measuredDateTime\":\"%s\",\"updatedDateTime\":\"%s\",\"accuracy\":%f},\"reqId\":\"%s\"}", latitude, longitude, elevation, updatedDateTime, updatedDateTime, accuracy, currentRequestID);

        rc = publish(UPDATE_LOCATION, data);
}

// Utility function to generate Unique Identifier
void generateUUID(char* uuid_str)
{
	char GUID[40];
	int t = 0;
	char *szTemp = "xxxxxxxx-xxxxy-4xxxx-yxxxy-xxxxxxxxxxxx";
	char *szHex = "0123456789ABCDEF-";
	int nLen = strlen (szTemp);

	for (t=0; t<nLen+1; t++)
	{
	    int r =( rand ())% 16;
	    char c = ' ';

	    switch (szTemp[t])
	    {
	        case 'x' : { c = szHex [r]; } break;
	        case 'y' : { c = szHex [((r & 0x03) | 0x08)]; } break;
	        case '-' : { c = '-'; } break;
	        case '4' : { c = '4'; } break;
	    }

	    GUID[t] = ( t < nLen ) ? c : 0x00;
	}
	strcpy(uuid_str , GUID);
}

// Utility function to get message from the return code
void getMessageFromReturnCode(int rc, char* msg)
{	
	switch(rc)
	{
	case 202:
		strcpy(msg ,"Device action initiated immediately");
		break;
	case 500:
		strcpy(msg ,"Device action attempt fails");
		break;
	case 501:
		strcpy(msg, "Device action is not supported");
		break;
	}	
}

//Handler for all requests and responses from the server. This function routes the
//right handlers
void onMessage(MessageData* md)
{
	//printf("onMessage\n");
	if (md) {
		MQTTMessage* message = md->message;

		char *topic = malloc(md->topicName->lenstring.len + 1);

		sprintf(topic, "%.*s", md->topicName->lenstring.len,
				md->topicName->lenstring.data);
		printf("onMessage topic:%s\n",topic);
		if(!strcmp(topic,DMRESPONSE)){
			messageResponse(md);
		}

		if (!strcmp(topic, dmUpdate)) {
			messageUpdate(md);
		}

		if (!strcmp(topic, dmObserve)) {
			messageObserve(md);
		}

		if (!strcmp(topic, dmCancel)) {
			messageCancel(md);
		}

		if (!strcmp(topic, dmReboot)) {
			messageForAction(md,1);
		}

		if (!strcmp(topic, dmFactoryReset)) {
			messageForAction(md,0);
		}

		if (!strcmp(topic, dmFirmwareDownload)) {
			messageFirmwareDownload(md);
		}

		if (!strcmp(topic, dmFirmwareUpdate)) {
			messageFirmwareUpdate(md);
		}

		free(topic);
	}
}

//Handler for Firmware Download request
void messageFirmwareDownload(MessageData* md)
{
	int rc = RESPONSE_ACCEPTED;
	char msg[100];
	char respmsg[300];

	MQTTMessage* message = md->message;
	void *payload = message->payload;
	cJSON * jsonPayload = cJSON_Parse(payload);
	strcpy(currentRequestID, cJSON_GetObjectItem(jsonPayload, "reqId")->valuestring);
	printf("messageFirmwareDownload with reqId:%s\n",currentRequestID);
	if(dmClient.DeviceData.mgmt.firmware.state != FIRMWARESTATE_IDLE)
	{
		printf("Error: Firmware Download with Bad Request\n");
		rc = BAD_REQUEST;
		strcpy(msg,"Cannot download as the device is not in the idle state");
	}
	else
	{
		printf("Firmware Download Initiated\n");
		rc = RESPONSE_ACCEPTED;
		strcpy(msg,"Firmware Download Initiated");
	}

	sprintf(respmsg,"{\"rc\":%d,\"reqId\":%s}",rc,currentRequestID);
	publishActionResponse(RESPONSE, respmsg);

	if(rc == RESPONSE_ACCEPTED)
		(*cbFirmwareDownload)();
}

//Handler for Firmware update request
void messageFirmwareUpdate(MessageData* md)
{
	int rc;
	char respmsg[300];
	printf("update firmware request called after download\n");
	printf("Firmware State: %d\n", dmClient.DeviceData.mgmt.firmware.state);
	if (dmClient.DeviceData.mgmt.firmware.state != FIRMWARE_DOWNLOADED) {
		printf("Error: Firmware state is not in Downloaded state while updating\n");
		rc = BAD_REQUEST;
	} else {
		rc = RESPONSE_ACCEPTED;
	}
	sprintf(respmsg, "{\"rc\":%d,\"reqId\":%s}", rc, currentRequestID);
	publishActionResponse(RESPONSE, respmsg);
	if(rc == RESPONSE_ACCEPTED)
		(*cbFirmwareUpdate)();
}

//Handler for Observe request
void messageObserve(MessageData* md) {
	int i = 0;
	MQTTMessage* message = md->message;
	void *payload = message->payload;
	char* respMsg;
	cJSON *resPayload, *resd, *resFields;
	resPayload = cJSON_CreateObject();
	cJSON_AddItemToObject(resPayload, "rc",
			cJSON_CreateNumber(RESPONSE_SUCCESS));
	cJSON * jsonPayload = cJSON_Parse(payload);
	cJSON* jreqId = cJSON_GetObjectItem(jsonPayload, "reqId");
	strcpy(currentRequestID, jreqId->valuestring);
	printf("Observe reqId:%s\n", currentRequestID);
	cJSON_AddItemToObject(resPayload, "reqId",
			cJSON_CreateString(currentRequestID));

	cJSON_AddItemToObject(resPayload, "d", resd = cJSON_CreateObject());

	cJSON_AddItemToObject(resd, "fields", resFields =
			cJSON_CreateArray());

	cJSON *d = cJSON_GetObjectItem(jsonPayload, "d");

	cJSON *fields = cJSON_GetObjectItem(d, "fields");

	//cJSON *fields = cJSON_GetObjectItem(jsonPayload,"fields");
	for (i = 0; i < cJSON_GetArraySize(fields); i++) {
		cJSON * field = cJSON_GetArrayItem(fields, i);

		cJSON* fieldName = cJSON_GetObjectItem(field, "field");

		cJSON * value = cJSON_GetArrayItem(fields, i);

		printf("Observe called for fieldName:%s\n",fieldName->valuestring);
		if (!strcmp(fieldName->valuestring, "mgmt.firmware")) {
			dmClient.bObserve = true;
			cJSON* resValue;
			cJSON* resField = cJSON_CreateObject();
			cJSON_AddItemToObject(resField, "field", cJSON_CreateString("mgmt.firmware"));
			cJSON_AddItemToObject(resField, "value", resValue = cJSON_CreateObject());
			cJSON_AddItemToObject(resValue, "state",
					cJSON_CreateNumber(
							dmClient.DeviceData.mgmt.firmware.state));

			cJSON_AddItemToObject(resValue, "updateStatus",
					cJSON_CreateNumber(
							dmClient.DeviceData.mgmt.firmware.updateStatus));

			cJSON_AddItemToArray(resFields,resField);

		}
	}
	respMsg = cJSON_Print(resPayload);
	cJSON_Delete(resPayload);
	//Publish the response to the IoTF
	publishActionResponse(RESPONSE, respMsg);

	cJSON_Delete(jsonPayload);
	free(respMsg);
}

//Handler for cancel observation request
void messageCancel(MessageData* md)
{
	printf("Cancel request called\n");
	int i = 0;
	char respMsg[100];
	MQTTMessage* message = md->message;
	void *payload = message->payload;
	cJSON * jsonPayload = cJSON_Parse(payload);
	cJSON* jreqId = cJSON_GetObjectItem(jsonPayload, "reqId");
	strcpy(currentRequestID, jreqId->valuestring);
	printf("Cancel reqId:%s\n", currentRequestID);
	cJSON *d = cJSON_GetObjectItem(jsonPayload, "d");
	cJSON *fields = cJSON_GetObjectItem(d, "fields");
	//cJSON *fields = cJSON_GetObjectItem(jsonPayload,"fields");
	for (i = 0; i < cJSON_GetArraySize(fields); i++) {
		cJSON * field = cJSON_GetArrayItem(fields, i);
		cJSON* fieldName = cJSON_GetObjectItem(field, "field");

		cJSON * value = cJSON_GetArrayItem(fields, i);
		printf("cancel called for field:%s\n",fieldName->valuestring);
		if (!strcmp(fieldName->valuestring, "mgmt.firmware")) {
			dmClient.bObserve = false;
			sprintf(respMsg,"{\"rc\":%d,\"reqId\":%s}",RESPONSE_SUCCESS,currentRequestID);
			//Publish the response to the IoTF
			publishActionResponse(RESPONSE, respMsg);
		}
	}
}

//Handler for update location request
void updateLocationRequest(cJSON* value)
{
	printf("updateLocationRequest called\n");
	double latitude, longitude, elevation,accuracy;
	char* measuredDateTime;
	char* updatedDateTime;

	latitude = cJSON_GetObjectItem(value,"latitude")->valuedouble;
	longitude = cJSON_GetObjectItem(value,"longitude")->valuedouble;
	elevation = cJSON_GetObjectItem(value,"elevation")->valuedouble;
	accuracy = cJSON_GetObjectItem(value,"accuracy")->valuedouble;
	measuredDateTime = cJSON_GetObjectItem(value,"measuredDateTime")->valuestring;
	updatedDateTime = cJSON_GetObjectItem(value,"updatedDateTime")->valuestring;
	updateLocationHandler(latitude, longitude, elevation,measuredDateTime,updatedDateTime,accuracy);
}

//Handler for update Firmware request
void updateFirmwareRequest(cJSON* value) {
	printf("updateFirmwareRequest called\n");
	char response[100];

	cJSON* val = cJSON_GetObjectItem(value, "version");
	/*if(val)
		printf("version:%s\n",val->valuestring);
	else
		printf("version is NULL\n");
*/
	strcpy(dmClient.DeviceData.mgmt.firmware.version,
			cJSON_GetObjectItem(value, "version")->valuestring);
	//printf("name:%s\n",cJSON_GetObjectItem(value, "name")->valuestring);
	strcpy(dmClient.DeviceData.mgmt.firmware.name,
			cJSON_GetObjectItem(value, "name")->valuestring);
	//printf("uri:%s\n",cJSON_GetObjectItem(value, "uri")->valuestring);
	strcpy(dmClient.DeviceData.mgmt.firmware.url,
			cJSON_GetObjectItem(value, "uri")->valuestring);
	//printf("verifier:%s\n",cJSON_GetObjectItem(value, "verifier")->valuestring);
	strcpy(dmClient.DeviceData.mgmt.firmware.verifier,
			cJSON_GetObjectItem(value, "verifier")->valuestring);
	//printf("state:%d\n",cJSON_GetObjectItem(value, "state")->valueint);
	dmClient.DeviceData.mgmt.firmware.state = cJSON_GetObjectItem(value,
			"state")->valueint;
	//printf("updateStatus:%d\n",cJSON_GetObjectItem(value, "updateStatus")->valueint);
	dmClient.DeviceData.mgmt.firmware.updateStatus = cJSON_GetObjectItem(value,
			"updateStatus")->valueint;
	//printf("updatedDateTime:%s\n",cJSON_GetObjectItem(value, "updatedDateTime")->valuestring);
	strcpy(dmClient.DeviceData.mgmt.firmware.updatedDateTime,
			cJSON_GetObjectItem(value, "updatedDateTime")->valuestring);

	sprintf(response, "{\"rc\":%d,\"reqId\":\"%s\"}", UPDATE_SUCCESS,
			currentRequestID);

	publishActionResponse(RESPONSE, response);
}

//Handler for update request from the server.
//It receives all the update requests like location, mgmt.firmware
//Currently only location and firmware updates are supported.
void messageUpdate(MessageData* md) {
	int i = 0;
	MQTTMessage* message = md->message;
	void *payload = message->payload;
	cJSON * jsonPayload = cJSON_Parse(payload);
	if (jsonPayload) {
		cJSON* jreqId = cJSON_GetObjectItem(jsonPayload, "reqId");
		strcpy(currentRequestID, jreqId->valuestring);
		printf("Update reqId:%s\n", currentRequestID);
		cJSON *d = cJSON_GetObjectItem(jsonPayload, "d");
		cJSON *fields = cJSON_GetObjectItem(d, "fields");

		for (i = 0; i < cJSON_GetArraySize(fields); i++) {
			cJSON * field = cJSON_GetArrayItem(fields, i);
			cJSON* fieldName = cJSON_GetObjectItem(field, "field");
			printf("update request received for fieldName :%s\n", fieldName->valuestring);
			cJSON * value = cJSON_GetObjectItem(field, "value");

			if (!strcmp(fieldName->valuestring, "location"))
				updateLocationRequest(value);
			else if (!strcmp(fieldName->valuestring, "mgmt.firmware"))
				updateFirmwareRequest(value);
			else if (!strcmp(fieldName->valuestring, "metadata"))
				;	//Currently not supported
			else if (!strcmp(fieldName->valuestring, "deviceInfo"))
				;	//Currently not supported
		}
		cJSON_Delete(jsonPayload);//Needs to delete the parsed pointer
	} else
		printf("Error in parsing Json\n");
}

//Handler for responses from the server . Invoke the callback for the response.
//Callback needs to be invoked only if the request Id is matched. While yielding we
//receives the response for old request Ids from the platform. But we are interested only
//with the request Id action was initiated.
void messageResponse(MessageData* md)
{
	fflush(stdout);
	if(cb != 0) {
		MQTTMessage* message = md->message;
		void *payload = message->payload;
		int sz = message->payloadlen;
		printf("Size of payload:%d",sz);
		char *pl = (char*) malloc(sizeof(char)*sz+1);
		strcpy(pl,message->payload);
		char *reqID;
		char *status;

		reqID = strtok(pl, ",");
		status= strtok(NULL, ",");

		reqID = strtok(reqID, ":\"");
		reqID = strtok(NULL, ":\"");
		reqID = strtok(NULL, ":\"");
		
		status= strtok(status, "}");
		status= strtok(status, ":");
		status= strtok(NULL, ":");

		//printf("reqId:%s %d \tcurrent:%s %d\n",reqID, strlen(reqID),currentRequestID, strlen(currentRequestID));
		if(!strcmp(currentRequestID,reqID))
		{
			interrupt = 1;
			(*cb)(status, reqID, payload);
		}
		else
		{
			printf("different response came for ID: %s\n",reqID);
		}
		free(pl);
	}
}

//Handler for Reboot and Factory reset action requests received from the platform.
//Invoke the respective callback for action.
void messageForAction(MessageData* md, bool isReboot)
{
	//printf("messageForAction called\n");
	if(cbReboot != 0 ){
	
		MQTTMessage* message = md->message;

		char *topic = malloc(md->topicName->lenstring.len + 1);

		sprintf(topic, "%.*s", md->topicName->lenstring.len,
				md->topicName->lenstring.data);

		void *payload = message->payload;
		char *pl = (char*) malloc(sizeof(char)*message->payloadlen+1);
		strcpy(pl,message->payload);

		strtok(topic, "/");
		strtok(NULL, "/");

		strtok(NULL, "/");
		strtok(NULL, "/");
		char *action = strtok(NULL, "/");

		char *reqID;

		reqID = strtok(pl, ":\"");
		reqID = strtok(NULL, ":\"");
		reqID = strtok(NULL, ":\"");
		
		strcpy(currentRequestID,reqID);
		printf("reqId: %s action: %s payload: %s\n",reqID, action, (char*)	payload);

		if(isReboot) //If it is reboot
			(*cbReboot)(reqID, action, payload);
		else // Factory reset
			(*cbFactoryReset)(reqID, action, payload);

		free(topic);
		free(pl);
	}
}
