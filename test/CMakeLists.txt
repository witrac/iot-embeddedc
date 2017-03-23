cmake_minimum_required(VERSION 2.8)
include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/lib")

add_executable(test_iotfclient test_iotfclient.c)
add_test(test_iotfclient test_iotfclient)

add_executable(test_gatewayclient test_gatewayclient.c)
add_test(test_gatewayclient test_gatewayclient)

add_executable(test_devicemgmt test_devicemgmt.c)
add_test(test_devicemgmt test_devicemgmt)

target_link_libraries(test_iotfclient iotfclient iotf_utils cmocka)
target_link_libraries(test_gatewayclient iotfgwayclient iotf_utils cmocka)
target_link_libraries(test_devicemgmt iotfdevmgt iotfclient iotf_utils cmocka m)
