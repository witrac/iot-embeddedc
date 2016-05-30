/* Unit Tests to test source code from GatewayClient.c */
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <malloc.h>
#include "gatewayclient.h"
#include "testUtils.c"

//GatewayClient gatewayClient;

/* Mock of the connect to IoTF.

extern int connectGateway(GatewayClient *client);
int connectGateway(GatewayClient *client){
	(void)client;
	return mock_type(int);
}

void testMockconnectGateway(){
	GatewayClient client;
	char *devCfgPath="/home/lharalak/EclipseWorkspace/iot-embeddedc/test/device.cfg";

	//Connect in registered mode
	assert_int_equal(initializeGateway_configfile(&client,devCfgPath),SUCCESS);
	will_return(connectGateway,0);
	assert_int_equal(connectGateway(&client),0);

	//Connect in quickstart mode
	assert_int_equal(initialize(&client, "quickstart", "haritestGateway", "hariGateway", NULL, NULL),SUCCESS);
	will_return(connectGateway,0);
	assert_int_equal(connectGateway(&client),0);

}
*/
struct config gatewayConfig;
struct config deviceConfig;

void testInitialize(){
	GatewayClient client;
	char devCfgPath[1024];
	char gatewayCfgPath[1024];
	getDeviceCfgFilePath(devCfgPath);
	getGatewayCfgFilePath(gatewayCfgPath);

	get_config(devCfgPath, &deviceConfig);
	get_config(gatewayCfgPath, &gatewayConfig);

	//orgID , deviceType and deviceId cannot be NULL
	assert_int_equal(
			initializeGateway(&client, NULL, gatewayConfig.type, gatewayConfig.id, gatewayConfig.authmethod, gatewayConfig.authtoken),
			MISSING_INPUT_PARAM);
	assert_int_equal(
			initializeGateway(&client, gatewayConfig.org, NULL, gatewayConfig.id, gatewayConfig.authmethod, gatewayConfig.authtoken),
			MISSING_INPUT_PARAM);
	assert_int_equal(
			initializeGateway(&client, gatewayConfig.org, gatewayConfig.type, NULL, gatewayConfig.authmethod, gatewayConfig.authtoken),
			MISSING_INPUT_PARAM);

	//Successful Initialization
	//assert_int_equal(initializeGateway(&client, "qbtkem", "haritestGateway", "hariGateway", "token", "sZZvqAXeyt5U0-Fnu7"),SUCCESS);
	assert_int_equal(
			initializeGateway(&client, gatewayConfig.org, gatewayConfig.type,
					gatewayConfig.id, gatewayConfig.authmethod,
					gatewayConfig.authtoken), SUCCESS);
}

void testInitializeConfigfile(){
	GatewayClient client;
	char gatewayCfgPath[256];
	getGatewayCfgFilePath(gatewayCfgPath);

	//Invalid Config File
	assert_int_not_equal(initializeGateway_configfile(&client,"dummyConfig.cfg"),SUCCESS);

	//Valid Config File with null values
    assert_int_equal(initializeGateway_configfile(&client,gatewayCfgPath),SUCCESS);
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

void testGateway(){
	GatewayClient client;
	char devCfgPath[256];
	getGatewayCfgFilePath(devCfgPath);

	printf("getGatewayCfgFilePath:%s\n",devCfgPath);
	//Client is not connected yet
	assert_int_not_equal(isGatewayConnected(&client),1);

	//Connect in registered mode
	assert_int_equal(initializeGateway_configfile(&client,devCfgPath),SUCCESS);
	assert_int_equal(connectGateway(&client),0);

	//Client is connected
	assert_int_equal(isGatewayConnected(&client),1);

	unsigned char *payload = "{ \"d\" : { \"temp\" : 34 }}";

	setGatewayCommandHandler(&client, myCallback);
	subscribeToGatewayCommands(&client);
	subscribeToDeviceCommands(&client, deviceConfig.type, deviceConfig.id, "+", "+", 0);

	assert_int_equal(publishGatewayEvent(&client,"status","json", payload , QOS0),0);

	assert_int_equal(publishDeviceEvent(&client, deviceConfig.type, deviceConfig.id,"status","json", "{\"d\" : {\"temp\" : 34 }}", QOS0),0);

	//Construct message data for Reboot
	MessageData md;
	int topicLen = 70;
	int loadLen = 30;

	MQTTString topicName;
	topicName.lenstring.data =
			"iot-2/type/devicetest/id/deviceConfig.id/cmd/command/fmt/json";
	topicName.lenstring.len = topicLen;

	MQTTMessage msg;
	msg.payload = "{\"temp\":\"43\"}";
	msg.payloadlen = loadLen;
	md.message = &msg;
	md.topicName = &topicName;

	messageArrived(&md);

	assert_int_equal(gatewayYield(&client, 10), 0);

	assert_int_equal(reconnect_delay(1), 3);
	assert_int_equal(reconnect_delay(11), 60);
	assert_int_equal(reconnect_delay(21), 600);
	retry_connection(&client);

	//Disconnect the client
	disconnectGateway(&client);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
        cmocka_unit_test(testInitialize),
        cmocka_unit_test(testInitializeConfigfile),
		cmocka_unit_test(testGateway)
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
