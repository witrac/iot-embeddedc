/*******************************************************************************
 * Copyright (c) 2015, 2016 IBM Corp.
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
 *    Jeffrey Dare            - initial implementation and API implementation
 *    Sathiskumar Palaniappan - Added support to create multiple Iotfclient
 *                              instances within a single process
 *    Lokesh Haralakatta      - Added SSL/TLS support
 *    Lokesh Haralakatta      - Added Client Side Certificates support
 *******************************************************************************/

#include "iotfclient.h"
#include "iotf_utils.h"

//Command Callback
commandCallback cb;

/**
* Function used to Publish events from the device to the IBM Watson IoT service
* @param eventType - Type of event to be published e.g status, gps
* @param eventFormat - Format of the event e.g json
* @param data - Payload of the event
* @param QoS - qos for the publish event. Supported values : QOS0, QOS1, QOS2
*
* @return int return code from the publish
*/

int publishEvent(iotfclient  *client, char *eventType, char *eventFormat, char* data, enum QoS qos)
{
	int rc = -1;

	char publishTopic[strlen(eventType) + strlen(eventFormat) + 16];

	sprintf(publishTopic, "iot-2/evt/%s/fmt/%s", eventType, eventFormat);

	rc = publishData(&(client->c),publishTopic,data,qos);

	if(rc != SUCCESS) {
		printf("connection lost.. \n");
		retry_connection(client);
		rc = publishData(&(client->c),publishTopic,data,qos);
	}

	return rc;

}

/**
* Function used to subscribe to all device commands.
*
* @return int return code
*/
int subscribeCommands(iotfclient  *client)
{
       int rc = -1;

       rc = MQTTSubscribe(&client->c, "iot-2/cmd/+/fmt/+", QOS0, messageArrived);

       return rc;
}

//Handler for all commands. Invoke the callback.
void messageArrived(MessageData* md)
{
	printf("message arrived\n");
	if(cb != 0) {
		MQTTMessage* message = md->message;

		char *topic = malloc(md->topicName->lenstring.len+1);

		sprintf(topic,"%.*s",md->topicName->lenstring.len,md->topicName->lenstring.data);

		void *payload = message->payload;

		strtok(topic, "/");
		strtok(NULL, "/");

		char *commandName = strtok(NULL, "/");
		strtok(NULL, "/");
		char *format = strtok(NULL, "/");


		(*cb)(commandName, format, payload);

		free(topic);

	}
}

/**
* Function used to set the Command Callback function. This must be set if you to recieve commands.
*
* @param cb - A Function pointer to the commandCallback. Its signature - void (*commandCallback)(char* commandName, char* payload)
* @return int return code
*/
void setCommandHandler(iotfclient  *client, commandCallback handler)
{
       cb = handler;
}
