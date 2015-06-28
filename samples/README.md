Embedded C Client Library - Samples
===================================

Usage
-----

Either run *make* or use *build.sh* to compile and build the samples.

    [root@localhost ~]# make

helloWorld.c
------------

This is sample application to demonstrate the quickstart in IoT
Foundation. Provide the MAC Address

    [root@localhost ~]# ./helloWorld 001122334455
    Connection Successful. Press Ctrl+C to quit
    View the visualization at https://quickstart.internetofthings.ibmcloud.com/#/device/001122334455
    Publishing the event stat with rc  0
    Publishing the event stat with rc  0
    Publishing the event stat with rc  0
    Publishing the event stat with rc  0
    Publishing the event stat with rc  0
    Publishing the event stat with rc  0
    ^C SigINT received.
    Quitting!!
    [root@localhost ~]#

sampleDevice.c
--------------

This is sample application to demonstrate the registered mode in IoT
Foundation. With registered mode, the devices can both publish events
and receive commands. You need to register for a device at
<https://internetofthings.ibmcloud.com/#/> and copy the credentials in
the *device.cfg* file

    [root@localhost ~]# vi device.cfg
    [root@localhost ~]# ./samplePublish
    Connecting to registered service with org xxxxx
    Publishing the event stat with rc  0
    Publishing the event stat with rc  0
    Publishing the event stat with rc  0
    ----------------------------------------------
    The command received :: blink
    format : json
    Payload is : {"numOfTimes" : 2}
    ----------------------------------------------
    Publishing the event stat with rc  0
    Publishing the event stat with rc  0
    ^C SigINT received.
    Quitting!!
