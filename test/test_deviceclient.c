/* Unit Tests to test source code from iotfclient.c */
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "deviceclient.h"

char *devCfgPath;
Config deviceconfig;

void testInitialize(){
	iotfclient  client;

	getTestCfgFilePath(&devCfgPath,"device.cfg");
	get_config(devCfgPath, &deviceconfig);
	free(devCfgPath);

        //orgID , deviceType and deviceId cannot be NULL
	assert_int_equal(initialize(&client, NULL, deviceconfig.domain, deviceconfig.type, deviceconfig.id, deviceconfig.authmethod, deviceconfig.authtoken,NULL,0,NULL,NULL,NULL,0),MISSING_INPUT_PARAM);
	assert_int_equal(initialize(&client, deviceconfig.org, NULL, deviceconfig.type, deviceconfig.id, deviceconfig.authmethod, deviceconfig.authtoken,NULL,0,NULL,NULL,NULL,0),SUCCESS);
	assert_int_equal(initialize(&client, deviceconfig.org, deviceconfig.domain, NULL, deviceconfig.id, deviceconfig.authmethod, deviceconfig.authtoken,NULL,0,NULL,NULL,NULL,0),MISSING_INPUT_PARAM);
	assert_int_equal(initialize(&client, deviceconfig.org, deviceconfig.domain, deviceconfig.type, NULL, deviceconfig.authmethod, deviceconfig.authtoken,NULL,0,NULL,NULL,NULL,0),MISSING_INPUT_PARAM);

	//In registered mode, authmethod and authtoken cannot be NULL
	assert_int_equal(initialize(&client, deviceconfig.org, deviceconfig.domain, deviceconfig.type, deviceconfig.id, NULL, deviceconfig.authtoken,NULL,0,NULL,NULL,NULL,0),MISSING_INPUT_PARAM);
	assert_int_equal(initialize(&client, deviceconfig.org, deviceconfig.domain, deviceconfig.type, deviceconfig.id, deviceconfig.authmethod,NULL,NULL,0,NULL,NULL, NULL,0),MISSING_INPUT_PARAM);

	//In quickstart mode, authmethod and authtoken can be NULL
        assert_int_equal(initialize(&client, "quickstart", deviceconfig.domain, deviceconfig.type, deviceconfig.id, NULL, NULL,NULL,0,NULL,NULL,NULL,0),SUCCESS);

	//RootCACertPath,ClientCertPath and ClientKeyPath can not be NULL when useClientCertificates parameter is 1
	assert_int_equal(initialize(&client, deviceconfig.org, deviceconfig.domain, deviceconfig.type, deviceconfig.id, deviceconfig.authmethod, deviceconfig.authtoken,NULL,1,NULL,"ClientCertPath","ClientKeyPath",0),MISSING_INPUT_PARAM);
	assert_int_equal(initialize(&client, deviceconfig.org, deviceconfig.domain, deviceconfig.type, deviceconfig.id, deviceconfig.authmethod, deviceconfig.authtoken,NULL,1,"RootCACertPath",NULL,"ClientKeyPath",0),MISSING_INPUT_PARAM);
	assert_int_equal(initialize(&client, deviceconfig.org, deviceconfig.domain, deviceconfig.type, deviceconfig.id, deviceconfig.authmethod, deviceconfig.authtoken,NULL,1,"RootCACertPath","ClientCertPath",NULL,0),MISSING_INPUT_PARAM);

	//Server certificate Path can be NULL
	assert_int_equal(initialize(&client, deviceconfig.org, deviceconfig.domain, deviceconfig.type, deviceconfig.id, deviceconfig.authmethod, deviceconfig.authtoken,NULL,0,NULL,NULL,NULL,0),SUCCESS);

	//Successful Initialization
        assert_int_equal(initialize(&client, deviceconfig.org, deviceconfig.domain, deviceconfig.type, deviceconfig.id, deviceconfig.authmethod, deviceconfig.authtoken,"ServerCertPath",1,"RootCACertPath","ClientCertPath","ClientKeyPath",0),SUCCESS);

}

void testInitializeConfigfile(){
	iotfclient  client;

	//Invalid Config File
	assert_int_equal(initialize_configfile(&client,"dummyConfig.cfg",0),CONFIG_FILE_ERROR);

	//Config File with null values
	getTestCfgFilePath(&devCfgPath,"null_values.cfg");
	assert_int_equal(initialize_configfile(&client,devCfgPath,0),MISSING_INPUT_PARAM);
	free(devCfgPath);

	//Valid Config File
	getTestCfgFilePath(&devCfgPath,"device.cfg");
        assert_int_equal(initialize_configfile(&client,devCfgPath,0),SUCCESS);
	free(devCfgPath);
}

void testConnectIotfAndPublishInQSMode(){
	iotfclient  client;

	//Connect in quickstart mode
	getTestCfgFilePath(&devCfgPath,"device.cfg");
	get_config(devCfgPath, &deviceconfig);
	assert_int_equal(initialize(&client, "quickstart", deviceconfig.domain, deviceconfig.type,
	                 deviceconfig.id, NULL, NULL,NULL,0,NULL,NULL,NULL,0),SUCCESS);
	assert_int_equal(connectiotf(&client),0);

	//Client is connected now in quickstart mode
	assert_int_equal(isConnected(&client),1);

	//Disconnect the client
	assert_int_equal(disconnect(&client),0);
}

void testConnectIotfAndPublishWithoutCerts(){
	iotfclient  client;

	//Get config details with SSL
	getTestCfgFilePath(&devCfgPath,"device.cfg");

	//Connecting in registered mode without Client Side Certificates, should succeed
	assert_int_equal(initialize_configfile(&client,devCfgPath,0),SUCCESS);
	assert_int_equal(connectiotf(&client),0);

	//Client is connected now without certificates
	assert_int_equal(isConnected(&client),1);

	//Disconnect the client
	assert_int_equal(disconnect(&client),0);
	free(devCfgPath);

}

void testConnectIotfAndPublishWithCerts(){
	iotfclient  client;

	//Get config details with Client Side Certificates
	getTestCfgFilePath(&devCfgPath,"device_with_csc.cfg");

	//Client is not connected yet
	assert_int_equal(isConnected(&client),0);

	//Connecting in registered mode using Client Side Certificates should succeed
	assert_int_equal(initialize_configfile(&client,devCfgPath,0),SUCCESS);
	assert_int_equal(connectiotf(&client),0);

	//Client is connected now using Client Side Certificates
	assert_int_equal(isConnected(&client),1);

	//Publish an event
	assert_int_equal(publishEvent(&client,"status","json", "{\"d\" : {\"temp\" : 34 }}" , QOS0),0);

	//Disconnect the client
	assert_int_equal(disconnect(&client),0);
	free(devCfgPath);

}

int main(void)
{
	const struct CMUnitTest tests[] = {
        cmocka_unit_test(testInitialize),
        cmocka_unit_test(testInitializeConfigfile),
	cmocka_unit_test(testConnectIotfAndPublishInQSMode),
	cmocka_unit_test(testConnectIotfAndPublishWithCerts),
	cmocka_unit_test(testConnectIotfAndPublishWithoutCerts),
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
