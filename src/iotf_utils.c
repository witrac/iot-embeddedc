/*******************************************************************************
 * Copyright (c) 2017 IBM Corp.
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
 *    Lokesh Haralakatta  -  Initial implementation
 *                        -  Contains Abstract Functions Definitions
 *******************************************************************************/

 //Include iotf_utils.h
 #include "iotf_utils.h"

 unsigned short keepAliveInterval = 60;

 /**
 * Function used to initialize the IBM Watson IoT client using the config file which is
 * generated when you register your device.
 * @param configFilePath - File path to the configuration file
 * @Param isGatewayClient - 0 for device client or 1 for gateway client
 *
 * @return int return code
 * error codes
 * CONFIG_FILE_ERROR -3 - Config file not present or not in right format
 */
 int initialize_configfile(iotfclient  *client, char *configFilePath, int isGatewayClient)
 {
 	Config configstr = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 1883,0};

 	int rc = 0;

 	rc = get_config(configFilePath, &configstr);

 	if(rc != SUCCESS) {
 		return rc;
 	}

 	if(configstr.org == NULL || configstr.type == NULL || configstr.id == NULL ||
 	   configstr.authmethod == NULL || configstr.authtoken == NULL) {
 		freeConfig(&configstr);
 		return MISSING_INPUT_PARAM;
 	}

 	if(configstr.useClientCertificates)
 		if(configstr.rootCACertPath == NULL || configstr.clientCertPath == NULL ||
 		   configstr.clientKeyPath == NULL){
 		   	freeConfig(&configstr);
 			return MISSING_INPUT_PARAM;
 		}

        if(isGatewayClient){
                if((strcmp(configstr.org,"quickstart") == 0)) {
        	        printf("Quickstart mode is not supported in Gateway Client\n");
                        freeConfig(&configstr);
        	        return QUICKSTART_NOT_SUPPORTED;
                }
                client->isGateway = 1;
        }
        else
                client->isGateway = 0;

 	client->cfg = configstr;

 	return rc;

 }

 /**
 * Function used to initialize the Watson IoT client
 * @param client - Reference to the Iotfclient
 * @param org - Your organization ID
 * @param domain - Your domain Name
 * @param type - The type of your device
 * @param id - The ID of your device
 * @param auth-method - Method of authentication (the only value currently supported is â€œtokenâ€�)
 * @param auth-token - API key token (required if auth-method is â€œtokenâ€�)
 * @Param serverCertPath - Custom Server Certificate Path
 * @Param useCerts - Flag to indicate whether to use client side certificates for authentication
 * @Param rootCAPath - if useCerts is 1, Root CA certificate Path
 * @Param clientCertPath - if useCerts is 1, Client Certificate Path
 * @Param clientKeyPath - if useCerts is 1, Client Key Path
 *
 * @return int return code
 */
 int initialize(iotfclient  *client, char *orgId, char* domainName, char *deviceType,
 	       char *deviceId, char *authmethod, char *authToken, char *serverCertPath, int useCerts,
                char *rootCACertPath, char *clientCertPath,char *clientKeyPath, int isGatewayClient)
 {

 	Config configstr = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 1883,0};

 	if(orgId == NULL || deviceType == NULL || deviceId == NULL) {
 		return MISSING_INPUT_PARAM;
 	}

 	if(useCerts){
 		if(rootCACertPath == NULL || clientCertPath == NULL || clientKeyPath == NULL)
 			return MISSING_INPUT_PARAM;
 	}

        if(isGatewayClient){
                if((strcmp(orgId,"quickstart") == 0)) {
        	        printf("Quickstart mode is not supported in Gateway Client\n");
                        freeConfig(&configstr);
        	        return QUICKSTART_NOT_SUPPORTED;
                }
                client->isGateway = 1;
        }

 	strCopy(&configstr.org, orgId);
 	strCopy(&configstr.domain,"internetofthings.ibmcloud.com");
 	if(domainName != NULL)
 		strCopy(&configstr.domain, domainName);
 	strCopy(&configstr.type, deviceType);
 	strCopy(&configstr.id, deviceId);

 	if((strcmp(orgId,"quickstart") != 0)) {
 		if(authmethod == NULL || authToken == NULL) {
                        freeConfig(&configstr);
 			return MISSING_INPUT_PARAM;
 		}
 		strCopy(&configstr.authmethod, authmethod);
 		strCopy(&configstr.authtoken, authToken);

 		if(serverCertPath == NULL)
 			if(isEMBDCHomeDefined())
 				getServerCertPath(&configstr.serverCertPath);
 			else
 				strCopy(&configstr.serverCertPath,"./IoTFoundation.pem");
 		else
 			strCopy(&configstr.serverCertPath,serverCertPath);

 		if(useCerts){
 			strCopy(&configstr.rootCACertPath,rootCACertPath);
 			strCopy(&configstr.clientCertPath,clientCertPath);
 			strCopy(&configstr.clientKeyPath,clientKeyPath);
 			configstr.useClientCertificates = 1;
 		}
 	}

 	client->cfg = configstr;

 	return SUCCESS;
 }

 // This is the function to read the config from the device.cfg file
 int get_config(char * filename, Config * configstr) {

        FILE* prop;
        prop = fopen(filename, "r");
        if (prop == NULL) {
 	       printf("Config file not found at %s\n",filename);
 	       return -3;
        }
        char line[256];
        int linenum = 0;
        while (fgets(line, 256, prop) != NULL) {
 	       char* prop;
 	       char* value;

 	       linenum++;
 	       if (line[0] == '#')
 		       continue;

 	       prop = strtok(line, "=");
 	       prop = trim(prop);
 	       value = strtok(NULL, "=");
 	       value = trim(value);
               if (strcmp(prop, "org") == 0){
 		   if(strlen(value) > 1)
 		     strCopy(&(configstr->org), value);
        	   if(strcmp(configstr->org,"quickstart") !=0)
 		      configstr->port = 8883;
 		}
 	       else if (strcmp(prop, "domain") == 0){
 	           if(strlen(value) <= 1)
 		      strCopy(&configstr->domain,"internetofthings.ibmcloud.com");
 		   else
 		      strCopy(&configstr->domain, value);
 		}
 	       else if (strcmp(prop, "type") == 0){
 		   if(strlen(value) > 1)
 		     strCopy(&configstr->type, value);
 		}
 	       else if (strcmp(prop, "id") == 0){
 		   if(strlen(value) > 1)
 		     strCopy(&configstr->id, value);
 		}
 	       else if (strcmp(prop, "auth-token") == 0){
 		   if(strlen(value) > 1)
 		     strCopy(&configstr->authtoken, value);
 		}
 	       else if (strcmp(prop, "auth-method") == 0){
 		   if(strlen(value) > 1)
 		     strCopy(&configstr->authmethod, value);
 		}
 	       else if (strcmp(prop, "serverCertPath") == 0){
 	       	   if(strlen(value) <= 1)
 		      getServerCertPath(&configstr->serverCertPath);
 		   else
 		      strCopy(&configstr->serverCertPath, value);
 		}
 	       else if (strcmp(prop, "rootCACertPath") == 0){
 		   if(strlen(value) > 1)
 		     strCopy(&configstr->rootCACertPath, value);
 		}
 	       else if (strcmp(prop, "clientCertPath") == 0){
 		   if(strlen(value) > 1)
 		     strCopy(&configstr->clientCertPath, value);
 		}
 	       else if (strcmp(prop, "clientKeyPath") == 0){
 		   if(strlen(value) > 1)
 		     strCopy(&configstr->clientKeyPath, value);
 		}
 	       else if (strcmp(prop,"useClientCertificates") == 0){
 		   configstr->useClientCertificates = value[0] - '0';
 		}
        }

        return 0;
 }

 /**
 * Function used to connect to the IBM Watson IoT client
 * @param client - Reference to the Iotfclient
 *
 * @return int return code
 */
 int connectiotf(iotfclient  *client)
 {

 	int rc = 0;
 	int useCerts = client->cfg.useClientCertificates;
 	client->isQuickstart = 0;
 	if(strcmp(client->cfg.org,"quickstart") == 0){
 		client->isQuickstart = 1 ;
 	}

 	char messagingUrl[120];
 	sprintf(messagingUrl, ".messaging.%s",client->cfg.domain);
 	char hostname[strlen(client->cfg.org) + strlen(messagingUrl) + 1];
        sprintf(hostname, "%s%s", client->cfg.org, messagingUrl);
        char clientId[strlen(client->cfg.org) + strlen(client->cfg.type) + strlen(client->cfg.id) + 5];

        if(client->isGateway)
           sprintf(clientId, "g:%s:%s:%s", client->cfg.org, client->cfg.type, client->cfg.id);
        else
     	   sprintf(clientId, "d:%s:%s:%s", client->cfg.org, client->cfg.type, client->cfg.id);

 	NewNetwork(&client->n);

 	if(!client->isGateway && client->isQuickstart){
 	    if(ConnectNetwork(&(client->n),hostname,client->cfg.port) != 0){
 		printf("ConnectNetwork failed for QuickStart Mode, exitting\n");
 		freeConfig(&client->cfg);
 		freeTLSConnectData(&(client->n.TLSConnectData));
 		return -1;
 	    }
 	}
 	else {
 	    tls_connect_params tls_params = {NULL,NULL,NULL,NULL,NULL};
 	    strCopy(&tls_params.pServerCertLocation,client->cfg.serverCertPath);
 	    if(useCerts){
 		strCopy(&tls_params.pRootCACertLocation,client->cfg.rootCACertPath);
 		strCopy(&tls_params.pDeviceCertLocation,client->cfg.clientCertPath);
 		strCopy(&tls_params.pDevicePrivateKeyLocation,client->cfg.clientKeyPath);
 		strCopy(&tls_params.pDestinationURL,hostname);
 	    }
 	    client->n.TLSConnectData = tls_params;
 	    if(tls_connect(&(client->n.TLSInitData),&(client->n.TLSConnectData),hostname,client->cfg.port,useCerts)!=0)
 	    {
 		printf("SSL/TLS Connect Failed, exiting\n");
 		freeConfig(&client->cfg);
 		freeTLSConnectData(&(client->n.TLSConnectData));
 		return -1;
 	    }
 	    client->n.my_socket = client->n.TLSInitData.server_fd.fd;
 	    client->n.mqttwrite = tls_write;
 	    client->n.mqttread = tls_read;
         }

 	MQTTClient(&client->c, &client->n, 1000, client->buf, BUFFER_SIZE, client->readbuf, BUFFER_SIZE);

 	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
 	data.willFlag = 0;
 	data.MQTTVersion = 3;
 	data.clientID.cstring = clientId;

 	if(!client->isQuickstart) {
 		data.username.cstring = "use-token-auth";
 		data.password.cstring = client->cfg.authtoken;
 	}

 	data.keepAliveInterval = keepAliveInterval;
 	data.cleansession = 1;

 	if((rc = MQTTConnect(&client->c, &data))==0){
 	    if(client->isQuickstart)
 	       printf("Device Client Connected to %s Platform in QuickStart Mode\n",hostname);
 	    else
            {
                char *clientType = (client->isGateway)?"Gateway Client":"Device Client";
                char *connType = (useCerts)?"Client Side Certificates":"Secure Connection";
 	        printf("%s Connected to %s in registered mode using %s\n",clientType,hostname,connType);
 	    }
 	}

 	return rc;
 }

 /**
 * Function used to publish the given data to the topic with the given QoS
 * @Param client - Address of MQTT Client
 * @Param topic - Topic to publish
 * @Param payload - Message payload
 * @Param qos - quality of service either of 0,1,2
 *
 * @return int - Return code from MQTT Publish
 **/
 int publishData(Client *mqttClient, char *topic, char *payload, int qos){
        int rc = -1;
        MQTTMessage pub;

 	pub.qos = qos;
 	pub.retained = '0';
 	pub.payload = payload;
 	pub.payloadlen = strlen(payload);

 	rc = MQTTPublish(mqttClient, topic , &pub);

        return(rc);
 }

 /**
 * Function used to Yield for commands.
 * @param time_ms - Time in milliseconds
 * @return int return code
 */
 int yield(iotfclient  *client, int time_ms)
 {
 	int rc = 0;
 	rc = MQTTYield(&client->c, time_ms);
 	return rc;
 }

 /**
 * Function used to check if the client is connected
 *
 * @return int return code
 */
 int isConnected(iotfclient  *client)
 {
 	return client->c.isconnected;
 }

 /**
 * Function used to disconnect from the IBM Watson IoT service
 *
 * @return int return code
 */

 int disconnect(iotfclient  *client)
 {
 	int rc = 0;
 	if(isConnected(client))
 	   rc = MQTTDisconnect(&client->c);
 	client->n.disconnect(&(client->n),client->isQuickstart);
 	freeConfig(&(client->cfg));
 	return rc;

 }

 /**
 * Function used to set the time to keep the connection alive with IBM Watson IoT service
 * @param keepAlive - time in secs
 *
 */
 void setKeepAliveInterval(unsigned int keepAlive)
 {
 	keepAliveInterval = keepAlive;

 }

 //Staggered retry
 int retry_connection(iotfclient  *client)
 {
 	int retry = 1;
 	int rc = -1;
 	printf("Attempting to connect\n");

 	while((rc = connectiotf(client)) != SUCCESS)
 	{
 		printf("Retry Attempt #%d ", retry);
 		int delay = reconnect_delay(retry++);
 		printf(" next attempt in %d seconds\n", delay);
 		sleep(delay);
 	}
 	return rc;
 }

 /** Function to check whether environment variable IOT_EMBDC_HOME defined.
 * @param - None
 * @return - 1 if IOT_EMBDC_HOME is defined
 *         - 0 if IOT_EMBDC_HOME is not defined
 **/
 int isEMBDCHomeDefined(){
	if(getenv("IOT_EMBDC_HOME")!= NULL)
	   return 1;
	else
	   return 0;
 }

 /** Function to store server certificate path using environment variable IOT_EMBDC_HOME.
 * @param - Address of character pointer to store the server certificate path
 * @return - void
 **/
 void getServerCertPath(char** path){
	char *embdC_home = getenv("IOT_EMBDC_HOME");
	char *serverCert = "/IoTFoundation.pem";
        int pathLen = strlen(embdC_home) + strlen(serverCert);
        *path = (char*)malloc(sizeof(char)*(pathLen+1));
        strcpy(*path,embdC_home);
        strcat(*path,serverCert);
        (*path)[pathLen]='\0';
 }

 /** Function to store samples path using environment variable IOT_EMBDC_HOME.
 * @param - Address of character pointer to store the samples path
 * @return - void
 **/
 void getSamplesPath(char** path){
	char *embdC_home = getenv("IOT_EMBDC_HOME");
	char *samplesDirName = "/samples";
        int pathLen = strlen(embdC_home) + strlen(samplesDirName);
        *path = (char*)malloc(sizeof(char)*(pathLen+1));
        strcpy(*path,embdC_home);
        strcat(*path,samplesDirName);
        (*path)[pathLen]='\0';
 }

 /** Function to store proper path for config file using environment variable IOT_EMBDC_HOME.
 * @param - Character string to store the path
 *        - Config file name to be appended to path
 * @return - void
 **/
 void getTestCfgFilePath(char** cfgFilePath, char* fileName){
        char *embdC_home = getenv("IOT_EMBDC_HOME");
        char *testDir = "/test/";
        int pathLen = strlen(embdC_home) + strlen(testDir) + strlen(fileName);
        *cfgFilePath = (char*)malloc(sizeof(char)*(pathLen+1));
        strcpy(*cfgFilePath,embdC_home);
        strcat(*cfgFilePath,testDir);
        strcat(*cfgFilePath,fileName);
        (*cfgFilePath)[pathLen]='\0';
 }

