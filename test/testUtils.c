/* Contains the utility functions for testing */

#include<stdlib.h>
#include<string.h>

void getDeviceCfgFilePath(char* devCfgFilePath){
	char *ctest_home = getenv("CTEST_HOME");
	char *cfgFile = "device.cfg";

	strcpy(devCfgFilePath,ctest_home);
	strcat(devCfgFilePath,cfgFile);
	strcat(devCfgFilePath,"\0");
}

void getGatewayCfgFilePath(char* devCfgFilePath){
	char *ctest_home = getenv("CTEST_HOME");
	char *cfgFile = "gateway.cfg";

	strcpy(devCfgFilePath,ctest_home);
	strcat(devCfgFilePath,cfgFile);
	strcat(devCfgFilePath,"\0");
}

//int setCtestHome(){
//	int rc = -1;
//    //char *test_path = "/var/lib/jenkins/jobs/Watson-IoT-EmbeddedC/workspace/test/";
//	//char *test_path = "/var/lib/jenkins/iot-embeddedc/test/";
//	char *test_path = "/home/amit/hari/iot-embeddedc/test/";
//	rc = setenv("CTEST_HOME",test_path,1);
//
//	return rc;
//}
