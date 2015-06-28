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
 *******************************************************************************/

#include <stdio.h>
#include <signal.h>
#include "iotfclient.h"

volatile int interrupt = 0;

// Handle signal interrupt
void sigHandler(int signo) {
	printf("SigINT received.\n");
	interrupt = 1;
}

void usage()
{
	printf("Usage: helloWorld deviceId\n");
	printf("where,\n deviceId is a 12 digit hex \n");

	exit(-1);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
		usage();

	char* deviceId = argv[1];

	int rc = -1;

	//catch interrupt signal
	signal(SIGINT, sigHandler);
	signal(SIGTERM, sigHandler);

	Iotfclient client;

	// for Quickstart, no need to pass the authmethod and token(last 2 params)
	rc = initialize(&client,"quickstart","iotsample",deviceId,NULL,NULL);

	// For registered mode
	// rc = initialize(&client,"ordid","deviceType","deviceid","token","r+i2I720I+EkZe1c@8");

	if(rc != SUCCESS){
		printf("Initialize returned rc = %d.\n Quitting..\n", rc);
		return 0;
	}

	rc = connectiotf(&client);

	if(rc != SUCCESS){
		printf("Connection returned rc = %d.\n Quitting..\n", rc);
		return 0;
	}

	printf("Connection Successful. Press Ctrl+C to quit\n");
	printf("View the visualization at https://quickstart.internetofthings.ibmcloud.com/#/device/%s\n", deviceId);

	char *data = "{\"d\" : {\"x\" : 26 }}";

	while(!interrupt) 
	{
		printf("Publishing the event stat with rc ");
		rc= publishEvent(&client,"status","json", data , QOS0);
		printf(" %d\n", rc);
		sleep(2);
	}

	printf("Quitting!!\n");

	disconnect(&client);

	return 0;
}