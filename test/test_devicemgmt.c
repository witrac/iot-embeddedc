/*
 * test_devicemgmt.c
 *
 *  Created on: 03-May-2016
 *      Author: HariPrasadReddy
 */
/* Unit Tests to test source code of devicemanagementclient.c */
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "devicemanagementclient.h"

//Iotfclient *pclient;
void populateMgmtConfig();


void testInitialize(){
	Config deviceconfig;
	char *devCfgPath;

	getTestCfgFilePath(&devCfgPath,"device.cfg");
	get_config(devCfgPath, &deviceconfig);
	free(devCfgPath);


        //orgID , deviceType and deviceId cannot be NULL
	assert_int_equal(initialize_dm( NULL, deviceconfig.domain, deviceconfig.type, deviceconfig.id,
		deviceconfig.authmethod, deviceconfig.authtoken,NULL,0,NULL,NULL,NULL),MISSING_INPUT_PARAM);

	assert_int_equal(initialize_dm( deviceconfig.org, deviceconfig.domain, NULL, deviceconfig.id,
		deviceconfig.authmethod, deviceconfig.authtoken,NULL,0,NULL,NULL,NULL),MISSING_INPUT_PARAM);

	assert_int_equal(initialize_dm( deviceconfig.org, deviceconfig.domain, deviceconfig.type, NULL,
		deviceconfig.authmethod, deviceconfig.authtoken,NULL,0,NULL,NULL,NULL),MISSING_INPUT_PARAM);

	//Domain can be NULL
	assert_int_equal(initialize_dm(deviceconfig.org, NULL, deviceconfig.type, deviceconfig.id,
		deviceconfig.authmethod, deviceconfig.authtoken,NULL,0,NULL,NULL,NULL),SUCCESS);


	//In registered mode, authmethod and authtoken cannot be NULL
	assert_int_equal(initialize_dm( deviceconfig.org, deviceconfig.domain, deviceconfig.type,
		deviceconfig.id, NULL, deviceconfig.authtoken,NULL,0,NULL,NULL,NULL),MISSING_INPUT_PARAM);

	assert_int_equal(initialize_dm( deviceconfig.org, deviceconfig.domain, deviceconfig.type,
		deviceconfig.id, deviceconfig.authmethod, NULL,NULL,0,NULL,NULL,NULL),MISSING_INPUT_PARAM);

	//In quickstart mode, authmethod and authtoken can be NULL
	assert_int_equal(initialize_dm( "quickstart", deviceconfig.domain, deviceconfig.type, deviceconfig.id,
					NULL, NULL,NULL,0,NULL,NULL,NULL),SUCCESS);

	//RootCACertPath,ClientCertPath and ClientKeyPath can not be NULL when useClientCertificates = 1
	assert_int_equal(initialize_dm( deviceconfig.org, deviceconfig.domain, deviceconfig.type,
		                        deviceconfig.id, deviceconfig.authmethod, deviceconfig.authtoken,
					NULL,1,NULL,"ClientCertPath","ClientKeyPath"), MISSING_INPUT_PARAM);

	assert_int_equal(initialize_dm( deviceconfig.org, deviceconfig.domain, deviceconfig.type,
					deviceconfig.id, deviceconfig.authmethod, deviceconfig.authtoken,
					NULL,1,"RootCACertPath",NULL,"ClientKeyPath"), MISSING_INPUT_PARAM);

	assert_int_equal(initialize_dm( deviceconfig.org, deviceconfig.domain, deviceconfig.type,
					deviceconfig.id, deviceconfig.authmethod, deviceconfig.authtoken,
					NULL,1,"RootCACertPath","ClientCertPath",NULL), MISSING_INPUT_PARAM);
	//Successful Initialization
	assert_int_equal(initialize_dm( deviceconfig.org, deviceconfig.domain, deviceconfig.type,
					deviceconfig.id, deviceconfig.authmethod, deviceconfig.authtoken,
					"serverCertPath",1,"RootCACertPath","ClientCertPath","ClientKeyPath"),
					SUCCESS);
	//Release the memory allocated for config
	freeConfig(&(dmClient.deviceClient.cfg));
}

void testInitializeConfigfile(){
	ManagedDevice client;
	char *devCfgPath;
	getTestCfgFilePath(&devCfgPath,"device.cfg");

	//Invalid Config File
	assert_int_not_equal(initialize_configfile_dm("dummyConfig.cfg"),SUCCESS);

	//Config File with null values
	//assert_int_equal(initialize_configfile(nullCfgPath),CONFIG_FILE_ERROR);

	//Valid Config File with null values
    	assert_int_equal(initialize_configfile_dm(devCfgPath),SUCCESS);
	freeConfig(&(dmClient.deviceClient.cfg));
	free(devCfgPath);
}

