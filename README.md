Embedded C Client Library - Introduction
========================================

Embedded C client for interacting with the Internet of Things
Foundation.

Dependencies
------------

1.  [Embedded C MQTT Client]

  [Embedded C MQTT Client]: http://www.eclipse.org/paho/clients/c/embedded/
  

Embedded C Client Library - Devices
===================================

*iotfclient* is client for the Internet of Things Foundation service.
You can use this client to connect to the service, publish events and
subscribe to commands.

Initialize
----------

There are 2 ways to initialize the *iotfclient*.

### Passing as parameters

The function *initialize* takes the following details to connect to the
IoT Foundation service

-   client - Pointer to the *iotfclient*
-   org - Your organization ID
-   type - The type of your device
-   id - The ID of your device
-   authmethod - Method of authentication (the only value currently
    supported is “token”)
-   authtoken - API key token (required if auth-method is “token”)

``` {.sourceCode .c}
#include "iotfclient.h"
   ....
   ....
   Iotfclient client;
   //quickstart
   rc = initialize(&client,"quickstart","iotsample","001122334455",NULL,NULL);
   //registered
   rc = initialize(&client,"orgid","type","id","token","authtoken");
   ....
```

### Using a configuration file

The function *initialize\_configfile* takes the configuration file path
as a parameter.

``` {.sourceCode .c}
#include "iotfclient.h"
   ....
   ....
   char *filePath = "./device.cfg";
   Iotfclient client;
   rc = initialize_configfile(&client, filePath);
   ....
```

The configuration file must be in the format of

``` {.sourceCode .}
org=$orgId
type=$myDeviceType
id=$myDeviceId
auth-method=token
auth-token=$token
```

Connect
-------

After initializing the *iotfclient*, you can connect to the Internet of
Things Foundation by calling the *connectiotf* function

``` {.sourceCode .c}
#include "iotfclient.h"
   ....
   ....
   Iotfclient client;
   char *configFilePath = "./device.cfg";

   rc = initialize_configfile(&client, configFilePath);

   if(rc != SUCCESS){
       printf("initialize failed and returned rc = %d.\n Quitting..", rc);
       return 0;
   }

   rc = connectiotf(&client);

   if(rc != SUCCESS){
       printf("Connection failed and returned rc = %d.\n Quitting..", rc);
       return 0;
   }
   ....
```

Handling commands
-----------------

When the device client connects, it automatically subscribes to any
command for this device. To process specific commands you need to
register a command callback function by calling the function
*setCommandHandler*. The commands are returned as

-   commandName - name of the command invoked
-   format - e.g json, xml
-   payload

``` {.sourceCode .c}
#include "iotfclient.h"

void myCallback (char* commandName, char* format, void* payload)
{
   printf("The command received :: %s\n", commandName);
   printf("format : %s\n", format);
   printf("Payload is : %s\n", (char *)payload);
}
 ...
 ...
 char *filePath = "./device.cfg";
 rc = connectiotfConfig(filePath);
 setCommandHandler(myCallback);

 yield(1000);
 ....
```

**Note** : *yield* function must be called periodically to receive commands.

Publishing events
------------------

Events can be published by using

-   eventType - Type of event to be published e.g status, gps
-   eventFormat - Format of the event e.g json
-   data - Payload of the event
-   QoS - qos for the publish event. Supported values : QOS0, QOS1, QOS2

``` {.sourceCode .c}
#include "iotfclient.h"
 ....
 rc = connectiotf (org, type, id , authmethod, authtoken);
 char *payload = {\"d\" : {\"temp\" : 34 }};

 rc= publishEvent("status","json", "{\"d\" : {\"temp\" : 34 }}", QOS0); 
 ....
```

Disconnect Client
------------------

Disconnects the client and releases the connections

``` {.sourceCode .c}
#include "iotfclient.h"
 ....
 rc = connectiotf (org, type, id , authmethod, authtoken);
 char *payload = {\"d\" : {\"temp\" : 34 }};

 rc= publishEvent("status","json", payload , QOS0);
 ...
 rc = disconnect();
 ....
```
