Embedded C Client Library - Samples
===================================

Usage
-----

Using the cd command in the command prompt, navigate to the samples directory of your installation of the Embedded C client library.

Go to the samples directory to run the sample. Either run make or use build.sh to compile and build the samples.

    [root@localhost ~]# make

helloWorld.c
------------

This sample application demonstrates the IBM Watson Internet of Things Quickstart service. The sample application connects to the Quickstart service and sends event data. To connect to the Quickstart service, you must provide an unique identifier for the device.

This sample is a great place to get started with the IBM Watson Internet of Things Platform Service using Quickstart.

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

This sample application demonstrates the registered mode in IBM Watson Internet of Things Platform. On registered mode, the devices can both publish events and receive commands.

But for running this sample, you must first register the device at <https://internetofthings.ibmcloud.com/#/> and copy the credentials in the *device.cfg* file. Using this sample, users can publish events and receive commands from the IBM Watson Internet of Things Platform service.

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

sampleGateway.c
--------------

This sample application demonstrates the usage of Gateways with IBM Watson Internet of Things Platform. 
But for running this sample, you must first register your gateway at <https://internetofthings.ibmcloud.com/#/> and copy the credentials in the *gateway.cfg* file. Using this sample, users can publish gateway events, publish device events on behalf of a device and receive commands from the  IBM Watson Internet of Things Platform.

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