void testConnectIotf(){
	char *devCfgPath;
	getTestCfgFilePath(&devCfgPath,"device_with_csc.cfg");

	//Client is not connected yet
	assert_int_not_equal(isConnected_dm(),1);

	//Connect in registered mode
	assert_int_equal(initialize_configfile_dm(devCfgPath),SUCCESS);
	assert_int_equal(connectiotf_dm(),0);

	//Client is connected
	assert_int_equal(isConnected_dm(),1);

	//Disconnect the client
	disconnect_dm();

	//Connect in quickstart mode
	assert_int_equal(initialize_dm("quickstart", "internetofthings.ibmcloud.com", "sample", "first",
					NULL, NULL, NULL, 0, NULL, NULL, NULL),SUCCESS);
	assert_int_equal(connectiotf_dm(),0);

	//Client is connected
	assert_int_equal(isConnected_dm(),1);

	//Disconnect the client
	disconnect_dm();

}

static int setup(){
	int rc=-1;
	char *devCfgPath;
	getTestCfgFilePath(&devCfgPath,"device_with_csc.cfg");
	rc = initialize_configfile_dm(devCfgPath);
	free(devCfgPath);
	return rc;
}

static int teardown(){
	int rc=-1;
    	rc=disconnect_dm();
	return rc;
}

void testPublishEvent(){
	unsigned char *payload = "{ \"d\" : { \"temp\" : 34 }}";

	//Publish an event
	assert_int_equal(publishEvent_dm("status","json", payload , QOS0),0);
}

void myCallback (char* commandName, char* format, void* payload)
{
   printf("The command received :: %s\n", commandName);
   printf("format : %s\n", format);
   printf("Payload is : %s\n", (char *)payload);
}

void managedCallBack (char* Status, char* requestId, void* payload)
{
	printf("\n------------------------------------\n" );
	printf("Status :: %s\n", Status);
	printf("requestId : %s\n", requestId);
	printf("Payload is : %s\n", (char *)payload);

	printf("------------------------------------\n" );
}

void rebootCallBack (char* reqID, char* action, void* payload)
{
	printf("\n-----------REBOOT-------------------------\n" );
	printf("request Id :: %s\n", reqID);
	printf("action : %s\n", action);
	printf("Payload is : %s\n", (char *)payload);

	int rc = changeState(REBOOT_INITIATED);
	//Reboot custom code needs to be added based on the platform the application is running
	//After Rebooting the device Manage request needs to be sent to the platform to successfully complete the action
	//So this program needs to be kept in the bashrc so that once the system reboots Manage event will be sent and the action will be successful.
	//#if defined(_WIN32) || defined(_WIN64)
        //	system("C:\\WINDOWS\\System32\\shutdown -r");
	//#else
	//	system("sudo shutdown -r now");
	//#endif
	printf("------------------------------------\n" );
}


void factoryResetCallBack (char* reqID, char* action, void* payload)
{
	printf("\n--------------FACTORYRESET----------------------\n" );
	printf("request Id: %s\n", reqID);
	printf("action : %s\n", action);
	printf("Payload is : %s\n", (char *)payload);

	// This sample doesn't support factory reset, so respond accordingly
	int rc = changeState(FACTORYRESET_NOTSUPPORTED);
	printf("Factory reset is not supported in this sample\n" );
	printf("------------------------------------\n" );
}

void firmwareDownloadCallBack()
{
	printf("\n--------------Firmware Download----------------------\n" );
	//Add the code for downloading the firmware
	changeFirmwareState(FIRMWARESTATE_DOWNLOADING);
	sleep(5);
	changeFirmwareState(FIRMWARE_DOWNLOADED);
}

void firmwareUpdateCallBack()
 {
	printf("\n--------------Firmware Update----------------------\n");
	//Add the code for updating the firmware
	changeFirmwareUpdateState(FIRMWAREUPDATE_INPROGRESS);
	sleep(5);
	changeFirmwareUpdateState(FIRMWAREUPDATE_SUCCESS);
	sleep(5);
	changeFirmwareState(FIRMWARESTATE_IDLE);
}


