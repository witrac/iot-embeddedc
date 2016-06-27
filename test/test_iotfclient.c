/* Unit Tests to test source code from iotfclient.c */
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <malloc.h>
#include "iotfclient.h"
#include "testUtils.c"

Iotfclient *pclient;

struct config deviceconfig;

void testInitialize(){
	Iotfclient client;

	char devCfgPath[1024];
	getDeviceCfgFilePath(devCfgPath);

	get_config(devCfgPath, &deviceconfig);

	//int initialize(Iotfclient *client, char *orgId, deviceconfig.domain, char *deviceType, char *deviceId, char *authmethod, char *authToken)
    //orgID , deviceType and deviceId cannot be NULL
	assert_int_equal(initialize(&client, NULL, deviceconfig.domain, deviceconfig.type, deviceconfig.id, deviceconfig.authmethod, deviceconfig.authtoken),MISSING_INPUT_PARAM);
	assert_int_equal(initialize(&client, deviceconfig.org, NULL, deviceconfig.type, deviceconfig.id, deviceconfig.authmethod, deviceconfig.authtoken),MISSING_INPUT_PARAM);
	assert_int_equal(initialize(&client, deviceconfig.org, deviceconfig.domain, NULL, deviceconfig.id, deviceconfig.authmethod, deviceconfig.authtoken),MISSING_INPUT_PARAM);
	assert_int_equal(initialize(&client, deviceconfig.org, deviceconfig.domain, deviceconfig.type, NULL, deviceconfig.authmethod, deviceconfig.authtoken),MISSING_INPUT_PARAM);

	//In registered mode, authmethod and authtoken cannot be NULL
	assert_int_equal(initialize(&client, deviceconfig.org, deviceconfig.domain, deviceconfig.type, deviceconfig.id, NULL, deviceconfig.authtoken),MISSING_INPUT_PARAM);
	assert_int_equal(initialize(&client, deviceconfig.org, deviceconfig.domain, deviceconfig.type, deviceconfig.id, deviceconfig.authmethod, NULL),MISSING_INPUT_PARAM);

	//In quickstart mode, authmethod and authtoken can be NULL
    assert_int_equal(initialize(&client, "quickstart", deviceconfig.domain, deviceconfig.type, deviceconfig.id, NULL, NULL),SUCCESS);

	//Successful Initialization
    assert_int_equal(initialize(&client, deviceconfig.org, deviceconfig.domain, deviceconfig.type, deviceconfig.id, deviceconfig.authmethod, deviceconfig.authtoken),SUCCESS);
}

void testInitializeConfigfile(){
	Iotfclient client;
	char devCfgPath[256];
	getDeviceCfgFilePath(devCfgPath);

	//Invalid Config File
	assert_int_not_equal(initialize_configfile(&client,"dummyConfig.cfg"),SUCCESS);

	//Config File with null values
	//assert_int_equal(initialize_configfile(&client,nullCfgPath),CONFIG_FILE_ERROR);

	//Valid Config File with null values
    assert_int_equal(initialize_configfile(&client,devCfgPath),SUCCESS);
}

void testConnectIotf(){
	Iotfclient client;
	char devCfgPath[256];
	getDeviceCfgFilePath(devCfgPath);

	//Client is not connected yet
	assert_int_not_equal(isConnected(&client),1);

	//Connect in registered mode
	assert_int_equal(initialize_configfile(&client,devCfgPath),SUCCESS);
	assert_int_equal(connectiotf(&client),0);

	//Client is connected
	assert_int_equal(isConnected(&client),1);

	//Connect in quickstart mode
	struct config devconfig = {"", "internetofthings.ibmcloud.com", "", "", "", ""};
	get_config(devCfgPath, &devconfig);	
	assert_int_equal(initialize(&client, "quickstart", devconfig.domain, devconfig.type, devconfig.id, NULL, NULL),SUCCESS);
	assert_int_equal(connectiotf(&client),0);

	//Client is connected
	assert_int_equal(isConnected(&client),1);

	//Disconnect the client
	disconnect(&client);
}

static int setup(){
	int rc=-1;
	pclient = (Iotfclient*)malloc(sizeof(Iotfclient));
	char devCfgPath[1024];
	getDeviceCfgFilePath(devCfgPath);
	rc = initialize_configfile(pclient,devCfgPath);
	rc = connectiotf(pclient);

	return rc;
}

static int teardown(){
	int rc=-1;
    rc=disconnect(pclient);
	return rc;
}

void testPublishEvent(){
	unsigned char *payload = "{ \"d\" : { \"temp\" : 34 }}";

	//Publish an event
	assert_int_equal(publishEvent(pclient,"status","json", payload , QOS0),0);
}

void myCallback (char* commandName, char* format, void* payload)
{
   printf("The command received :: %s\n", commandName);
   printf("format : %s\n", format);
   printf("Payload is : %s\n", (char *)payload);
}

void testMsgReceived(){
	unsigned char *payload = "{ \"d\" : { \"temp\" : 34 }}";

	assert_int_equal(connectiotf(pclient),0);

	setCommandHandler(pclient,myCallback);

	assert_int_equal(publishEvent(pclient,"status","json", payload , QOS0),0);

	//Construct message data for command
	MessageData md;
	int topicLen = 70;
	int loadLen = 30;

	MQTTString topicName;
	topicName.lenstring.data = "iot-2/type/sample/id/first/cmd/command/fmt/json";
	topicName.lenstring.len = topicLen;

	MQTTMessage msg;
	msg.payload = "{\"temp\":\"43\"}";
	msg.payloadlen = loadLen;
	md.message = &msg;
	md.topicName = &topicName;

	messageArrived(&md);

	assert_int_equal(yield(pclient,10),0);

	assert_int_equal(reconnect_delay(1), 3);
	assert_int_equal(reconnect_delay(11), 60);
	assert_int_equal(reconnect_delay(21), 600);
	retry_connection(pclient);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
        cmocka_unit_test(testInitialize),
        cmocka_unit_test(testInitializeConfigfile),
		cmocka_unit_test(testConnectIotf),
		//cmocka_unit_test(testMockConnectIotf),
		cmocka_unit_test_setup_teardown(testPublishEvent,setup,teardown),
		cmocka_unit_test_setup_teardown(testMsgReceived,setup,teardown)
    };

	if(getenv("CTEST_HOME")!=NULL){
	  printf("\n CTEST_HOME set to path %s\n",getenv("CTEST_HOME"));
	  return cmocka_run_group_tests(tests, NULL, NULL);
	}
	else {
	  printf("\n Error while setting CTEST_HOME variable....");
	  return -1;
	}

}
