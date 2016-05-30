cmake_minimum_required(VERSION 2.8)
include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/lib")

add_library(testUtils testUtils.c)

add_executable(test_iotfclient test_iotfclient.c)
add_test(test_iotfclient test_iotfclient)

add_executable(test_devicemgmt test_devicemgmt.c)
add_test(test_devicemgmt test_devicemgmt)

add_executable(test_gatewayclient test_gatewayclient.c)
add_test(test_gatewayclient test_gatewayclient)

#set(LINK_LIBS "testUtils iotfclient libiotfdevmgt cmocka m")
target_link_libraries(test_iotfclient testUtils iotfclient cmocka)
#target_link_libraries(test_devicemgmt ${LINK_LIBS} ${LINK_LIBS})
#target_link_libraries(iotfclient libiotfdevmgt)
#target_link_libraries(iotfclient)
target_link_libraries(test_devicemgmt testUtils iotfdevmgt iotfclient cmocka m)
#target_link_libraries(test_devicemgmt ${LINK_LIBS} ${LINK_LIBS})
target_link_libraries(test_gatewayclient testUtils iotfgwayclient cmocka)
