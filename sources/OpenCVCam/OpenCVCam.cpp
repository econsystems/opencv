#ifdef _WIN32
#pragma once
#include <Windows.h>
#include "eCAMFwSw.h"
#include <SDKDDKVer.h>
#include <tchar.h>
#include <string>
#include <conio.h>
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "imgcodecs.hpp"
#include <mutex>
#endif

#ifdef __linux__
#include "opencv2/opencv.hpp"
#include <string.h>
#include <pthread.h>
#include <mutex>
#include <chrono>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <libudev.h>
#endif

#include <stdio.h>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;
using namespace cv;

#define EXIT				0
#define AUTO				1
#define MANUAL				2
#define AUTOANDMANUAL		3
#define	READFIRMWAREVERSION	0X40
#define BUFFERLENGTH		65

#ifdef _WIN32

#define PROPERTY			19

typedef BOOL (*Readfirmwareversion_t) (UINT8 *, UINT8 *, UINT16 *, UINT16 *);
typedef BOOL (*Initextensionunit_t) (TCHAR *);
typedef BOOL (*Deinitextensionunit_t) ();
Readfirmwareversion_t readfirmwareversion;
Initextensionunit_t initextensionunit;
Deinitextensionunit_t deinitextensionunit;

#elif __linux__

#define PROPERTY			17

#endif

//Variable Declarations
VideoCapture cap;
Mat Frame, BayerFrame8, IRImage, BGRImage, ResultImage;
char keyPressed = '\0', dilemma = 'y';
int devices = 0, formats = 0, width = 0, height = 0, fps = 0;
long minimum = 0, maximum = 0, defaultValue = 0, currentValue = 0, steppingDelta = 0, supportedMode = 0, currentMode = 0, value = 0;
vector< pair <int, String> > uvcProperty;
unsigned char outputBuffer[BUFFERLENGTH], inputBuffer[BUFFERLENGTH];
String deviceName, vid, pid, devicePath, formatType;
bool bOpenHID = false, bCapture, bPreview, bSwitch, _12CUNIR, _CU51, _CU40, _10CUG_C;
mutex mu;

#ifdef _WIN32

TCHAR *tDevicePath;
HINSTANCE hinstLib;
bool bDetach = false;
thread t;

#elif __linux__

static int hid_fd;
pthread_t threadId;

#endif


//Function Declarations
bool listDevices();
bool exploreCam();


bool bReadSet(int tid, bool bRead)
{
	std::lock_guard<std::mutex> guard(mu);
	
	if(tid == 1)
	{
		bCapture = bRead;
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}

	return bCapture;
}

