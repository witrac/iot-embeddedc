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

void usage()
{
	printf("Usage: helloWorld orgID deviceType deviceId token useCerts caCertsPath clientCertPath clientKeyPath\n");
	printf("where:\n");
	printf("orgId: Watson IoT Platform organization Id to connect. \n\n");
	printf("deviceType: Name of Device Type as registered on Watson IoT Platform. \n\n");
	printf("deviceId: Name/ID of Device as registered on Watson IoT Platform. \n\n");
	printf("token: Device token / password provided at the time of registration. \n\n");
	printf("useCerts: Whether to use Client Side Certificates or not. \n\n");
	printf("caCertsPath: If useCerts==1, then valid path containing CA Certificates otherwise not needed. \n\n");
	printf("clientCertPath: If useCerts==1, then valid path containing Client Certificate otherwise not needed. \n\n");
	printf("clientKeyPath: If useCerts==1, then valid path containing Client Private Key otherwise not needed. \n\n");

	exit(-1);
}

int main(int argc, char *argv[])
{
        int useCerts = 0;
	if ( argc < 6 ){
		printf("Sample expects minimum of 5 args.\n");
		usage();
	}
	else{
	      useCerts = argv[5][0] - '0';
     	      if ( useCerts && argc != 9){
		printf("Sample expects extactly 9 args when useCerts = %d.\n",useCerts);
		usage();
	      }
	      else
	      {
		int rc = -1;
		//catch interrupt signal
		signal(SIGINT, sigHandler);
		signal(SIGTERM, sigHandler);
		iotfclient client;

		if(!useCerts)
			rc = initialize(&client,argv[1],"internetofthings.ibmcloud.com",argv[2],
	                 		argv[3],"token",argv[4],NULL,useCerts,NULL,NULL,NULL,0);
		else
			rc = initialize(&client,argv[1],"internetofthings.ibmcloud.com",argv[2],
					argv[3],"token",argv[4],NULL,useCerts,argv[6],argv[7],argv[8],0);

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
      }
}