//Trimming characters
 char *trim(char *str) {
 	size_t len = 0;
 	char *frontp = str - 1;
 	char *endp = NULL;

 	if (str == NULL)
 		return NULL;

 	if (str[0] == '\0')
 		return str;

 	len = strlen(str);
 	endp = str + len;

 	while (isspace(*(++frontp)))
 		;
 	while (isspace(*(--endp)) && endp != frontp)
 		;

 	if (str + len - 1 != endp)
 		*(endp + 1) = '\0';
 	else if (frontp != str && endp == frontp)
 		*str = '\0';

 	endp = str;
 	if (frontp != str) {
 		while (*frontp)
 			*endp++ = *frontp++;
 		*endp = '\0';
 	}

 	return str;
 }

 /** Function to copy source to destination string after allocating required memory.
 * @param - Address of character pointer as destination
 *        - Contents of source string
 * @return - void
 **/
 void strCopy(char **dest, char *src){
     if(strlen(src) >= 1){
        *dest = (char*)malloc(sizeof(char)*(strlen(src)+1));
        strcpy(*dest,src);
     }
 }

 /** Function to free the allocated memory for character string.
 * @param - Character pointer pointing to allocated memory
 * @return - void
 **/
 void freePtr(char* p){
        if(p != NULL)
           free(p);
 }

 /* Reconnect delay time
  * depends on the number of failed attempts
  */
 int reconnect_delay(int i)
 {
 	if (i < 10) {
 		return 3; // first 10 attempts try every 3 seconds
 	}
 	if (i < 20)
 		return 60; // next 10 attempts retry after every 1 minute

 	return 600;	// after 20 attempts, retry every 10 minutes
 }

 void freeConfig(Config *cfg){
 	freePtr(cfg->org);
 	freePtr(cfg->domain);
 	freePtr(cfg->type);
 	freePtr(cfg->id);
 	freePtr(cfg->authmethod);
 	freePtr(cfg->authtoken);
 	freePtr(cfg->rootCACertPath);
 	freePtr(cfg->clientCertPath);
 	freePtr(cfg->clientKeyPath);
 }