void testManage(){
	unsigned char *payload = "{ \"d\" : { \"temp\" : 34 }}";

	assert_int_equal(connectiotf_dm(),0);

	setCommandHandler_dm(myCallback);
	setManagedHandler_dm(managedCallBack );
	subscribeCommands_dm();
	populateMgmtConfig();

	char reqId[40] = {0};
	char test[40] = {0};
	assert_string_equal(reqId, test);
	publishManageEvent(4000,1,1, reqId);
	assert_string_not_equal(reqId, test);

	strcpy(reqId, test);
	addErrorCode(121, reqId);
	assert_string_not_equal(reqId, test);

	strcpy(reqId, test);
	clearErrorCodes(reqId);
	assert_string_not_equal(reqId, test);

	//strcpy(reqId, test);
	//addLog("test","",1, reqId);
	//assert_string_not_equal(reqId, test);

	strcpy(reqId, test);
	clearLogs(reqId);
	assert_string_not_equal(reqId, test);

	strcpy(reqId, test);
	time_t t = time(NULL);
	time(&t);
	struct tm* tt = localtime(&t);
	char updatedDateTime[80];	//"2016-03-01T07:07:56.323Z"
	strftime(updatedDateTime, 80, "%Y-%m-%dT%TZ",&tt);
	updateLocation(77.5667, 12.9667, 0, updatedDateTime, 0, reqId);
	assert_string_not_equal(reqId, test);

	strcpy(reqId, test);
	publishUnManageEvent(reqId);
	assert_string_not_equal(reqId, test);


	assert_int_equal(yield_dm(10),0);
}

void testDeviceActions(){

	assert_int_equal(connectiotf_dm(),0);

	setCommandHandler_dm(myCallback);
	setManagedHandler_dm(managedCallBack);
	setRebootHandler(rebootCallBack);
	setFactoryResetHandler(factoryResetCallBack);
	subscribeCommands_dm();
	populateMgmtConfig();

	char reqId[40] = {0};
	char test[40] = {0};

	publishManageEvent(4000, 1, 1, reqId);
	assert_string_not_equal(reqId, test);

	char action[15];
	char payload[100];

	strcpy(reqId, "9c982454-3b44-4160-b577-cd6df67be560");
	strcpy(action,"reboot");

	strcpy(payload,"{\"reqId\":\"9c982454-3b44-4160-b577-cd6df67be560\"}");

	//Construct message data for Reboot
	MessageData md;
	int topicLen = 50;
	int loadLen = 100;

	MQTTString topicName;
	topicName.lenstring.data = "iotdm-1/mgmt/initiate/device/reboot";
	topicName.lenstring.len = topicLen;

	MQTTMessage msg;
	msg.payload = "{\"reqId\":\"9c982454-3b44-4160-b577-cd6df67be560\"}";
	msg.payloadlen = loadLen;
	md.message = &msg;
	md.topicName = &topicName;


	messageForAction(&md,1);

	//Construct Factory reset request
	//rebootCallBack(reqId,action,payload);

	topicName.lenstring.data = "iotdm-1/mgmt/initiate/device/factory_reset";
	topicName.lenstring.len = topicLen;

	msg.payload = "{\"reqId\":\"8c982454-3b44-4160-b577-cd6df67be5702\"}";
	msg.payloadlen = loadLen;
	md.message = &msg;
	md.topicName = &topicName;

	messageForAction(&md,0);

}

