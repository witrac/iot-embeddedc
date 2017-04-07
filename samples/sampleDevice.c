/*******************************************************************************
 * Copyright (c) 2015 IBM Corp.
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

#include "deviceclient.h"

volatile int interrupt = 0;

// Handle signal interrupt
void sigHandler(int signo) {
	printf("SigINT received.\n");
	interrupt = 1;
}

void myCallback (char* commandName, char* format, void* payload)
{
	printf("------------------------------------\n" );
	printf("The command received :: %s\n", commandName);
	printf("format : %s\n", format);
	printf("Payload is : %s\n", (char *)payload);

	printf("------------------------------------\n" );
}

int main(int argc, char const *argv[])
{
	int rc = -1;

	iotfclient  client;

	//catch interrupt signal
	signal(SIGINT, sigHandler);
	signal(SIGTERM, sigHandler);

	char* configFilePath;

	if(isEMBDCHomeDefined()){

	    getSamplesPath(&configFilePath);
	    configFilePath = realloc(configFilePath,strlen(configFilePath)+15);
	    strcat(configFilePath,"device.cfg");
        }
	else{
	    printf("IOT_EMBDC_HOME is not defined\n");
	    printf("Define IOT_EMBDC_HOME to client library path to execute samples\n");
	    return -1;
        }

	rc = initialize_configfile(&client, configFilePath,0);
	free(configFilePath);

	if(rc != SUCCESS){
		printf("initialize failed and returned rc = %d.\n Quitting..", rc);
		return 0;
	}

	//unsigned short interval = 59;
	//setKeepAliveInterval(interval);

	rc = connectiotf(&client);

	if(rc != SUCCESS){
		printf("Connection failed and returned rc = %d.\n Quitting..", rc);
		return 0;
	}

	if(!client.isQuickstart){
	    subscribeCommands(&client);
	    setCommandHandler(&client, myCallback);
        }

	while(!interrupt)
	{
		printf("Publishing the event stat with rc ");
		rc= publishEvent(&client, "status","json", "{\"d\" : {\"temp\" : 34 }}", QOS0);
		printf(" %d\n", rc);
		yield(&client,1000);
		sleep(2);
	}

	printf("Quitting!!\n");

	disconnect(&client);
	return 0;
}
