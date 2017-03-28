/* Unit Tests to test source code from GatewayClient.c */
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "gatewayclient.h"

//iotfclient  gatewayClient;

/* Mock of the connect to IoTF.

extern int connectGateway(iotfclient  *client);
int connectGateway(iotfclient  *client){
	(void)client;
	return mock_type(int);
}

void testMockconnectGateway(){
	iotfclient  client;
	char *devCfgPath="/home/lharalak/EclipseWorkspace/iot-embeddedc/test/device.cfg";

	//Connect in registered mode
	assert_int_equal(initializeGateway_configfile(&client,devCfgPath),SUCCESS);
	will_return(connectGateway,0);
	assert_int_equal(connectGateway(&client),0);

	//Connect in quickstart mode
	assert_int_equal(initialize(&client, "quickstart", "internetofthings.ibmcloud.com", "haritestGateway", "hariGateway", NULL, NULL),SUCCESS);
	will_return(connectGateway,0);
	assert_int_equal(connectGateway(&client),0);

}
*/

void testInitialize(){
	iotfclient  client;
	Config gatewayConfig;
	char *gatewayCfgPath;

	getTestCfgFilePath(&gatewayCfgPath,"gateway.cfg");

	get_config(gatewayCfgPath, &gatewayConfig);

	//orgID , deviceType and deviceId cannot be NULL
	assert_int_equal(initialize(&client, NULL, gatewayConfig.domain,
	        gatewayConfig.type, gatewayConfig.id, gatewayConfig.authmethod,
	        gatewayConfig.authtoken,NULL,0,NULL,NULL,NULL,1),MISSING_INPUT_PARAM);

	assert_int_equal(initialize(&client, gatewayConfig.org, gatewayConfig.domain, NULL,
		gatewayConfig.id, gatewayConfig.authmethod, gatewayConfig.authtoken,
		NULL,0,NULL,NULL,NULL,1),MISSING_INPUT_PARAM);

	assert_int_equal(initialize(&client, gatewayConfig.org, gatewayConfig.domain,
		gatewayConfig.type, NULL, gatewayConfig.authmethod, gatewayConfig.authtoken,
		NULL,0,NULL,NULL,NULL,1),MISSING_INPUT_PARAM);

	//domain can be NULL
	assert_int_equal(initialize(&client, gatewayConfig.org, NULL, gatewayConfig.type,
		gatewayConfig.id, gatewayConfig.authmethod, gatewayConfig.authtoken,
		NULL,0,NULL,NULL,NULL,1),SUCCESS);


	//auth-token and auth-method cannot be NULL
	assert_int_equal(initialize(&client, gatewayConfig.org, gatewayConfig.domain,
		gatewayConfig.type, gatewayConfig.id, NULL, gatewayConfig.authtoken,
		NULL,0,NULL,NULL,NULL,1),MISSING_INPUT_PARAM);

	assert_int_equal(initialize(&client, gatewayConfig.org, gatewayConfig.domain,
		gatewayConfig.type, gatewayConfig.id, gatewayConfig.authmethod, NULL,
		NULL,0,NULL,NULL,NULL,1),MISSING_INPUT_PARAM);

	//CA Certificate, Client Certificate and Client Key cannot be NULL when useClientCertificates = 1
	assert_int_equal(initialize(&client, gatewayConfig.org, gatewayConfig.domain,
		gatewayConfig.type, gatewayConfig.id, gatewayConfig.authmethod, gatewayConfig.authtoken,
		NULL,1,NULL,"ClientCertPath","ClientKeyPath",1),MISSING_INPUT_PARAM);

	assert_int_equal(initialize(&client, gatewayConfig.org, gatewayConfig.domain,
		gatewayConfig.type, gatewayConfig.id, gatewayConfig.authmethod, gatewayConfig.authtoken,
		NULL,1,"RootCACertPath",NULL,"ClientKeyPath",1),MISSING_INPUT_PARAM);

	assert_int_equal(initialize(&client, gatewayConfig.org, gatewayConfig.domain,
		gatewayConfig.type, gatewayConfig.id, gatewayConfig.authmethod, gatewayConfig.authtoken,
		NULL,1,"RootCACertPath","ClientCertPath",NULL,1),MISSING_INPUT_PARAM);

	//Successful Initialization
	assert_int_equal(initialize(&client, gatewayConfig.org, gatewayConfig.domain, gatewayConfig.type,
		gatewayConfig.id, gatewayConfig.authmethod,gatewayConfig.authtoken, "ServerCertPath", 1,
		"RootCACertPath","ClientCertPath","ClientKeyPath",1), SUCCESS);

	//Free allocated memory
	free(gatewayCfgPath);
}

void testInitializeConfigfile(){
	iotfclient  client;
	char *gatewayCfgPath;
	getTestCfgFilePath(&gatewayCfgPath,"gateway.cfg");

	//Invalid Config File
	assert_int_equal(initialize_configfile(&client,"dummyConfig.cfg",1),CONFIG_FILE_ERROR);

	//Valid Config File
        assert_int_equal(initialize_configfile(&client,gatewayCfgPath,1),SUCCESS);

	//Free allocated memory
	free(gatewayCfgPath);
}

void testGatewayConnectAndPublishWithoutCerts(){
	iotfclient  client;
	char *gatewayCfgPath;

	//Connecting in registered mode with out Client Side Certificates
	getTestCfgFilePath(&gatewayCfgPath,"gateway.cfg");
	assert_int_equal(initialize_configfile(&client,gatewayCfgPath,1),SUCCESS);
	assert_int_equal(connectiotf(&client),0);

	//Gateway Client is connected
	assert_int_equal(isConnected(&client),1);

	//Disconnect the client
	assert_int_equal(disconnectGateway(&client),0);
	free(gatewayCfgPath);
}

void testGatewayConnectAndPublishWithCerts(){
	iotfclient  client;
	char *gatewayCfgPath;

	//Connecting in registered mode with Client Side Certificates should succeed
	getTestCfgFilePath(&gatewayCfgPath,"gateway_with_csc.cfg");
	assert_int_equal(initialize_configfile(&client,gatewayCfgPath,1),SUCCESS);
	assert_int_equal(connectiotf(&client),0);

	//Client is connected
	assert_int_equal(isConnected(&client),1);

	//publishGatewayEvent & publishDeviceEvent
	assert_int_equal(publishGatewayEvent(&client,"status","json", "{\"d\" : {\"temp\" : 34 }}" , QOS0),0);
	assert_int_equal(publishDeviceEvent(&client, "attached", "testGatewayPublish","status","json",
								"{\"d\" : {\"temp\" : 34 }}", QOS0),0);

	//Disconnect the client
	//assert_int_equal(disconnectGateway(&client),0);
	free(gatewayCfgPath);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
        cmocka_unit_test(testInitialize),
        cmocka_unit_test(testInitializeConfigfile),
	cmocka_unit_test(testGatewayConnectAndPublishWithoutCerts),
	cmocka_unit_test(testGatewayConnectAndPublishWithCerts),
    };
    if(isEMBDCHomeDefined()){
      printf("\n IOT_EMBDC_HOME set to path %s\n",getenv("IOT_EMBDC_HOME"));
      return cmocka_run_group_tests(tests, NULL, NULL);
    }
    else {
      printf("\n IOT_EMBDC_HOME Environment Variable not set");
      return -1;
    }
}