void testDeviceFirmwareActions(){
	assert_int_equal(connectiotf_dm(),0);

	setCommandHandler_dm(myCallback);
	setManagedHandler_dm(managedCallBack);
	setFirmwareDownloadHandler(firmwareDownloadCallBack);
	setFirmwareUpdateHandler(firmwareUpdateCallBack);
	subscribeCommands_dm();

	populateMgmtConfig();

	char reqId[40] = {0};
	char test[40] = {0};
	//reqId[0] = '\0';
	//assert_null(reqId);
	assert_string_equal(reqId, test);

	publishManageEvent(4000,1,1, reqId);

	assert_string_not_equal(reqId, test);



	//Construct message data for location update
	MessageData md;
	MQTTMessage msg;

	int loadLen = 400;
	int topicLen = 100;

	MQTTString topicName;
	topicName.lenstring.data = "iotdevice-1/device/update/location";
	topicName.lenstring.len = topicLen;

	msg.payload =
			"{\"reqId\" : \"b38faafc-53de-47a8-a940-e697552c3194\",\"d\" : {\"fields\" : [{\"field\" : \"location\",\"value\" : {\"latitude\" : \"latitude value\",\"longitude\" : \"longitude val\",\"elevation\" : \"some elevation\",\"accuracy\" : \"accuracy val\",\"measuredDateTime\" : \"date and time\",\"updatedDateTime\" : \"updated date\"}}]}}";
	msg.payloadlen = loadLen;
	md.message = &msg;
	md.topicName = &topicName;
	messageUpdate(&md);

	//Construct message data for firmware update
//	MessageData md;
//	MQTTMessage msg;
//
//	int loadLen = 400;
//	int topicLen = 100;

//	MQTTString topicName;
	topicName.lenstring.data = "iotdm-1/device/update";
	topicName.lenstring.len = topicLen;


	msg.payload = "{\"reqId\" : \"f38faafc-53de-47a8-a940-e697552c3194\",\"d\" : {\"fields\" : [{\"field\" : \"mgmt.firmware\",\"value\" : {\"version\" : \"some firmware version\",\"name\" : \"some firmware name\",\"uri\" : \"some uri for firmware location\",\"verifier\" : \"some validation code\",\"state\" : 0,\"updateStatus\" : 0,\"updatedDateTime\" : \"\"}}]}}";
	msg.payloadlen = loadLen;
	md.message = &msg;
	md.topicName = &topicName;
	messageUpdate(&md);


	//Construct request for observe for Firmware state
	topicName.lenstring.data =  "iotdm-1/observe";


	char payLoad[] = "{\"reqId\" : \"909b477c-cd37-4bee-83fa-1d568664fbe8\",\"d\" : {\"fields\" : [{\"field\":\"mgmt.firmware\"}]}}";
	msg.payload = payLoad;

	md.message = &msg;
	md.topicName = &topicName;
	messageObserve(&md);

	//Construct message request for firmware download
	topicName.lenstring.data = "iotdm-1/mgmt/initiate/firmware/download";
	msg.payload = "{\"reqId\" : \"7b244053-c08e-4d89-9ed6-6eb2618a8734\"}";
	md.message = &msg;
	md.topicName = &topicName;
	messageFirmwareDownload(&md);
	messageFirmwareDownload(&md);// Negative  test case where to test download after the state change


	//Construct message request for cancel
	topicName.lenstring.data = "iotdm-1/cancel";
	msg.payload = "{\"reqId\" : \"d9ca3635-64d5-46e2-93ee-7d1b573fb20f\",\"d\" : {\"fields\" : [{\"field\":\"mgmt.firmware\"}]}}";
	md.message = &msg;
	md.topicName = &topicName;
	messageCancel(&md);

	//Construct request for observe for Firmware state
	topicName.lenstring.data = "iotdm-1/observe";
	msg.payload =
			"{\"reqId\" : \"909b477c-cd37-4bee-83fa-1d568664fbe8\",\"d\" : {\"fields\" : [{\"field\":\"mgmt.firmware\"}]}}";
	md.message = &msg;
	md.topicName = &topicName;
	messageObserve(&md);

	//Construct message request for firmware update
	topicName.lenstring.data = "iotdm-1/mgmt/initiate/firmware/update";
	msg.payload = "{\"reqId\" : \"7b244053-c08e-4d89-9ed6-6eb2618a8734\"}";
	md.message = &msg;
	md.topicName = &topicName;
	messageFirmwareUpdate(&md);

	//Construct message request for cancel
	topicName.lenstring.data = "iotdm-1/cancel";
	msg.payload =
			"{\"reqId\" : \"d9ca3635-64d5-46e2-93ee-7d1b573fb20f\",\"d\" : {\"fields\" : [{\"field\":\"mgmt.firmware\"}]}}";
	md.message = &msg;
	md.topicName = &topicName;
	messageCancel(&md);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
        cmocka_unit_test(testInitialize),
        cmocka_unit_test(testInitializeConfigfile),
	cmocka_unit_test(testConnectIotf),
	//cmocka_unit_test_setup_teardown(testPublishEvent,setup,teardown),
	//cmocka_unit_test_setup_teardown(testManage,setup,teardown),
	//cmocka_unit_test_setup_teardown(testDeviceActions,setup,teardown),
	//cmocka_unit_test_setup_teardown(testDeviceFirmwareActions,setup,teardown)
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

void populateMgmtConfig(){
	strcpy(dmClient.DeviceData.deviceInfo.serialNumber, "10087" );
	strcpy(dmClient.DeviceData.deviceInfo.manufacturer , "IBM");
	strcpy(dmClient.DeviceData.deviceInfo.model , "7865");
	strcpy(dmClient.DeviceData.deviceInfo.deviceClass , "A");
	strcpy(dmClient.DeviceData.deviceInfo.description , "My Ras");
	strcpy(dmClient.DeviceData.deviceInfo.fwVersion , "1.0.0");
	strcpy(dmClient.DeviceData.deviceInfo.hwVersion , "1.0");
	strcpy(dmClient.DeviceData.deviceInfo.descriptiveLocation , "EGL C");
	strcpy(dmClient.DeviceData.metadata.metadata ,"{}");
}
