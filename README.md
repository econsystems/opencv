# OpenCV: Open Source Computer Vision Library

## Videoio Module

* This Videoio Module can be supported for  Opencv version 4.5.5 with both Linux and Windows OS.

### APIs Introduced

* bool getDevices(int &devices);
* bool getDeviceInfo(int device, String &deviceName, String &vid, String &pid, String &devicePath);
* bool getFormats(int &formats);
* bool getFormatType(int format, String &formatType, int &width, int &height, int &fps);
* bool setFormatType(int format)
* bool get(int propId, int &min, int &max, int &steppingDelta, int &supportedMode, int &currentValue, int &currentMode, int &defaultValue);
* bool set(int propId, int value, int mode);

### APIs Explanation

#### bool getDevices(int &devices);

* Get total number of camera devices connected to the port.


#### bool getDeviceInfo(int device, String &deviceName, String &vid, String &pid, String &devicePath);

* Get specific camera device information such Device name, Device path, VendorID and productID.


#### bool getFormats(int &formats);

* Get total number of video formats supported by the camera device.


#### bool getFormatType(int format, String &formatType, int &width, int &height, int &fps);

* Get video resolution, video format and fps supported by the camera


#### bool setFormatType(int format);

* Set the video format, resolution and fps to the device. 


#### bool get(int propId, int &min, int &max, int &steppingDelta, int &supportedMode, int &currentValue, int &currentMode, int &defaultValue);

* Get specific UVC Property(Brightness, Contrast,...) minimum value, maximum value, current value, default value, steppingDelta, current mode and supported mode.


#### bool set(int propId, int value, int mode);

* Set specific UVC Property(Brightness, Contrast, Hue,...) value and mode.


## Y8GrabberFilter

* The Y8GrabberFilter Transform Filter is to grab the Y8 Frames as SampleGrabber. The SampleGrabber Input Pin does not accept the Y8 Format.


## PythonScript 

* Python Script can be used to access the UVC settings, HID settings, Streaming and Image capture of any e-con System cameras in both Windows and Linux.


## OpenCVCam Command Line Application

* OpenCVCam command line application can be used to access the UVC settings, HID settings, Streaming and Image capture of any e-con System cameras in both Windows and Linux.


## How to Use

* OpenCV can be downloaded from [here](https://github.com/opencv/opencv)
```
	$ cd opencv

	$ git checkout <opencv version>
```

* Replace Videoio module from OpenCV with [this videoio module](https://github.com/econsystems/opencv/tree/master/Source)
* Build OpenCV using [this Installation manual](https://github.com/econsystems/opencv/tree/master/Documents)
* Information on how to use those newly Introduced APIs were [explained in API Documentation](https://github.com/econsystems/opencv/tree/master/Documents)
* OpenCVCam command line application can be downloaded [from here](https://github.com/econsystems/opencv/tree/master/Source), which is used to access OpenCV APIs
* Run Sample application using [this user manual](https://github.com/econsystems/opencv/tree/master/Documents)


## Supported Camera's

	* See3CAM_CU20
	* See3CAM_CU30
	* Seecam_CU38
	* See3CAM_CU130
	* See3CAM_CU135
	* See3CAM_10CUG
	* See3CAM_11CUG
	* See3CAM_12CUNIR
	* See3CAM_30 (Liquid Lens)
	* See3CAM_130 Autofocus
	* See3CAM_CU55
	* FSCAM_CU135
	* See3CAM_CU55M
	* See3CAM_20CUG
	* See3CAM_CU22
	* See3CAM_CU27
	* See3CAM_160
	* See3CAM_CU81
	* e-cam82USB
	* See3CAM_CU135M_H03R1
	* See3CAM_CU135M_H01R1
	* See3CAM_CU135M

## Releases

* Latest releases can be downloaded from [this link](https://github.com/econsystems/opencv/releases)


## What's new

* Added Support for OpenCV 4.5.5

## Release

* OpenCV v1.0.4		-	10-Oct-22
* OpenCV v1.0.3		-	15-Aug-20
* OpenCV v1.0.2		-	03-Jun-20
* OpenCV v1.0.1		-	17-Dec-19
* OpenCV v1.0.0		-	27-Jul-18
