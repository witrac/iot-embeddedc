cmake_minimum_required(VERSION 2.8)
include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/lib")

add_executable(test_deviceclient test_deviceclient.c)
add_test(test_deviceclient test_deviceclient)

add_executable(test_gatewayclient test_gatewayclient.c)
add_test(test_gatewayclient test_gatewayclient)

add_executable(test_devicemgmt test_devicemgmt.c)
add_test(test_devicemgmt test_devicemgmt)

target_link_libraries(test_deviceclient iotfdeviceclient cmocka)
target_link_libraries(test_gatewayclient iotfgatewayclient cmocka)
target_link_libraries(test_devicemgmt iotfdevicemgmt cmocka m)
