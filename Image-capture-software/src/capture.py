#!/usr/bin/env python3
# coding=utf-8

import PySpin
import time
import os
import logging
import RPi.GPIO as GPIO

# Create logger
logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)
# Create file handler
fh = logging.FileHandler('/data/log/capture.log')
fh.setLevel(logging.DEBUG)
# Create a console handler
ch = logging.StreamHandler()
ch.setLevel(logging.DEBUG)
# Create formatter and add it to the handlers
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
fh.setFormatter(formatter)
ch.setFormatter(formatter)
# Add the handlers to the logger
logger.addHandler(fh)
logger.addHandler(ch)



try:
    #1/0
    # Increase the USB buffer size
    os.system('echo 1500 > /sys/module/usbcore/parameters/usbfs_memory_mb')
    # Set the system clock from the RTC
    os.system('hwclock -s')
    #
    logger.debug("Increased USB buffer size and set system clock from RTC")
except Exception as e:
    logger.debug("Exception increasing USB buffer size and set system clock from RTC: " + e.args[0])


try:
    GPIO.setmode(GPIO.BCM)
    # Strobe enable
    GPIO.setup(16, GPIO.OUT)
    GPIO.output(16, GPIO.HIGH)
    # Automatic shutdown bypass input
    GPIO.setup(5, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
except Exception as e:
    logger.debug("Exception with strobe GPIO enable: " + e.args[0])


try:
    system = PySpin.System.GetInstance()
    cam_list = system.GetCameras()
    logger.debug('# cameras detected: '+str(cam_list.GetSize()))
    cam = cam_list[0]
    cam.Init()
except Exception as e:
    logger.debug("Exception while initializing camera: " + e.args[0])


try:
    # Set manual buffer count and not auto buffer count
    # Recommended by FLIR for host devices with <= 2GB RAM
    # Number of buffers to preallocate
    NUM_BUFFERS = 1
    s_node_map = cam.GetTLStreamNodeMap()
    stream_buffer_count_mode = PySpin.CEnumerationPtr(s_node_map.GetNode("StreamBufferCountMode"))
    stream_buffer_count_mode_manual = PySpin.CEnumEntryPtr(stream_buffer_count_mode.GetEntryByName("Manual"))
    stream_buffer_count_mode.SetIntValue(stream_buffer_count_mode_manual.GetValue())
    buffer_count = PySpin.CIntegerPtr(s_node_map.GetNode("StreamBufferCountManual"))
    buffer_count.SetValue(NUM_BUFFERS)
except Exception as e:
    logger.debug("Exception while setting manual buffers: " + e.args[0])


try:
    assert cam.ExposureAuto.GetAccessMode() == PySpin.RW
    cam.ExposureAuto.SetValue(PySpin.ExposureAuto_Off)
    logger.debug("Automatic exposure disabled...")

    assert cam.AcquisitionMode.GetAccessMode() == PySpin.RW
    cam.AcquisitionMode.SetValue(PySpin.AcquisitionMode_SingleFrame)
    logger.debug("Single frame acquisition mode...")
except Exception as e:
    logger.debug("Exception while setting manual exposure and acquisition mode: " + e.args[0])


#List all the nodes available
#for node in cam.GetNodeMap().GetNodes():
#    pit = node.GetPrincipalInterfaceType()
#    name = node.GetName()
#    logger.debug(pit)
#    logger.debug(name)

try:
    # Select line (Optoisolated output)
    node_LineSelector = PySpin.CEnumerationPtr(cam.GetNodeMap().GetNode("LineSelector"))
    if not PySpin.IsAvailable(node_LineSelector) or not PySpin.IsWritable(node_LineSelector):
        logger.debug("Unable to select GPIO line (enum retrieval), aborting...")
    node_LineSelector_Line1 = node_LineSelector.GetEntryByName("Line1")
    if not PySpin.IsAvailable(node_LineSelector_Line1) or not PySpin.IsReadable(node_LineSelector_Line1):
        logger.debug("Unable to select GPIO line (entry retrieval), aborting...")
    LineSelector_Line1 = node_LineSelector_Line1.GetValue()
    node_LineSelector.SetIntValue(LineSelector_Line1)

    # Invert line polarity
    node_LineInverter = PySpin.CBooleanPtr(cam.GetNodeMap().GetNode("LineInverter"))
    if not PySpin.IsAvailable(node_LineInverter) or not PySpin.IsWritable(node_LineInverter):
        logger.debug("Unable to select line polarity node, aborting...")
    node_LineInverter.SetValue(True)

    # Select line mode (Output)
    node_LineMode = PySpin.CEnumerationPtr(cam.GetNodeMap().GetNode("LineMode"))
    if not PySpin.IsAvailable(node_LineMode) or not PySpin.IsWritable(node_LineMode):
        logger.debug("Unable to select line mode (enum retrieval), aborting...")
    node_LineMode_Output = node_LineMode.GetEntryByName("Output")
    if not PySpin.IsAvailable(node_LineMode_Output) or not PySpin.IsReadable(node_LineMode_Output):
        logger.debug("Unable to select line mode (entry retrieval), aborting...")
    LineMode_Output = node_LineMode_Output.GetValue()
    node_LineMode.SetIntValue(LineMode_Output)

    # Select line source (Exposure Active)
    node_LineSource = PySpin.CEnumerationPtr(cam.GetNodeMap().GetNode("LineSource"))
    if not PySpin.IsAvailable(node_LineSource) or not PySpin.IsWritable(node_LineSource):
        logger.debug("Unable to select line source (enum retrieval), aborting...")
    node_LineSource_Exposure = node_LineSource.GetEntryByName("ExposureActive")
    if not PySpin.IsAvailable(node_LineSource_Exposure) or not PySpin.IsReadable(node_LineSource_Exposure):
        logger.debug("Unable to select line source (entry retrieval), aborting...")
    LineSource_Exposure = node_LineSource_Exposure.GetValue()
    node_LineSource.SetIntValue(LineSource_Exposure)

    # Select user set 0
    node_UserSetSelector = PySpin.CEnumerationPtr(cam.GetNodeMap().GetNode("UserSetSelector"))
    if not PySpin.IsAvailable(node_UserSetSelector) or not PySpin.IsWritable(node_UserSetSelector):
        logger.debug("Unable to select user set (enum retrieval), aborting...")
    node_UserSetSelector_UserSet0 = node_UserSetSelector.GetEntryByName("UserSet0")
    if not PySpin.IsAvailable(node_UserSetSelector_UserSet0) or not PySpin.IsReadable(node_UserSetSelector_UserSet0):
        logger.debug("Unable to select user set (entry retrieval), aborting...")
    UserSetSelector_UserSet0 = node_UserSetSelector_UserSet0.GetValue()
    node_UserSetSelector.SetIntValue(UserSetSelector_UserSet0)

    # Command - save selected user set
    node_UserSetSave_cmd = PySpin.CCommandPtr(cam.GetNodeMap().GetNode("UserSetSave"))
    if not PySpin.IsAvailable(node_UserSetSave_cmd) or not PySpin.IsWritable(node_UserSetSave_cmd):
        logger.debug("Unable to select user set save command, aborting...")
    node_UserSetSave_cmd.Execute()

    # Save user set 0 as default to be applied when the camera powers up
    node_UserSetDefault = PySpin.CEnumerationPtr(cam.GetNodeMap().GetNode("UserSetDefault"))
    if not PySpin.IsAvailable(node_UserSetDefault) or not PySpin.IsWritable(node_UserSetDefault):
        logger.debug("Unable to select user set default (enum retrieval), aborting...")
    node_UserSetDefault_UserSet0 = node_UserSetDefault.GetEntryByName("UserSet0")
    if not PySpin.IsAvailable(node_UserSetDefault_UserSet0) or not PySpin.IsReadable(node_UserSetDefault_UserSet0):
        logger.debug("Unable to select user set default (entry retrieval), aborting...")
    UserSetDefault_UserSet0 = node_UserSetDefault_UserSet0.GetValue()
    node_UserSetDefault.SetIntValue(UserSetDefault_UserSet0)

except Exception as e:
    logger.debug("Exception while setting strobe mode: " + e.args[0])



try:
    timestr=time.strftime('%Y-%m-%d-%H-%M-%S')
    # ********** THE FOLLOWING LIST CONTAINS ALL THE EXPOSURE TIMES FOR A GIVEN WAKEUP CYCLE
    # ********** ADJUST TO BEST SUIT THE APPLICATION
    for exp_ms in [16.0, 32.0, 64.0, 128.0, 256.0]:
        cam.ExposureTime.SetValue(exp_ms*1e3)
        logger.debug("Beginning acquisition of picture at exposure: %1.2f ms" % (exp_ms))
        cam.BeginAcquisition()
        logger.debug("Begin acquisition call ended")
        image = cam.GetNextImage()
        logger.debug("Picture taken")
        filename = '/data/image/'+timestr+'_%1.2fms.png' % (exp_ms)
        image.Save(filename)
        logger.debug('Saved '+filename)
        image.Release()
        cam.EndAcquisition()
except Exception as e:
    logger.debug("Exception while taking picture: " + e.args[0])


try:
    GPIO.output(16, GPIO.LOW)
except Exception as e:
    logger.debug("Exception while disabling strobe enable signal: " + e.args[0])


try:
    cam.DeInit()
    del cam
    cam_list.Clear()
    system.ReleaseInstance()
except Exception as e:
    logger.debug("Exception while releasing camera resources: " + e.args[0])

try:
    if GPIO.input(5) == GPIO.HIGH:
        logger.debug("GPIO 5 is high, switch was closed, bypassing autoshutdown")
    elif GPIO.input(5) == GPIO.LOW:
        logger.debug("GPIO 5 is low, switch was open or missing, executing automatic shutdown")
        os.system("sudo shutdown -h now")
except Exception as e:
    logger.debug("Exception while reading autoshutdown disable input GPIO: " + e.args[0])

