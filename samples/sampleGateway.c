/*******************************************************************************
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
 *    Jeffrey Dare - initial implementation and API implementation
 *    Lokesh Haralakatta - Added required changes to use Client Side certificates
 *******************************************************************************/

#include "gatewayclient.h"

volatile int interrupt = 0;

// Handle signal interrupt
void sigHandler(int signo) {
	printf("SigINT received.\n");
	interrupt = 1;
}

//Command Handler
void myCallback (char* type, char* id, char* commandName, char *format, void* payload, size_t payloadlen)
{
	printf("------------------------------------\n" );
	printf("Type is : %s\n", type);
	printf("Id is : %s\n", id);
	printf("The command received :: %s\n", commandName);
	printf("format : %s\n", format);
	printf("Payload is : %.*s\n", (int)payloadlen, (char *)payload);
	printf("------------------------------------\n" );
}

int main(int argc, char const *argv[])
{
	int rc = -1;

	iotfclient  client;

	//catch interrupt signal
	signal(SIGINT, sigHandler);
	signal(SIGTERM, sigHandler);

	char *configFilePath = NULL;

	if(isEMBDCHomeDefined()){

	    getSamplesPath(&configFilePath);
	    configFilePath = realloc(configFilePath,strlen(configFilePath)+15);
	    strcat(configFilePath,"gateway.cfg");
        }
	else{
	    printf("IOT_EMBDC_HOME is not defined\n");
	    printf("Define IOT_EMBDC_HOME to client library path to execute samples\n");
	    return -1;
        }

	rc = initialize_configfile(&client, configFilePath,1);
	free(configFilePath);

	if(rc != SUCCESS){
		printf("initialize failed and returned rc = %d.\n Quitting..", rc);
		return 0;
	}

	setKeepAliveInterval(59);

	rc = connectiotf(&client);

	if(rc != SUCCESS){
		printf("Connection failed and returned rc = %d.\n Quitting..", rc);
		return 0;
	}

	//Registering the function "myCallback" as the command handler.
	setGatewayCommandHandler(&client, myCallback);
	// providing "+" will subscribe to all the command of all formats.
	subscribeToDeviceCommands(&client, "elevator", "elevator-1", "+", "+", 0);

	while(!interrupt)
	{
		printf("Publishing the event stat with rc ");
		//publishing gateway events
		rc= publishGatewayEvent(&client, "elevatorDevices","elevatorGateway", "{\"d\" : {\"temp\" : 34 }}", QOS0);
		//publishing device events on behalf of a device
		rc= publishDeviceEvent(&client, "elevator","elevator-1","status","json", "{\"d\" : {\"temp\" : 34 }}", QOS0);

		printf(" %d\n", rc);
		//Yield for receiving commands.
		yield(&client, 1000);
		sleep(1);
	}

	printf("Quitting!!\n");

	//Be sure to disconnect the gateway at exit
	disconnectGateway(&client);

	return 0;
}