bool bPreviewSet(int tid, bool bPrev)
{	
	std::lock_guard<std::mutex> guard(mu);

	if(tid == 1)
    {
        bPreview = bPrev;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return bPreview;
}

// Actual Data format BGIRR after conversion BGGR - IR is replaced with the G 
//IR data is collected as a separate image
bool ConvertRGIR2RGGB(Mat BayerRGIR, Mat &BayerRGGB, Mat &IRimage)
{
	//Result image after replacing the IR pixel with the G data
	BayerRGGB = BayerRGIR.clone();
	
	//IR data will be half the size of Bayer Image
	IRimage = Mat(BayerRGIR.size() / 2, CV_8UC1);

	//copying the IR data and replacing the IR data with G
	for (int Row = 0; Row < BayerRGIR.rows; Row += 2)
	{
		for (int Col = 0; Col < BayerRGIR.cols; Col += 2)
		{
			//Set the IR Data with Nearby Green 
			BayerRGGB.at<uchar>(Row + 1, Col) = BayerRGIR.at<uchar>(Row, Col + 1);
			//Set the IR Data 
			IRimage.at<uchar>(Row / 2, Col / 2) = BayerRGIR.at<uchar>(Row + 1, Col);
		}
	}

	return true;
}

#ifdef _WIN32

//Preview Window for Windows
//
void stream()
{
	bCapture = true;
    while(true)
    {
        while(bPreviewSet(2, true))
        {
            if(bReadSet(2, true))
            {
                cap >> Frame;
            }
		
			if(!Frame.empty())
			{
				if((_12CUNIR) || (_CU51))
				{		
					//Convert to 8 Bit: 
					//Scale the 12 Bit (4096) Pixels into 8 Bit(255) (255/4096)= 0.06226
					convertScaleAbs(Frame, ResultImage, 0.06226);
	
					namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					imshow("OpenCVCam", ResultImage);
				}
				else if(_CU40)
				{
					//Convert to 8 Bit: 
					//Scale the 10 Bit (1024) Pixels into 8 Bit(255) (255/1024)= 0.249023
					convertScaleAbs(Frame, BayerFrame8, 0.249023);
	
					//Filling the missing G -channel bayer data
					ConvertRGIR2RGGB(BayerFrame8, BayerFrame8, IRImage);
			
					//Actual Bayer format BG but Opencv uses BGR & Not RGB So taking RG Bayer format
					cvtColor(BayerFrame8, BGRImage, COLOR_BayerRG2BGR);

					namedWindow("OpenCVCam BGR Frame", WINDOW_AUTOSIZE);
					imshow("OpenCVCam BGR Frame", BGRImage);
	
					namedWindow("OpenCVCam IR Frame", WINDOW_AUTOSIZE);
					imshow("OpenCVCam IR Frame", IRImage);
				}
				else
				{
					namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					imshow("OpenCVCam", Frame);
				}
			}
            keyPressed = waitKey(10);

            while(bSwitch)
            {
                destroyAllWindows();
            }
        }
    }
}

#elif __linux__

bool closeHID()
{
	if(hid_fd > 0)
		close(hid_fd);
	return true;
}

//Preview Window for Linux
//
void *preview(void *arg)
{
    bCapture = true;
    while(true)
    {
        while(bPreviewSet(2, true))
        {
            if(bReadSet(2, true))
            {
                cap >> Frame;
            }
			
			if(!Frame.empty())
			{
				if((_12CUNIR) || (_CU51))
				{		
					//Convert to 8 Bit: 
					//Scale the 12 Bit (4096) Pixels into 8 Bit(255) (255/4096)= 0.06226
					convertScaleAbs(Frame, ResultImage, 0.06226);

					namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					imshow("OpenCVCam", ResultImage);
				}
				else if(_CU40)
				{
					//Convert to 8 Bit: 
					//Scale the 10 Bit (1024) Pixels into 8 Bit(255) (255/1024)= 0.249023
					convertScaleAbs(Frame, BayerFrame8, 0.249023);

					//Filling the missing G -channel bayer data
					ConvertRGIR2RGGB(BayerFrame8, BayerFrame8, IRImage);
			
					//Actual Bayer format BG but Opencv uses BGR & Not RGB So taking RG Bayer format
					cvtColor(BayerFrame8, BGRImage, COLOR_BayerRG2BGR);

					namedWindow("OpenCVCam BGR Frame", WINDOW_AUTOSIZE);
					imshow("OpenCVCam BGR Frame", BGRImage);	
	
					namedWindow("OpenCVCam IR Frame", WINDOW_AUTOSIZE);
					imshow("OpenCVCam IR Frame", IRImage);
				}
				else if(_10CUG_C) //10CUG and other camera's
				{
					cvtColor(Frame, BGRImage, COLOR_BayerGB2BGR);
					namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					imshow("OpenCVCam", BGRImage);
				}
    	        else
				{
					namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					imshow("OpenCVCam", Frame);
				}
			}
            keyPressed = waitKey(5);

            while(bSwitch)
            {
                destroyAllWindows();
            }
        }
    }
}

#endif

int main()
{
	//Basic Introduction about the Application
	cout << endl << "e-con's Sample OpenCV Application to Custom Formats " << endl;
	cout << endl << "Demonstrates the working of e-con's Custom Format cameras with the modified libraries of OpenCV" << endl;

#ifdef _WIN32
	hinstLib = LoadLibrary(L"eCAMFwSw.dll");
	if(hinstLib == NULL)
	{
		cout << "The eCAMFwSw.dll is not loaded properly" <<  endl;
		return 0;
	}

	readfirmwareversion = (Readfirmwareversion_t) GetProcAddress(hinstLib, "ReadFirmwareVersion");
	initextensionunit = (Initextensionunit_t) GetProcAddress(hinstLib, "InitExtensionUnit");
	deinitextensionunit = (Deinitextensionunit_t) GetProcAddress(hinstLib, "DeinitExtensionUnit");

#endif

	//Open a Camera Device
	if(!(listDevices()))
	{
		cout << endl << "List Devices Information Failed" << endl;
        cout << endl << '\t' << "Press Any key to exit the Application: " << '\t';
#ifdef _WIN32
		_getch();
#endif
        return 0;
	}

#ifdef _WIN32
	t = thread(&stream);
#elif __linux__
	pthread_create(&threadId, NULL, preview, NULL);
#endif

	if(!(exploreCam()))
	{
		cout << endl << "Camera Exploration Failed" << endl;
		return 0;
	}
	
#ifdef _WIN32
	t.detach();
#endif
	
	if(cap.isOpened())
	{
		cap.release();
	}

	return 0;
}


//Listing the Devices
//
bool listDevices()
{
	int camId = -1;
	//List total Number of Devices
#ifdef __linux__
	bSwitch = true;
#endif	
	bSwitch = true;
	if(!(cap.getDevices(devices)))
    {
        cout << endl << "Get total number of devices Failed" << endl;
        return false;
    }

    if( devices < 0 )
    {
        cout << endl << "No Camera Devices Connected to the port" << endl;
        return false;
    }

    cout << endl << "Number of Camera Devices Connected to the Port : " << devices << endl;
    cout << endl << "Camera Devices Connected to the PC Port : " << endl << endl;
    cout << '\t' << "0 - Exit" << endl;

	//List the Camera Names
	for(int eachDevice = 0; eachDevice < devices; eachDevice++)
	{
		if(!(cap.getDeviceInfo(eachDevice, deviceName, vid, pid, devicePath)))
		{
			cout << endl << "Device " << eachDevice << " Information couldn't be Retrieved" << endl;
		}

		cout << '\t' << eachDevice+1 << " . " << deviceName << endl;
        /*cout << '\t' << eachDevice+1 << " . " << vid << endl; 
        cout << '\t' << eachDevice+1 << " . " << pid << endl; 
        cout << '\t' << eachDevice+1 << " . " << devicePath << endl;*/
	}

	while((camId < 0) || (camId > devices))
	{
		printf("\n Pick a Camera Device to Explore : \t");
		scanf("%d", &camId);
		while(getchar() != '\n' && getchar() != EOF)
		{
		}
	}

	switch(camId)
	{
	case EXIT:
#ifdef _WIN32

		if(deinitextensionunit())
			if(bDetach)
				t.detach();
		bSwitch = true;
		if(cap.isOpened())
			cap.release();

#elif __linux__

		bPreviewSet(1, false);
    	if(closeHID())
    		destroyAllWindows();

#endif	

		exit(0);
		break;	

	default:
		bPreviewSet(1, false);
		cap.getDeviceInfo((camId-1), deviceName, vid, pid, devicePath);
		if((vid == "2560") && (pid == "c140"))
		{
			_CU40 = true;
			_CU51 = false;
			_12CUNIR = false;
			_10CUG_C = false;
		}
		else if((vid == "2560") && (pid == "c152"))
		{
			_CU40 = false;
			_CU51 = true;
			_12CUNIR = false;
			_10CUG_C = false;
		}
		else if((vid == "2560") && (pid == "c113"))
		{
			_CU40 = false;
			_CU51 = false;
			_12CUNIR = true;
			_10CUG_C = false;
		}
		else if((vid == "2560") && (pid == "c111"))
		{
			_CU40 = false;
			_CU51 = false;
			_12CUNIR = false;
			_10CUG_C = true;
		}

		if(cap.isOpened())
			cap.release();

#ifdef __linux__
		struct udev *udev;
		struct udev_enumerate *enumerate;
		struct udev_list_entry *devices, *dev_list_entry;
		struct udev_device *dev;
		udev = udev_new();
		if (!udev) 
		{
       		printf("Can't create udev\n");
	        exit(1);
	    }
		int camdevices = 0, indexId;
    	enumerate = udev_enumerate_new(udev);
	    udev_enumerate_add_match_subsystem(enumerate, "video4linux");
    	udev_enumerate_scan_devices(enumerate);
	    devices = udev_enumerate_get_list_entry(enumerate);
	    udev_list_entry_foreach(dev_list_entry, devices) 
		{
	        const char *pathvid;	
	        pathvid = udev_list_entry_get_name(dev_list_entry);
	        dev = udev_device_new_from_syspath(udev, pathvid);
			const char *pathvalue = udev_device_get_devnode(dev);
			indexId = pathvalue[10] - '0';
	        udev_device_unref(dev);
			camdevices++;	
			if(camdevices == camId)
				break;
	    }
	    udev_enumerate_unref(enumerate);
	
	    udev_unref(udev);

#endif

#ifdef _WIN32
		if(cap.open(camId - 1))
#elif __linux__
		if(cap.open(indexId))
#endif
    	{
    		if(!cap.isOpened())
	        {
    	    	cout << endl << "\t Camera Device not Initialised Successfully \n\n Press any Key to exit the application\n" ;
        	    return 0;
	        }
    	}
		if((vid == "2560") && (pid == "c140") || (pid == "c152") || (pid == "c113") || (pid == "c111"))
		{	
			cap.set(CV_CAP_PROP_CONVERT_RGB, false);
		}

#ifdef _WIN32

		tDevicePath = new TCHAR[devicePath.size() + 1];
		copy(devicePath.begin(), devicePath.end(), tDevicePath);
		bOpenHID = initextensionunit(tDevicePath);

#elif __linux__

		udev = udev_new();
		if (!udev) 
		{
			printf("Can't create udev\n");
		}
	
		enumerate = udev_enumerate_new(udev);
		udev_enumerate_add_match_subsystem(enumerate, "hidraw");
		udev_enumerate_scan_devices(enumerate);
		devices = udev_enumerate_get_list_entry(enumerate);

		udev_list_entry_foreach(dev_list_entry, devices) 
		{
			const char *path;
			String hidPath, VID, PID;
		
			path = udev_list_entry_get_name(dev_list_entry);
			dev = udev_device_new_from_syspath(udev, path);

			//HID Device Path
			hidPath = udev_device_get_devnode(dev);
			dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
			if (!dev) 
			{
				printf("Unable to find parent usb device.");
			}

			VID = udev_device_get_sysattr_value(dev,"idVendor");
			PID = udev_device_get_sysattr_value(dev, "idProduct");

			if((vid == VID) && (pid == PID))
				devicePath = hidPath;

			udev_device_unref(dev);
		}

		udev_enumerate_unref(enumerate);
		udev_unref(udev);  

		if(hid_fd > 0)
			closeHID();

		hid_fd = ::open(devicePath.c_str(), O_RDWR | O_NONBLOCK, 0);
        if(hid_fd < 0)
            bOpenHID = false;
        bOpenHID = true;
#endif
		bSwitch = false;
		bPreviewSet(1, true);
		break;
	}
#ifdef _WIN32
	bDetach = true;
#endif
	return true;
}


//Configuring Camera Format/Resolution
//
bool configFormats()
{
	while((dilemma == 'y') || (dilemma == 'Y'))
	{	
		int index = -1;
		
		if(!(cap.getFormats(formats)))
		{
			cout << endl << "Get Total number of Formats Failed" << endl;
		        return false;
		}
		
#ifdef _WIN32
		cout << endl << "Total Number of Formats Supported by the Camera:  " << '\t' << formats/2 << endl;
#elif __linux__
		cout << endl << "Total Number of Formats supported by the Camera:  " << '\t' << formats << endl;
#endif

        cout << '\t' << "0 - Exit" << endl;
        cout << '\t' << "1 - Back" << endl;
        cout << '\t' << "2 - Main Menu" << endl;
        int option = 3;

		for(int formatList = 0; formatList < formats; formatList++)
		{
#ifdef _WIN32

			//Skipping the Format_VideoInfo2, Only Format_VideoInfo is allowed
            if((formatList % 2) == 0)
            {
                //Display Camera Formats Supported by the Camera
                if(!(cap.getFormatType(formatList, formatType, width, height, fps)))
                {
                    cout << endl << "Camera Get Format Type Failed" << endl;
                    return false;
                }
                cout << '\t' << option << " . " << "FormatType: " << formatType << " Width: " << width << " Height: " << height << " Fps: " << fps << endl;
                option++;
            }

#elif __linux__

			if(!(cap.getFormatType(formatList, formatType, width, height, fps)))
            {
                cout << endl << "Camera Get Format Type Failed" << endl;
                return false;
            }

            cout << '\t' << option << " . " << "FormatType = " << formatType << "  Width = " << width << "  Height = " << height << " FPS = " << fps << endl;
            option++;

#endif
		}

		while((index < 0) || (index >= option))
        {
            printf("\nPick a choice to set a Particular Preview Format: \t");
			scanf("%d", &index);
			while(getchar() != '\n' && getchar() != EOF)
			{
			}
        }
		
		switch(index)
		{
		case EXIT:
#ifdef _WIN32

			if(deinitextensionunit())
				t.detach();
			bSwitch = true;
			if(cap.isOpened())
				cap.release();

#elif __linux__

			bPreviewSet(1, false);
            if(closeHID())
                destroyAllWindows();

#endif

			exit(0);

		case 1:
		case 2:
			exploreCam();
			break;

		default:
			bReadSet(1, false);
#ifdef _WIN32
			
			if(!(cap.setFormatType(2*(index - 3))))
            {
                cout << endl << "Camera Set Format Type Failed" << endl;
                return false;
            }

#elif __linux__

			if(!(cap.setFormatType(index - 3)))
            {
                cout << endl << "Camera Set Format Type Faied" << endl;
                return false;
            }
            
#endif
			bReadSet(1, true);

			formatType = '\0';
			width = height = fps = 0;
			break;
		}
	}

	return true;
}


//Set Video Property
//
bool setVidProp(int Property, string PropStr)
{
	while((dilemma == 'y') || (dilemma == 'Y'))
	{
		int mode = 0;
		
		if(!(cap.get(Property, minimum, maximum, steppingDelta, supportedMode, currentValue, currentMode, defaultValue)))
        {
            cout << endl << PropStr << " Properties Couldn't be Retrieved for the Camera Connected" << endl;
            return false;
        }

		cout << endl << "Camera " << PropStr << " Values:: " << endl;
		
		if(supportedMode != AUTO)
        {
            cout << '\t' << "Minimum Value: " << minimum << endl ;
            cout << '\t' << "Maximum Value: " << maximum << endl ;
            cout << '\t' << "SteppingDelta: " << steppingDelta << endl ;    //incrementing scale Value between min and max value
			cout << '\t' << "Default Value: " << defaultValue << endl;
        }

		((supportedMode < AUTOANDMANUAL) ? ((supportedMode == AUTO) ? (cout << '\t' << "Supported Mode: Auto" << endl) : (cout << '\t' << "Supported Mode: Manual" << endl)) : (cout << '\t' << "Supported Mode: Auto/Manual" << endl));

		if(currentMode == MANUAL)
            cout << '\t' << "Current Value: " << currentValue << endl;
        ((currentMode == AUTO) ? (cout << '\t' << "Current Mode: Auto" << endl << endl) : (cout << '\t' << "Current Mode: Manual" << endl << endl));

		switch(supportedMode)
		{
		case AUTO:
			cout << "Only Auto " << PropStr << " is Supported by the Camera, User can't set the Mode" << endl << endl;
            mode = AUTO;
            break;

		case MANUAL:
			cout << "Only Manual " << PropStr << " is Supported by the Camera, User can't set the Mode " << endl << endl;
             mode = MANUAL;
            break;
		
		case AUTOANDMANUAL:
			while((mode <= 0) || (mode > 2))
			{
				printf("\n Enter a Valid mode to get selected: 1. Auto 2. Manual \n");
				scanf("%d", &mode);
				while(getchar() != '\n' && getchar() != EOF)
				{	
				}
			}
			break;
		}
	
		while(mode == MANUAL)
		{
			cout << endl << "Enter a Valid value to Set " << PropStr << " : " << '\t';
			scanf("%ld", &value);
			while(getchar() != '\n' && getchar() != EOF)
			{	
			}
            if( (value >= minimum) && (value <= maximum) && ((value%steppingDelta) == 0))
            {
                break;
            }
            cout << endl << "Certain Conditions Not met; Please ";
		}

		if(!(cap.set(Property, value, mode)))
		{
			cout << endl << "Camera " << PropStr << " is Not Set" << endl;
            return false;
		}

		((mode == AUTO) ? (cout << endl << "Auto " << PropStr << " Mode is Set" << endl) : (cout << endl << "Manual " << PropStr << " Mode is set with the Value : " << value << endl));

		cout << endl << "Camera " << PropStr << " Exploration: " << '\n' ;
		while(true)
		{
			cout << endl << "Enter y/Y to Continue or n/N to dis-Continue: " << '\t';
			scanf("%c", &dilemma);
			while(getchar() != '\n' && getchar() != EOF)
			{}
			if( dilemma == 'y' || dilemma == 'Y' || dilemma == 'n' || dilemma == 'N')
				break;
		}
	}

	return true;
}


//Configuring UVC Settings
//
bool configUVCSettings()
{
#ifdef _WIN32

	int vid[PROPERTY] = {EXIT, 1, 2, CV_CAP_PROP_BRIGHTNESS, CV_CAP_PROP_CONTRAST, CV_CAP_PROP_SATURATION, CV_CAP_PROP_HUE, CV_CAP_PROP_GAIN, CV_CAP_PROP_EXPOSURE, CV_CAP_PROP_WHITE_BALANCE_BLUE_U, CV_CAP_PROP_SHARPNESS,
                            CV_CAP_PROP_GAMMA, CV_CAP_PROP_ZOOM, CV_CAP_PROP_FOCUS, CV_CAP_PROP_BACKLIGHT, CV_CAP_PROP_PAN, CV_CAP_PROP_TILT, CV_CAP_PROP_ROLL, CV_CAP_PROP_IRIS};

    string vidStr[PROPERTY] = {"Exit", "Back", "Main Menu", "Brightness", "Contrast", "Saturation", "Hue", "Gain", "Exposure", "White Balance", "Sharpness", "Gamma", "Zoom", "Focus", "Backlight", "Pan", "Tilt", "Roll", "Iris"};

#elif __linux__
	
	int vid[PROPERTY] = {EXIT, 1, 2, CV_CAP_PROP_BRIGHTNESS, CV_CAP_PROP_CONTRAST, CV_CAP_PROP_SATURATION, CV_CAP_PROP_HUE, CV_CAP_PROP_GAIN, CV_CAP_PROP_EXPOSURE, CV_CAP_PROP_WHITE_BALANCE_BLUE_U, CV_CAP_PROP_SHARPNESS, CV_CAP_PROP_GAMMA, CV_CAP_PROP_ZOOM, CV_CAP_PROP_FOCUS, CV_CAP_PROP_BACKLIGHT, CV_CAP_PROP_PAN, CV_CAP_PROP_TILT};
    string vidStr[PROPERTY] = {"Exit", "Back", "Main Menu", "Brightness", "Contrast", "Saturation", "Hue", "Gain", "Exposure", "White Balance", "Sharpness", "Gamma", "Zoom", "Focus", "Backlight", "Pan", "Tilt"};

#endif

	while(true)
	{
		int choice = -1, settings = 0;

		for(int eachVideoSetting = 0; eachVideoSetting < PROPERTY; eachVideoSetting++)
		{
			if(!((eachVideoSetting >= 0) && (eachVideoSetting < 3)))
			{
				//Checks whether the Specific UVC Property is supported or not
                if(cap.get(vid[eachVideoSetting], minimum, maximum, steppingDelta, supportedMode, currentValue, currentMode, defaultValue))
                {
                    cout << '\t' << settings << " - " << vidStr[eachVideoSetting] << endl;
                    uvcProperty.push_back(make_pair(vid[eachVideoSetting], vidStr[eachVideoSetting]));
                    settings++;
                }
                continue;
			}			
			uvcProperty.push_back(make_pair(vid[eachVideoSetting], vidStr[eachVideoSetting]));
            cout << '\t' << settings << " - " << vidStr[eachVideoSetting] << endl;
            settings++;
		}

		while((choice < 0) || (choice >= settings))
        {
			printf("\n Pick a Choice to Configure UVC Settings : \t");
			scanf("%d", &choice);
			while(getchar() != '\n' && getchar() != EOF)
			{	
			}
        }

		dilemma = 'y';

		switch(uvcProperty[choice].first)
		{
		case EXIT:
#ifdef _WIN32

			if(deinitextensionunit())
				t.detach();
			bSwitch = true;
			if(cap.isOpened())
				cap.release();

#elif __linux__

			bPreviewSet(1, false);
			if(closeHID())
				destroyAllWindows();

#endif		

			exit(0);
	
 		case 1:
		case 2:
			if(!(exploreCam()))
            {
                cout << endl << "Camera Exploration Failed" << endl;
                return false;
            }
            cout << endl << "Camera Exploration is done" << endl;
            break;
		
		case CV_CAP_PROP_BRIGHTNESS:
		case CV_CAP_PROP_CONTRAST:
        case CV_CAP_PROP_HUE:
        case CV_CAP_PROP_SATURATION:
        case CV_CAP_PROP_SHARPNESS:
        case CV_CAP_PROP_GAMMA:
        case CV_CAP_PROP_WHITE_BALANCE_BLUE_U:
        case CV_CAP_PROP_BACKLIGHT:
        case CV_CAP_PROP_GAIN:
        case CV_CAP_PROP_PAN:
        case CV_CAP_PROP_TILT:
        case CV_CAP_PROP_ROLL:
        case CV_CAP_PROP_ZOOM:
        case CV_CAP_PROP_EXPOSURE:
        case CV_CAP_PROP_IRIS:
        case CV_CAP_PROP_FOCUS:
            if(!(setVidProp(uvcProperty[choice].first, uvcProperty[choice].second)))
            {
                cout << endl << "Set Video Property Failed" << endl;
                return false;
            }
            cout << endl << "Camera " << uvcProperty[choice].second << " is Modified" << endl;
            break;
		}
	}

	return true;
}


//Capture Still Images
//
bool captureStill()
{
	int num;
	char buf[240], buf1[240];
	memset(buf, 0, 240);
	memset(buf1, 0, 240);
	time_t t = time(0);
#ifdef _WIN32
	
	struct tm tm;
    localtime_s(&tm, &t);
	
	if(_CU40)
	{
		num = sprintf_s(buf, "OpenCVCamBGR%d%d%d%d%d%d.jpeg", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
		num = sprintf_s(buf1, "OpenCVCamIR%d%d%d%d%d%d.jpeg", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
	else
		num = sprintf_s(buf, "OpenCVCam%d%d%d%d%d%d.jpeg", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

#elif __linux__

	struct tm *tm;
    tm = localtime(&t);

    char cwd[256];
    getcwd(cwd, sizeof(cwd));

	if(_CU40)
	{	sprintf(buf, "%s/OpenCVCamBGR%d%d%d%d%d%d.jpeg", cwd, tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
		sprintf(buf1, "%s/OpenCVCamIR%d%d%d%d%d%d.jpeg", cwd, tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
	}
	else
		sprintf(buf, "%s/OpenCVCam%d%d%d%d%d%d.jpeg", cwd, tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);

#endif
	if(cap.read(Frame))
	{
		if(!Frame.empty())
		{
			if((_12CUNIR) || (_CU51))
			{		
				//Convert to 8 Bit: 
				//Scale the 12 Bit (4096) Pixels into 8 Bit(255) (255/4096)= 0.06226
				convertScaleAbs(Frame, ResultImage, 0.06226);
				imwrite(buf, ResultImage);
			}
			else if(_CU40)
			{
				//Convert to 8 Bit: 
				//Scale the 10 Bit (1024) Pixels into 8 Bit(255) (255/1024)= 0.249023
				convertScaleAbs(Frame, BayerFrame8, 0.249023);
	
				//Filling the missing G -channel bayer data
				bPreviewSet(1, false);
				ConvertRGIR2RGGB(BayerFrame8, BayerFrame8, IRImage);
				bPreviewSet(1, true);
			
				//Actual Bayer format BG but Opencv uses BGR & Not RGB So taking RG Bayer format
				cvtColor(BayerFrame8, BGRImage, COLOR_BayerRG2BGR);

				imwrite(buf, BGRImage);		
				imwrite(buf1, IRImage);		
				cout << endl << '\t' << buf1 << " image is saved " << endl;
			}
			else
			{
				imwrite(buf, Frame);
			}
			cout << endl << '\t' << buf << " image is saved " << endl << endl;
		}
	}

	memset(buf, 0, 240);
	memset(buf1, 0, 240);

	return true;

}


//Configure Extension UVC Settings
//
bool configExtUVCSettings()
{
#ifdef __linux__
	int result;
	memset(inputBuffer, 0x00, BUFFERLENGTH);
    memset(outputBuffer, 0x00, BUFFERLENGTH);

	//Writes data to the HID Device
    inputBuffer[0] = 0x00;
    inputBuffer[1] = READFIRMWAREVERSION;

	if(hid_fd < 0)
        result = false;

    int ret = 0;
    ret = ::write(hid_fd, inputBuffer, BUFFERLENGTH);

    if(ret < 0)
        result = false;
    result = true;
	
    if(!result)
    {
        cout << endl << "Writing Data to the UVC Extension is Failed" << endl;
        return false;
    }

	//Reads Data From the Device
    if(hid_fd < 0)
        result = false;

    ret = 0;

    struct timeval tv;
    fd_set rfds;

    FD_ZERO(&rfds);
    FD_SET(hid_fd, &rfds);

    //Wait up to 1 seconds. 
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    // Monitor read file descriptor for 1 secs
    if(0 > select(1, &rfds, NULL, NULL, &tv)){
      perror("select");
        return false;
    }

    ret = ::read(hid_fd, outputBuffer, BUFFERLENGTH);
    if((ret < 0) || (outputBuffer[0] != READFIRMWAREVERSION))
        result = false;
    else
        result = true;

    if(!result)
    {
        cout << endl << "Reading Data from the UVC Extension is Failed" << endl;
        return false;
    }

	int pMajorVersion, pMinorVersion1, pMinorVersion2, pMinorVersion3, SDK_VER, SVN_VER;

	SDK_VER = (outputBuffer[3] << 8)+outputBuffer[4];
    SVN_VER = (outputBuffer[5] << 8)+outputBuffer[6];
    pMajorVersion = outputBuffer[1];
    pMinorVersion1 = outputBuffer[2];
	
	pMinorVersion2 = SDK_VER;
    pMinorVersion3 = SVN_VER;

#elif _WIN32

	UINT8 pMajorVersion = 0;
	UINT8 pMinorVersion1 = 0;
	UINT16 pMinorVersion2 = 0;
	UINT16 pMinorVersion3 = 0;						

	BOOL result = readfirmwareversion(&pMajorVersion, &pMinorVersion1, &pMinorVersion2, &pMinorVersion3);
	if(!result)
	{
		cout << endl << "Reading Data from the UVC Extension is Failed" << endl;
		return false;
	}
	
#endif

	printf("\nFirmWareVersion Number = %d.%d.%d.%d \n\n" , pMajorVersion, pMinorVersion1, pMinorVersion2, pMinorVersion3);

	return true;
}


//HID Settings
//
bool hidProp()
{
	while(true)
	{
		int choice  = -1;
		cout << endl << '\t' << "0 - Exit" << endl;
        cout << '\t' << "1 - Back" << endl;
        cout << '\t' << "2 - Main Menu" << endl;
        cout << '\t' << "3 - GetFirmware Version Number" << endl;

        while((choice < 0) || (choice > 3))
        {
			printf("\n Pick a Relevant Choice to Configure Particular Camera Properties: \t");
			scanf("%d", &choice);
			while(getchar() != '\n' && getchar() != EOF)
			{	
			}
        }
		
		switch(choice)
		{
		case EXIT:
#ifdef _WIN32

			if(deinitextensionunit())
				t.detach();
			bSwitch = true;
			if(cap.isOpened())
				cap.release();

#elif __linux__

			bPreviewSet(1, false);
			if(closeHID())
                destroyAllWindows();

#endif			

			exit(0);		

		case 1:
		case 2:
			if(!(exploreCam()))
				cout << "Camera Exploration is Failed" << endl;
			break;
		
		case 3:
			if(!configExtUVCSettings())
            {
                cout << endl << "Extension UVC Settings Configuration is Failed" << endl;
                return false;
            }
            cout << endl << "Extension UVC Settings Configuration is Done" << endl;
            break;
		}
	}

	return true;
}


//Explore Camera Properties
//
bool exploreCam()
{
	while(true)
	{
		int choice = -1;
		cout << endl << '\t' << "0 - Exit" << endl;
		cout << '\t' << "1 - Back" << endl;
		cout << '\t' << "2 - Configure Camera Format/Resolution" << endl;
		cout << '\t' << "3 - Configure UVC Settings" << endl;
		cout << '\t' << "4 - Capture Still Images" << endl;
		
		if(bOpenHID)
		{
			cout << '\t' << "5 - HID Properties" << endl;
			while((choice < 0) || (choice > 5))
            {
				printf("\n Pick a Relevant Choice to Configure Particular Camera Properties : \t");
				scanf( "%d", &choice);
				while(getchar() != '\n' && getchar() != EOF)
				{	
				}
            }
		}
		else
		{
			while((choice < 0) || (choice >= 5))
            {
				printf("\n Pick a Relevant Choice to Configure Particular Camera Properties : \t");
				scanf( "%d", &choice);
				while(getchar() != '\n' && getchar() != EOF)
				{	
				}
            }
		}

		switch(choice)
		{
		case EXIT:
#ifdef _WIN32

			if(deinitextensionunit())
				t.detach();
			bSwitch = true;
			if(cap.isOpened())
				cap.release();
			
#elif __linux__

			bPreviewSet(1, false);
			if(closeHID())
				destroyAllWindows();

#endif

			exit(0);			

		case 1:
			if(!listDevices())
            {
                cout << endl << "List Devices Information failed" << endl;
                return false;
            }
            cout << endl << "Connected Devices were Listed" << endl;
            break;

		case 2:
			if(!configFormats())
            {
                cout << endl << "Format Configuration is Failed" << endl;
                return false;
            }
            cout << endl << "Format Configuration is done" << endl;
            break;

		case 3:
			if(!configUVCSettings())
            {
                cout << endl << "UVC Settings Configuration is Failed" << endl;
                return false;
            }
            cout << endl << "UVC Settings Configuration is Done" << endl;
            break;

		case 4:
			if(!captureStill())
            {
                cout << endl << "Still Capture Failed" << endl;
                return false;
            }
            cout << endl << "Still Capture is Done" << endl;
            break;

		case 5:
			if(!hidProp())
            {
                cout << endl << "HID Properties Configuration is Failed" << endl;
                return false;
            }
            cout << endl << "HID Properties Configuration is Done" << endl;
            break;
		}
	}
	
	return true;
}
