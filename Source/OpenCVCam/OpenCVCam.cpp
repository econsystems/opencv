#ifdef _WIN32
#pragma once
#define _CRT_SECURE_NO_WARNINGS   // to use scanf instead of scanf_s (scanf_s is not working in Linux).
#include <Windows.h>
#include <SDKDDKVer.h>
#include <tchar.h>
#include <string>
#include <conio.h>
#include <strsafe.h>

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
// #include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <libudev.h>

#define LOWORD(l) ((WORD)(l))
#define HIWORD(l) ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w) ((BYTE)(w))
#define HIBYTE(w) ((BYTE)(((WORD)(w) >> 8) & 0xFF))
#define WORD __uint16_t
#define DWORD __uint32_t
#define BYTE __uint8_t

#endif

#include <stdio.h>
#include <iostream>
#include <fstream>      // Added to Save buffer as .raw format. (CU55_MH).
#include <thread>
#include <vector>

using namespace std;
using namespace cv;

#define EXIT				0
#define AUTO				1
#define MANUAL				2
#define AUTOANDMANUAL			3
#define	READFIRMWAREVERSION		0X40
#define BUFFERLENGTH			65
#define SDK_VERSION			"1.0.5"

#ifdef _WIN32

#define PROPERTY			19

typedef BOOL(*Readfirmwareversion_t)(uint32_t* Handle, uint8_t* pMajorVersion, uint8_t* pMinorVersion1, uint16_t* pMinorVersion2, uint16_t* pMinorVersion3);
typedef BOOL(*Initextensionunit_t)(TCHAR* USBInstanceID, uint32_t** handle);
typedef BOOL(*Deinitextensionunit_t)(uint32_t* handle);
typedef BOOL(*Readfirmwareversion82USB_t) (uint32_t*, uint8_t *, uint8_t *, uint16_t *);
typedef BOOL(*ReadfirmwareversionTania_t) (uint32_t*, CHAR *);
typedef BOOL(*ReadfirmwareversionECAM22USB_t) (uint32_t*, uint8_t , uint8_t *);
typedef BOOL(*ReadfirmwareversionECAM51_t) (uint32_t* , uint8_t *);
typedef BOOL(*Readfirmwareversion83USB_t) (uint32_t*, uint8_t *, uint8_t *, uint16_t * , uint32_t*);
typedef BOOL(*InitextensionunitExt_t)(uint32_t** handle , TCHAR* USBInstanceID);

Initextensionunit_t initextensionunit;
InitextensionunitExt_t initextensionunitExt;
Deinitextensionunit_t deinitextensionunit;
Readfirmwareversion_t readfirmwareversion;
Readfirmwareversion83USB_t readfirmwareversion83USB;
ReadfirmwareversionECAM51_t readfirmwareversionECAM51;
ReadfirmwareversionECAM22USB_t readfirmwareversionECAM22;
ReadfirmwareversionTania_t readfirmwareversionTANIA;
Readfirmwareversion82USB_t readfirmwareversion82USB;


#elif __linux__

#define PROPERTY			17

#endif

#ifndef V4L2_CAP_META_CAPTURE
#define V4L2_CAP_META_CAPTURE    0x00800000  /* Specified in kernel header v4.16 */
#endif // V4L2_CAP_META_CAPTURE

//Variable Declarations
VideoCapture cap;
Mat Frame, BayerFrame8, IRImage, BGRImage, ResultImage, IRImage27CUG, RGBImage27CUG, IRImageCU83, RGBImageCU83, RGBStillImageCU83, IRStillImageCU83;
int IRBufferSizeCU83, RGBBufferSizeCU83, IRStillBufferSizeCU83, RGBStillBufferSizeCU83 = 0;
char keyPressed = '\0', dilemma = 'y';
int devices = 0, formats = 0, width = 0, curWidth = 0, curHeight = 0, height = 0, fps = 0;
int minimum = 0, maximum = 0, defaultValue = 0, currentValue = 0, steppingDelta = 0, supportedMode = 0, currentMode = 0, value = 0;
vector< pair <int, String> > uvcProperty;
unsigned char outputBuffer[BUFFERLENGTH], inputBuffer[BUFFERLENGTH];
String deviceName, vid, pid, devicePath, formatType, CurrFormatType;
bool bOpenHID = false, bCapture, bPreview, bSwitch, _12CUNIR, _CU51, _CU40, _10CUG_C, _CU55M, _20CUG, _CU135, _27CUG, _CU83, _50CUG_M, _CU83_H03R1, _37CUG, _16CUG, _512M, cameraswitch = false , IsExtDevice;
mutex mu;
uchar* Y12Buff, *Y16Buff, *StillBuff, *PixelBuff, *RGBIRBuff, *RGBBuffCU83, *IRBuffCU83, *IRBuffY8 = NULL; // PixelBuff is for _CU55M used to convert Y12 to Y8, initial resolution of the device is 640x480..
uchar* input27CugBuffer;
bool Y12Format = true, Y16Format = true, checkFormat = true, Y16Cu83Format = false, Y10Format = false;
int countValue, writeReturnValue = 0;
FILE *fp = NULL;
uint32_t* handle = nullptr;


#ifdef _WIN32

TCHAR *tDevicePath;
HINSTANCE hinstLib;
HINSTANCE hinstLibExt;
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

	if (tid == 1)
	{
		bCapture = bRead;
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}

	return bCapture;
}

bool bPreviewSet(int tid, bool bPrev)
{
	std::lock_guard<std::mutex> guard(mu);

	if (tid == 1)
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


// To get the Format string from Fourcc value. by Murali.
String DwordToFourCC(double fcc)
{
	char buffer[4] = { "" };

	buffer[3] = HIBYTE(HIWORD(fcc));
	buffer[2] = LOBYTE(HIWORD(fcc));
	buffer[1] = HIBYTE(LOWORD(fcc));
	buffer[0] = LOBYTE(LOWORD(fcc));

	return buffer;
}

// To get weather the format is raw supported or not
bool IsRawSaveSupport()
{
	String Format = DwordToFourCC(cap.get(CAP_PROP_FOURCC/*CV_CAP_PROP_FOURCC*/));

	if (Format.substr(0, 4) == "UYVY" && !_27CUG 
		|| Format.substr(0, 4) == "YUY2" 
		|| Format.substr(0, 2) == "Y8" 
		|| Format.substr(0, 4) == "YUYV"
		|| Format.substr(0,2) == "Y16")
		return true;
	return false;
}

// To get the current set format and Resolution,when application launches.
bool getCurrentFormat() {

	formatType = DwordToFourCC(cap.get(CAP_PROP_FOURCC/*CV_CAP_PROP_FOURCC*/));

	curWidth = Frame.cols;
	curHeight = Frame.rows;
#ifdef _WIN32
	if (formatType.substr(0, 3) == "Y12")
		Y12Format = true;
	else
		Y12Format = false;

	if (formatType.substr(0, 3) == "Y16")
		Y16Format = true;
	else
		Y16Format = false;

	if (formatType.substr(0, 3) == "Y10")
		Y10Format = true;
	else
		Y10Format = false;
#elif __linux__

	if (formatType.substr(0, 3) == "Y12") {
		Y12Format = true;
	}
	else if (formatType.substr(0, 3) == "Y16") {
		Y16Format = true;
		Y16Cu83Format = true;
	}
	else if (formatType.substr(0, 3) == "Y10") {
		Y10Format = true;
	}
	else {
		Y10Format = false;
		Y12Format = false;
		Y16Format = false;
		Y16Cu83Format = false;
	}
#endif

	if ((_CU55M || _50CUG_M || _37CUG || _512M))
		PixelBuff = new uchar[curWidth * curHeight];

	/** cout<<"\n getCurrentFormat Completed"; **/
	return true;
}

/** Converting Y12 format to Y8 for See3CAM_CU55M.
* Y12 format is decoded with 3 bytes for 2 pixels.
* Need to skip the 3rd byte everytime in order to get a byte per pixel
**/

bool ConvertY12toY8(Mat &Y12Mat, Mat &Y8Buff)
{
	int m = 0;
	Y12Buff = Y12Mat.data;
	for (int i = 0; i < Y12Mat.rows; i++)
	{
		for (int j = 0; j < Y12Mat.cols; j += 2)
		{
			PixelBuff[i*Y12Mat.cols + j] = Y12Buff[m];
			PixelBuff[i*Y12Mat.cols + j + 1] = Y12Buff[m + 1];
			m += 3;
		}
	}

	Y8Buff = Mat(Y12Mat.rows, Y12Mat.cols, CV_8UC1, PixelBuff);  // convert to Mat from buffer
	return true;
}

/** Actual Data format is Y12 (12 bit, after conversion Y12 (16 bit).
* Y12 format is decoded with 3 bytes for 2 pixels.
* For still capture need to padding 4 bits (i.e. 2 bytes for 1 pixel).
**/

bool ConvertY12forStill(Mat &Y12Mat, uchar* Y12StillBuff)
{
	uchar* DataBuff = Y12Mat.data;      // Conversion from Mat data to Array.
	uchar bPixel1 = 0, bPixel2 = 0;
	int i = 0;
	int m = 0;
	// Looping with Output (Dest.) Buffer
	int stride = Y12Mat.cols * 2;
	for (int j = 0; j < Y12Mat.rows; j++)
	{
		for (int i = 0; i < stride; i += 4)
		{
			Y12StillBuff[(stride * j) + i + 1] = ((0XF0 & DataBuff[m]) >> 4);

			bPixel1 = (DataBuff[m] & 0X0F);
			bPixel2 = (DataBuff[m + 2] & 0X0F);
			bPixel1 = (bPixel1 << 4);

			Y12StillBuff[(stride * j) + i] = bPixel1 + bPixel2;

			Y12StillBuff[(stride * j) + i + 3] = ((0XF0 & DataBuff[m + 1]) >> 4);

			bPixel1 = (DataBuff[m + 1] & 0X0F);
			bPixel2 = (DataBuff[m + 2] & 0XF0);
			bPixel1 = (bPixel1 << 4);
			bPixel2 = (bPixel2 >> 4);

			Y12StillBuff[(stride * j) + i + 2] = bPixel1 + bPixel2;

			m += 3;
		}
	}
	return true;
}

// Write frame data into raw file
void SaveInRAW(uchar* Buffer, char* buf, int FrameSize)
{
	ostringstream filename;
	filename << buf;
	std::ofstream outfile(filename.str().c_str(), ios::out | ios::binary);
	outfile.write((char*)(Buffer), FrameSize);
	outfile.close();
	return;
}

/** Splitting RGB and IR Frames for 27CUG
**/

bool ConvertRGBIR(Mat Frame, Mat &RGBImage, Mat &IRImage)
{
	RGBIRBuff = Frame.data;

	if (RGBIRBuff)
	{
		IRImage.release();
		RGBImage.release();
		if (RGBIRBuff[7] == 0)
		{
			RGBImage = Mat(Frame.rows, Frame.cols, CV_8UC2, RGBIRBuff);
		}
		else
		{
			IRImage = Mat(Frame.rows, Frame.cols, CV_16UC1, RGBIRBuff);  // convert to Mat from buffer
		}
	}
	return true;
}
/**
Separating RGBIR Buffer for Preview in See3CAM_CU83.
In preview we are skipping the 5th byte
**/
bool SeparatingRGBIRBuffers(Mat Frame, Mat* IRImageCU83, Mat* RGBImageCU83, int *RGBBufferSizeCU83, int *IRBufferSizeCU83)
{
	RGBIRBuff = Frame.data;
	int long size = Frame.cols*Frame.rows * 2;

	int Buffcnt = 0;
	int cnt = 0;
	int RGBBufsize = 0;
	//Changed the datatype from UINT32 to uint32_t - commented by Sushanth
	//Reason - compatible for both linux & windows
	uint32_t IrBufsize = 0;
	int rgbcnt = 0, Ircnt = 0;
	BYTE * IRBuff = NULL;
	IRBuff = (BYTE*)malloc(1920 * 1080 * 2);
	int long BuffSize = 3120 * 1080 * 2;

	// Updated the seperation logic to support 3120x1080 resolution - commented by Adithya TS
	if ((Frame.cols == 3120 && Frame.rows == 1080))
	{
		int rgbbuffsize = 1920 * 1080 * 2;
		int irbuffsize = 2592000;

		while (BuffSize > 0)
		{
			if ((RGBIRBuff[Buffcnt] & 0x03) == 0x00)
			{
				memcpy(RGBImageCU83->data + (RGBBufsize), RGBIRBuff + Buffcnt, 3839);
				Buffcnt += 3840;
				RGBBufsize += 3840;
				BuffSize -= 3840;
				rgbcnt += 1;
			}
			else if ((RGBIRBuff[Buffcnt] & 0x03) == 0x03)
			{
				memcpy(IRBuff + (IrBufsize), RGBIRBuff + Buffcnt, 2399);
				IrBufsize += 2400;
				Buffcnt += 2400;
				BuffSize -= 2400;
				Ircnt += 1;
			}
			else
			{
				return 0;
			}
			cnt++;
		}
	}
	else
	{
		while (size > 0)
		{
			if ((RGBIRBuff[Buffcnt] & 0x03) == 0x00)
			{
				memcpy(RGBImageCU83->data + (RGBBufsize), RGBIRBuff + Buffcnt, 7679);
				Buffcnt += 7680;
				RGBBufsize += 7680;
				size -= 7680;
				rgbcnt += 1;
			}
			else if ((RGBIRBuff[Buffcnt] & 0x03) == 0x03)
			{
				memcpy(IRBuff + (IrBufsize), RGBIRBuff + Buffcnt, 2399);
				IrBufsize += 2400;
				Buffcnt += 2400;
				size -= 2400;
				Ircnt += 1;
			}
			else
			{
				return 0;
			}
			cnt++;
		}
	}

	int bufsize_IR = 0;
	Buffcnt = 0;
	while (IrBufsize > 0)
	{
		memcpy(IRImageCU83->data + (bufsize_IR), IRBuff + Buffcnt, 4);
		bufsize_IR += 4;
		Buffcnt += 5;
		IrBufsize -= 5;
	}
	Buffcnt = 0;

	*RGBBufferSizeCU83 = RGBBufsize;
	*IRBufferSizeCU83 = IrBufsize;
	free(IRBuff);
	// IRBuff = NULL; //Added by Sushanth - Assigning IRBuff to NULL when it is freed.
	return 1; 
}

/**
Separating RGBIR Buffer for StillCapture in See3CAM_CU83.
**/
bool SeparatingRGBIRBufferStillCapture(Mat Frame, Mat* IRImageCU83, Mat* RGBImageCU83, int *RGBBufferSizeCU83, int *IRBufferSizeCU83)
{
	RGBIRBuff = Frame.data;
	int long size = Frame.cols*Frame.rows * 2;
	int Buffcnt = 0;
	int cnt = 0;
	int RGBBufsize = 0;
	//Changed the datatype from UINT32 to uint32_t - commented by Sushanth
	//Reason - compatible for both linux & windows
	uint32_t IrBufsize = 0;
	int ir = 0;
	int rgbcnt = 0, Ircnt = 0;
	BYTE * IRBuff = NULL;
	IRBuff = (BYTE*)malloc(1920 * 1080 * 2);
	int long BuffSize = 3120 * 1080 * 2;

	if ((Frame.cols == 3120 && Frame.rows == 1080))
	{
		int rgbbuffsize = 1920 * 1080 * 2;
		int irbuffsize = 2592000;

		while (BuffSize > 0)
		{
			if ((RGBIRBuff[Buffcnt] & 0x03) == 0x00)
			{
				memcpy(RGBImageCU83->data + (RGBBufsize), RGBIRBuff + Buffcnt, 3839);
				Buffcnt += 3840;
				RGBBufsize += 3840;
				BuffSize -= 3840;
				rgbcnt += 1;
			}
			else if ((RGBIRBuff[Buffcnt] & 0x03) == 0x03)
			{
				memcpy(IRBuff + (IrBufsize), RGBIRBuff + Buffcnt, 2399);
				IrBufsize += 2400;
				Buffcnt += 2400;
				BuffSize -= 2400;
				Ircnt += 1;
			}
			else
			{
				return 0;
			}
			cnt++;
		}
	}
	else {

		while (size > 0)
		{
			if ((RGBIRBuff[Buffcnt] & 0x03) == 0)
			{
				memcpy(RGBImageCU83->data + (RGBBufsize), RGBIRBuff + Buffcnt, 7679);
				Buffcnt += 7680;
				RGBBufsize += 7680;
				size -= 7680;
				rgbcnt += 1;
			}
			else if ((RGBIRBuff[Buffcnt] & 0x03) == 0x03)
			{
				memcpy(IRImageCU83->data + (IrBufsize), RGBIRBuff + Buffcnt, 2399);
				IrBufsize += 2400;
				Buffcnt += 2400;
				size -= 2400;
				Ircnt += 1;
			}
			else
			{
				return 0;
			}
			cnt++;
		}
	}
	Buffcnt = 0;
	*RGBBufferSizeCU83 = RGBBufsize;
	*IRBufferSizeCU83 = IrBufsize;
	return 1;
}
/** Converting RAW10 format to RAW8 in See3CAM_CU83 for preview.
* Need to skip the 5th byte everytime in order to get a byte per pixel
**/
bool ConvertRAW10toRAW8(Mat pBufIn, Mat pBufOut)
{
	int cnt = 0;
	int Buffcnt = 0;
	//Changed the datatype from UINT32 to uint32_t - commented by Sushanth
	//Reason - compatible for both linux & windows
	uint32_t IrBufsize = 0;
	int bufsize_IR = 0;
	Buffcnt = 0;
	size_t size = (pBufIn.elemSize1() * pBufIn.total() * pBufIn.channels());
	while (size > 0)
	{
		memcpy(pBufOut.data + (bufsize_IR), pBufIn.data + Buffcnt, 4);
		bufsize_IR += 4;
		Buffcnt += 5;
		size -= 5;
	}
	//Changed TRUE to true - commented by Sushanth
	//Reason - compatible for both linux & windows
	return true;
}
/** Converting RAW10 format to RAW8 in See3CAM_CU83 for Still capture.
**/
//Changed BOOL to bool
//Reason - compatible for both linux & windows
bool RAW10ConversionStillCU83(BYTE *pBuffer, Mat *m_RAWData10Bit, DWORD sizeInputBuffer)
{
	long int file_size = 0, read_file_itr = 0, write_file_itr = 0;
	//CONVERSION
	for (read_file_itr = 0; read_file_itr <= (sizeInputBuffer); read_file_itr = read_file_itr + 5, write_file_itr = write_file_itr + 8)
	{
		*(m_RAWData10Bit->data + write_file_itr + 0) = ((*(pBuffer + read_file_itr + 0)) << 2) | (((*(pBuffer + read_file_itr + 4)) & 0xC0) >> 6);
		*(m_RAWData10Bit->data + write_file_itr + 1) = ((*(pBuffer + read_file_itr + 0)) >> 6)  & (0x03);

		*(m_RAWData10Bit->data + write_file_itr + 2) = ((*(pBuffer + read_file_itr + 1)) << 2) | (((*(pBuffer + read_file_itr + 4)) & 0x30) >> 4);
		*(m_RAWData10Bit->data + write_file_itr + 3) = ((*(pBuffer + read_file_itr + 1)) >> 6)  & (0x03);

		*(m_RAWData10Bit->data + write_file_itr + 4) = ((*(pBuffer + read_file_itr + 2)) << 2) | (((*(pBuffer + read_file_itr + 4)) & 0x0C) >> 2);
		*(m_RAWData10Bit->data + write_file_itr + 5) = ((*(pBuffer + read_file_itr + 2)) >> 6)  & (0x03);

		*(m_RAWData10Bit->data + write_file_itr + 6) = ((*(pBuffer + read_file_itr + 3)) << 2) | (((*(pBuffer + read_file_itr + 4)) & 0x03));
		*(m_RAWData10Bit->data + write_file_itr + 7) = ((*(pBuffer + read_file_itr + 3)) >> 6)  & (0x03);

	}
	return true;
}

#ifdef _WIN32

//Preview Window for Windows
//
void stream()
{
	bCapture = true;
	while (true)
	{
		while (bPreviewSet(2, true))
		{			
			if (bReadSet(2, true))
			{
				cap >> Frame;
			}
			
			if (!Frame.empty())
			{
				if (cameraswitch == true)
				{
					cameraswitch = false;
					cv::destroyAllWindows();					
				}
				if (checkFormat) {  // Call getCurrentFormat API,where we will get the initial set format and Resolution.
					getCurrentFormat();
					checkFormat = false;  // Make it false,as it is used only once,just to get the initial set values.
				}

				else if ((_12CUNIR) || (_CU51))
				{
					//Convert to 8 Bit:
					//Scale the 12 Bit (4096) Pixels into 8 Bit(255) (255/4096)= 0.06226
					convertScaleAbs(Frame, ResultImage, 0.06226);

					namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					imshow("OpenCVCam", ResultImage);
				}
				else if (_CU40)
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
				else if (_27CUG)
				{
					ConvertRGBIR(Frame, RGBImage27CUG, IRImage27CUG);

					if (!RGBImage27CUG.empty())
					{
						cvtColor(RGBImage27CUG, ResultImage, COLOR_YUV2BGR_UYVY);
						namedWindow("OpenCVCam RGB Frame", WINDOW_AUTOSIZE);
						imshow("OpenCVCam RGB Frame", ResultImage);
					}
					else if (!IRImage27CUG.empty())
					{
						namedWindow("OpenCVCam IR Frame", WINDOW_AUTOSIZE);
						imshow("OpenCVCam IR Frame", IRImage27CUG);
					}
				}
				else if ((_CU83 && Y16Format) || (_CU83_H03R1 && Y16Format))
				{
					if (Frame.cols == 4440 && Frame.rows == 2160)
					{
						RGBImageCU83 = Mat(2160, 3840, CV_8UC2); //allocation
						IRImageCU83 = Mat(1080, 1920, CV_8UC1);
						if (SeparatingRGBIRBuffers(Frame, &IRImageCU83, &RGBImageCU83, &RGBBufferSizeCU83, &IRBufferSizeCU83) == 1)
						{
							if (!RGBImageCU83.empty())
							{
								cvtColor(RGBImageCU83, ResultImage, COLOR_YUV2BGR_UYVY);
								namedWindow("OpenCVCam RGB Frame", WINDOW_AUTOSIZE);
								imshow("OpenCVCam RGB Frame", ResultImage);
							}
							if (!IRImageCU83.empty())
							{
								namedWindow("OpenCVCam IR Frame", WINDOW_AUTOSIZE);
								imshow("OpenCVCam IR Frame", IRImageCU83);
							}
						}
					}
					else
					{
						if (Frame.cols == 3840 && Frame.rows == 1350)
						{
							size_t size = (Frame.elemSize1() * Frame.total() * Frame.channels());
							IRImageCU83 = Mat(2160, 3840, CV_8UC1);
							ConvertRAW10toRAW8(Frame, IRImageCU83);
							namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
							imshow("OpenCVCam", IRImageCU83);
						}
						else if ((Frame.cols == 1920 && Frame.rows == 675))
						{
							size_t size = (Frame.elemSize1() * Frame.total() * Frame.channels());
							IRImageCU83 = Mat(1080, 1920, CV_8UC1);
							ConvertRAW10toRAW8(Frame, IRImageCU83);
							namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
							imshow("OpenCVCam", IRImageCU83);
						}
						else if ((Frame.cols == 3120 && Frame.rows == 1080))
						{
							RGBImageCU83 = Mat(1080, 1920, CV_8UC2); //allocation
							IRImageCU83 = Mat(1080, 1920, CV_8UC1);
							SeparatingRGBIRBuffers(Frame, &IRImageCU83, &RGBImageCU83, &RGBBufferSizeCU83, &IRBufferSizeCU83);

							if (!RGBImageCU83.empty())
							{
								cvtColor(RGBImageCU83, ResultImage, COLOR_YUV2BGR_UYVY);
								namedWindow("OpenCVCam RGB Frame", WINDOW_AUTOSIZE);
								imshow("OpenCVCam RGB Frame", ResultImage);
							}
							if (!IRImageCU83.empty())
							{
								namedWindow("OpenCVCam IR Frame", WINDOW_AUTOSIZE);
								imshow("OpenCVCam IR Frame", IRImageCU83);
							}
						}
					}
				}
				else if ((_CU55M || _50CUG_M || _37CUG || _512M) && Y12Format)
				{
					ConvertY12toY8(Frame, ResultImage);
					namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					imshow("OpenCVCam", ResultImage);
				}
				else if ((_CU135 || _20CUG || _16CUG) && (Y16Format)) // included _CU135
				{
					//Scale the 10 Bit (1024) Pixels into 8 Bit(255) (255/1024)= 0.2490234375
					convertScaleAbs(Frame, ResultImage, 0.2490234375);
					namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					imshow("OpenCVCam", ResultImage);
				}

				//Add support for Y10 format - Input file size is Y16 frame, so it should render as Y16 frame.
				else if (formatType.substr(0, 4) == "Y10 ") {
					convertScaleAbs(Frame, ResultImage, 0.2490234375);
					namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					imshow("OpenCVCam", ResultImage);
				}
				// added for UYVY format stream with appropriate conversions
				else if (formatType.substr(0, 4) == "UYVY")
				{
					//std::cout << "Format Type (first 4 chars): " << formatType.substr(0, 4) << std::endl;

					cvtColor(Frame, ResultImage, COLOR_YUV2BGR_UYVY);
					namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					imshow("OpenCVCam", ResultImage);
				}
				// added for YUY2 format stream with appropriate conversions
				else if (formatType.substr(0, 4) == "YUY2")
				{
					cvtColor(Frame, ResultImage, COLOR_YUV2BGR_YUY2);
					namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					imshow("OpenCVCam", ResultImage);
				}
				else
				{
					//std::cout << "Format Type (first 4 chars): " << formatType.substr(0, 4) << std::endl;

					if (Frame.channels() == 2) 
					{
						//std::cout << "Number of channels in the frame ++: " << Frame.channels() << std::endl;
						if (formatType.substr(0, 4) == "YUY2")
						{
							//std::cout << "Number of channels in the frame 3: " << Frame.channels() << std::endl;
							cvtColor(Frame, ResultImage, COLOR_YUV2BGR_YUY2);
						}
						else if (formatType.substr(0, 4) == "UYVY")
						{
							//std::cout << "Number of channels in the frame 4: " << Frame.channels() << std::endl;
							cvtColor(Frame, ResultImage, COLOR_YUV2BGR_UYVY);
						}
					
						namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
						imshow("OpenCVCam", ResultImage);
						
					}
					else
					{
						//std::cout << "Number of channels in the frame --: " << Frame.channels() << std::endl;
						namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
						imshow("OpenCVCam", Frame);
					}				
				}
			}

			keyPressed = waitKey(10);
			while (bSwitch)
			{
				bSwitch = false;
				destroyAllWindows();
			}
		}
	}
}

#elif __linux__

bool closeHID()
{
	if (hid_fd > 0)
		close(hid_fd);
	return true;
}

//Preview Window for Linux

void *preview(void *arg)
{
	bCapture = true;
	while (true)
	{
		while (bPreviewSet(2, true))
		{
			if (bReadSet(2, true))
			{
				cap >> Frame;
			}

			if (!Frame.empty())
			{
				if (checkFormat) {  // Call getCurrentFormat API,where we will get the initial set format and Resolution.
					getCurrentFormat();
					checkFormat = false;  // Make it false,as it is used only once,just to get the initial set values.
				}
				if ((_12CUNIR) || (_CU51))
				{
					//Convert to 8 Bit:
					//Scale the 12 Bit (4096) Pixels into 8 Bit(255) (255/4096)= 0.06226
					convertScaleAbs(Frame, ResultImage, 0.06226);
					namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					imshow("OpenCVCam", ResultImage);
				}
				else if (_CU40)
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
				else if (_27CUG)
				{
					ConvertRGBIR(Frame, RGBImage27CUG, IRImage27CUG);

					if (!RGBImage27CUG.empty())
					{
						cvtColor(RGBImage27CUG, ResultImage, COLOR_YUV2BGR_UYVY);
						namedWindow("OpenCVCam RGB Frame", WINDOW_AUTOSIZE);
						imshow("OpenCVCam RGB Frame", ResultImage);
					}
					else if (!IRImage27CUG.empty())
					{
						namedWindow("OpenCVCam IR Frame", WINDOW_AUTOSIZE);
						imshow("OpenCVCam IR Frame", IRImage27CUG);
					}
				}
				else if (_CU83 && Y16Format || (_CU83_H03R1 && Y16Format))
				{
					struct timeval  tv, m_tv, res_time;

					if (Frame.cols == 4440 && Frame.rows == 2160)
					{
						if (RGBImageCU83.empty())
						{
							RGBImageCU83 = Mat(2160, 3840, CV_8UC2); //allocation
						}
						if (IRImageCU83.empty())
						{
							IRImageCU83 = Mat(1080, 1920, CV_8UC1);
						}

						gettimeofday(&m_tv, NULL);

						SeparatingRGBIRBuffers(Frame, &IRImageCU83, &RGBImageCU83, &RGBBufferSizeCU83, &IRBufferSizeCU83);

						gettimeofday(&tv, NULL);
						timersub(&tv, &m_tv, &res_time);
						//cout << "Time taken (in ms) by PrepareCU83Buffer() :" << ((int)((res_time.tv_sec * 1000) + (res_time.tv_usec / 1000)));

						if (!RGBImageCU83.empty())
						{
							cvtColor(RGBImageCU83, ResultImage, COLOR_YUV2BGR_UYVY);
							namedWindow("OpenCVCam RGB Frame", WINDOW_AUTOSIZE);
							imshow("OpenCVCam RGB Frame", ResultImage);
						}
						if (!IRImageCU83.empty())
						{
							namedWindow("OpenCVCam IR Frame", WINDOW_AUTOSIZE);
							imshow("OpenCVCam IR Frame", IRImageCU83);
						}
					}
					else
					{
						if (Frame.cols == 3840 && Frame.rows == 1350)
						{
							int size = (Frame.elemSize1() * Frame.total() * Frame.channels());
							IRImageCU83 = Mat(2160, 3840, CV_8UC1);
							ConvertRAW10toRAW8(Frame, IRImageCU83);
							namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
							imshow("OpenCVCam", IRImageCU83);
						}
						else if ((Frame.cols == 1920 && Frame.rows == 675))
						{
							int size = (Frame.elemSize1() * Frame.total() * Frame.channels());
							IRImageCU83 = Mat(1080, 1920, CV_8UC1);
							ConvertRAW10toRAW8(Frame, IRImageCU83);
							namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
							imshow("OpenCVCam", IRImageCU83);
						}
						else if ((Frame.cols == 3120 && Frame.rows == 1080))
						{
							RGBImageCU83 = Mat(1080, 1920, CV_8UC2); //allocation
							IRImageCU83 = Mat(1080, 1920, CV_8UC1);
							SeparatingRGBIRBuffers(Frame, &IRImageCU83, &RGBImageCU83, &RGBBufferSizeCU83, &IRBufferSizeCU83);

							if (!RGBImageCU83.empty())
							{
								cvtColor(RGBImageCU83, ResultImage, COLOR_YUV2BGR_UYVY);
								namedWindow("OpenCVCam RGB Frame", WINDOW_AUTOSIZE);
								imshow("OpenCVCam RGB Frame", ResultImage);
							}
							if (!IRImageCU83.empty())
							{
								namedWindow("OpenCVCam IR Frame", WINDOW_AUTOSIZE);
								imshow("OpenCVCam IR Frame", IRImageCU83);
							}
						}
					}
					cout << "\n ConverstionDone";
				}
				else if (_10CUG_C) //10CUG and other camera's
				{
					cvtColor(Frame, BGRImage, COLOR_BayerGB2BGR);
					namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					imshow("OpenCVCam", BGRImage);
				}
				else if ((_CU55M || _50CUG_M || _37CUG || _512M) && Y12Format) {    // Will allow for the Conversion only if format is Y12.Skip conversion if it is Y8.
					ConvertY12toY8(Frame, ResultImage);
					namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					imshow("OpenCVCam", ResultImage);
				}
				else if ((_CU135 || _20CUG || _16CUG) && (Y16Format)) // included _CU135
				{
					//Scale the 10 Bit (1024) Pixels into 8 Bit(255) (255/1024)= 0.2490234375
					convertScaleAbs(Frame, ResultImage, 0.2490234375);
					namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					imshow("OpenCVCam", ResultImage);
				} //Add support for Y10 format - Input file size is Y16 frame, so it should render as Y16 frame.
				else if (formatType.substr(0, 4) == "Y10 ") {
					convertScaleAbs(Frame, ResultImage, 0.2490234375);
					namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					imshow("OpenCVCam", ResultImage);
				}
				// #ifdef _WIN32
				// added for UYVY format stream with appropriate conversions
				else if (formatType.substr(0, 4) == "UYVY")
				{
					// cvtColor(Frame, ResultImage, COLOR_YUV2BGR_UYVY);
					// namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					// imshow("OpenCVCam", ResultImage);

					if (Frame.channels() == 2) 
					{
						//std::cout << "Number of channels in the frame 2: " << Frame.channels() << std::endl;
						if (formatType.substr(0, 4) == "YUY2")
						{
							//std::cout << "Number of channels in the frame 3: " << Frame.channels() << std::endl;
							cvtColor(Frame, ResultImage, COLOR_YUV2BGR_YUY2);
						}
						else if (formatType.substr(0, 4) == "UYVY")
						{
							//std::cout << "Number of channels in the frame 4: " << Frame.channels() << std::endl;
							cvtColor(Frame, ResultImage, COLOR_YUV2BGR_UYVY);
						}

						//std::cout << "Number of channels in the frame 5: " << Frame.channels() << std::endl;
						namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
						imshow("OpenCVCam", ResultImage);
					}
					else
					{
						//std::cout << "Number of channels in the frame 0: " << Frame.channels() << std::endl;
						namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
						imshow("OpenCVCam", Frame);
					}		
				}
				// added for YUY2 format stream with appropriate conversions
				else if (formatType.substr(0, 4) == "YUY2")
				{
					cvtColor(Frame, ResultImage, COLOR_YUV2BGR_YUY2);
					namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					imshow("OpenCVCam", ResultImage);
				}
				// #endif
				else
				{
					namedWindow("OpenCVCam", WINDOW_AUTOSIZE);
					imshow("OpenCVCam", Frame);
				}
			}
			else {
				cout << "\n EmptyFrame";
			}


			keyPressed = waitKey(5);

			while (bSwitch)
			{
				bSwitch = false;
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
	cout << endl << "\t" << "OpenCV SDK-Version = " << SDK_VERSION << "\n\n";

#ifdef _WIN32
	hinstLib = LoadLibrary(L"eCAMFwSw.dll");
	if (hinstLib == NULL)
	{
		cout << "The eCAMFwSw.dll is not loaded properly" << endl;
		return 0;
	}
	hinstLibExt = LoadLibrary(L"eCAMFwExt.dll");
	if (hinstLibExt == NULL)
	{
		cout << "The eCAMFwExt.dll is not loaded properly" << endl;
		return 0;
	}

#endif

	//Open a Camera Device
	if (!(listDevices()))
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

	if (!(exploreCam()))
	{
		cout << endl << "Camera Exploration Failed" << endl;
		return 0;
	}

#ifdef _WIN32
	if (bDetach)
		t.detach();
#endif

	if (cap.isOpened())
	{
		cap.release();
	}

	return 0;
}


//Listing the Devices

bool listDevices()
{
	//// added by adithya for clearing the frames before switching another device
	

	checkFormat = true;
	int camId = -1;
	//List total Number of Devices
#ifdef __linux__
	bSwitch = true;
#endif
	bSwitch = true;
	if (!(cap.getDevices(devices)))
	{
		cout << endl << "Get total number of devices Failed" << endl;
		return false;
	}

	if (devices < 0)
	{
		cout << endl << "No Camera Devices Connected to the port" << endl;
		return false;
	}

	cout << endl << "Number of Camera Devices Connected to the Port : " << devices << endl;
	cout << endl << "Camera Devices Connected to the PC Port : " << endl << endl;
	cout << '\t' << "0 - Exit" << endl;

	//List the Camera Names
	for (int eachDevice = 0; eachDevice < devices; eachDevice++)
	{
		if (!(cap.getDeviceInfo(eachDevice, deviceName, vid, pid, devicePath)))
		{
			cout << endl << "Device " << eachDevice << " Information couldn't be Retrieved" << endl;
		}

		cout << '\t' << eachDevice + 1 << " . " << deviceName << endl;
	}

	while ((camId < 0) || (camId > devices))
	{
		printf("\n Pick a Camera Device to Explore : \t");
		scanf("%d", &camId);
		while (getchar() != '\n' && getchar() != EOF)
		{

		}
	}

	switch (camId)
	{
	case EXIT:
	
#ifdef _WIN32

		if (handle != NULL)
		{
			if (deinitextensionunit(handle))
			{

			}
			handle = nullptr;
		}

		if (bDetach)
			t.detach();

		bSwitch = true;
		if (cap.isOpened())
			cap.release();

#elif __linux__

		bPreviewSet(1, false);
		if (closeHID())
			destroyAllWindows();

#endif

		exit(0);
		break;

	default:
		bPreviewSet(1, false);
		cap.getDeviceInfo((camId - 1), deviceName, vid, pid, devicePath);
		if ((vid == "2560") && (pid == "c1d8" || pid == "c1d7" || pid == "c0d7")) // condition added for CU135 devices
		{
			_CU40 = false;
			_CU51 = false;
			_12CUNIR = false;
			_10CUG_C = false;
			_CU55M = false;
			_20CUG = false;
			_CU135 = true;
			_27CUG = false;
			_CU83 = false;
			_50CUG_M = false;
			_CU83_H03R1 = false;
			_37CUG = false;
			_16CUG = false;
			_512M = false;
			IsExtDevice = false;
		}
		else if ((vid == "2560") && (pid == "c140"))
		{
			_CU40 = true;
			_CU51 = false;
			_12CUNIR = false;
			_10CUG_C = false;
			_CU55M = false;
			_20CUG = false;
			_CU135 = false;
			_27CUG = false;
			_CU83 = false;
			_50CUG_M = false;
			_CU83_H03R1 = false;
			_37CUG = false;
			_16CUG = false;
			_512M = false;
			IsExtDevice = false;
		}
		else if ((vid == "2560") && (pid == "c152"))
		{
			_CU40 = false;
			_CU51 = true;
			_12CUNIR = false;
			_10CUG_C = false;
			_CU55M = false;
			_20CUG = false;
			_CU135 = false;
			_27CUG = false;
			_CU83 = false;
			_50CUG_M = false;
			_CU83_H03R1 = false;
			_37CUG = false;
			_16CUG = false;
			_512M = false;
			IsExtDevice = false;
		}
		else if ((vid == "2560") && (pid == "c113"))
		{
			_CU40 = false;
			_CU51 = false;
			_12CUNIR = true;
			_10CUG_C = false;
			_CU55M = false;
			_20CUG = false;
			_CU135 = false;
			_27CUG = false;
			_CU83 = false;
			_50CUG_M = false;
			_CU83_H03R1 = false;
			_37CUG = false;
			_16CUG = false;
			_512M = false;
			IsExtDevice = false;
		}
		else if ((vid == "2560") && (pid == "c111"))
		{
			_CU40 = false;
			_CU51 = false;
			_12CUNIR = false;
			_10CUG_C = true;
			_CU55M = false;
			_20CUG = false;
			_CU135 = false;
			_27CUG = false;
			_CU83 = false;
			_50CUG_M = false;
			_CU83_H03R1 = false;
			_37CUG = false;
			_16CUG = false;
			_512M = false;
			IsExtDevice = false;
		}
		else if ((vid == "2560") && (pid == "c155"))
		{
			_CU40 = false;
			_CU51 = false;
			_12CUNIR = false;
			_10CUG_C = false;
			_CU55M = true;
			_20CUG = false;
			_CU135 = false;
			_27CUG = false;
			_CU83 = false;
			_50CUG_M = false;
			_CU83_H03R1 = false;
			_37CUG = false;
			_16CUG = false;
			_512M = false;
			IsExtDevice = false;
		}
		else if ((vid == "2560") && (pid == "c124"))
		{
			_CU40 = false;
			_CU51 = false;
			_12CUNIR = false;
			_10CUG_C = false;
			_CU55M = false;
			_20CUG = true;
			_CU135 = false;
			_27CUG = false;
			_CU83 = false;
			_50CUG_M = false;
			_CU83_H03R1 = false;
			_37CUG = false;
			_16CUG = false;
			_512M = false;
			IsExtDevice = false;
		}
		else if ((vid == "2560") && (pid == "0121"))
		{
			_CU40 = false;
			_CU51 = false;
			_12CUNIR = false;
			_10CUG_C = false;
			_CU55M = false;
			_20CUG = false;
			_CU135 = false;
			_27CUG = true;
			_CU83 = false;
			_50CUG_M = false;
			_CU83_H03R1 = false;
			_37CUG = false;
			_16CUG = false;
			_512M = false;
			IsExtDevice = false;
		}
		else if ((vid == "2560") && (pid == "c188"))
		{
			_CU40 = false;
			_CU51 = false;
			_12CUNIR = false;
			_10CUG_C = false;
			_CU55M = false;
			_20CUG = false;
			_CU135 = false;
			_27CUG = false;
			_CU83 = true;
			_50CUG_M = false;
			_CU83_H03R1 = false;
			_37CUG = false;
			_16CUG = false;
			_512M = false;
			IsExtDevice = false;
		}
		else if ((vid == "2560") && (pid == "c157"))
		{
			_CU40 = false;
			_CU51 = false;
			_12CUNIR = false;
			_10CUG_C = false;
			_CU55M = false;
			_20CUG = false;
			_CU135 = false;
			_27CUG = false;
			_CU83 = false;
			_50CUG_M = true;
			_CU83_H03R1 = false;
			_37CUG = false;
			_16CUG = false;
			_512M = false;
			IsExtDevice = false;
		}
		else if ((vid == "2560") && (pid == "c18d"))
		{
			_CU40 = false;
			_CU51 = false;
			_12CUNIR = false;
			_10CUG_C = false;
			_CU55M = false;
			_20CUG = false;
			_CU135 = false;
			_27CUG = false;
			_CU83 = false;
			_50CUG_M = false;
			_CU83_H03R1 = true;
			_37CUG = false;
			_16CUG = false;
			_512M = false;
			IsExtDevice = false;
		}
		else if ((vid == "2560") && (pid == "c13a"))
		{
			_CU40 = false;
			_CU51 = false;
			_12CUNIR = false;
			_10CUG_C = false;
			_CU55M = false;
			_20CUG = false;
			_CU135 = false;
			_27CUG = false;
			_CU83 = false;
			_50CUG_M = false;
			_CU83_H03R1 = false;
			_37CUG = true;
			_16CUG = false;
			_512M = false;
			IsExtDevice = false;
		}
		else if ((vid == "2560") && (pid == "c117"))
		{
			_CU40 = false;
			_CU51 = false;
			_12CUNIR = false;
			_10CUG_C = false;
			_CU55M = false;
			_20CUG = false;
			_CU135 = false;
			_27CUG = false;
			_CU83 = false;
			_50CUG_M = false;
			_CU83_H03R1 = false;
			_37CUG = false;
			_16CUG = true;
			_512M = false;
		}
		else if ((vid == "2560") && (pid == "c158"))
		{
			_CU40 = false;
			_CU51 = false;
			_12CUNIR = false;
			_10CUG_C = false;
			_CU55M = false;
			_20CUG = false;
			_CU135 = false;
			_27CUG = false;
			_CU83 = false;
			_50CUG_M = false;
			_CU83_H03R1 = false;
			_37CUG = false;
			_16CUG = false;
			_512M = true;
			IsExtDevice = false;
		}
		else if ((vid == "2560") && (pid == "0102" || pid == "c123" || pid == "c129" || pid == "c05a" || pid == "c05c" || pid == "c181" || pid == "c184" || pid == "0035"))
		{
			IsExtDevice = true;
			_CU40 = false;
			_CU51 = false;
			_12CUNIR = false;
			_10CUG_C = false;
			_CU55M = false;
			_20CUG = false;
			_CU135 = false;
			_27CUG = false;
			_CU83 = false;
			_50CUG_M = false;
			_CU83_H03R1 = false;
			_37CUG = false;
			_16CUG = false;
			_512M = false;
		}
		else
		{
			IsExtDevice = false;
			_CU40 = false;
			_CU51 = false;
			_12CUNIR = false;
			_10CUG_C = false;
			_CU55M = false;
			_20CUG = false;
			_CU135 = false;
			_27CUG = false;
			_CU83 = false;
			_50CUG_M = false;
			_CU83_H03R1 = false;
			_37CUG = false;
			_16CUG = false;
			_512M = false;
		}
#ifdef _WIN32
		if (!IsExtDevice) 
		{
			readfirmwareversion = (Readfirmwareversion_t)GetProcAddress(hinstLib, "ReadFirmwareVersion");
			initextensionunit = (Initextensionunit_t)GetProcAddress(hinstLib, "InitExtensionUnit");
			deinitextensionunit = (Deinitextensionunit_t)GetProcAddress(hinstLib, "DeinitExtensionUnit");
		}
		else
		{
			if ((vid == "2560") && (pid == "0035")) { // tania
				readfirmwareversionTANIA = (ReadfirmwareversionTania_t)GetProcAddress(hinstLibExt, "GetFirmwareVersion");
				initextensionunitExt = (InitextensionunitExt_t)GetProcAddress(hinstLibExt, "InitTaniaExtensionUnit");
				deinitextensionunit = (Deinitextensionunit_t)GetProcAddress(hinstLibExt, "DeinitExtUnit");
			}
			else if ((vid == "2560") && (pid == "0102" || pid == "c123" || pid == "c129")) // eCAM22
			{
				readfirmwareversionECAM22 = (ReadfirmwareversionECAM22USB_t)GetProcAddress(hinstLibExt, "GetFirmwareVersionECAM22");
				initextensionunitExt = (InitextensionunitExt_t)GetProcAddress(hinstLibExt, "InitExtensionUnitECAM22");
				deinitextensionunit = (Deinitextensionunit_t)GetProcAddress(hinstLibExt, "DeinitExtUnit");
			}
			else if ((vid == "2560") && (pid == "c05a")) // eCAM51A
			{
				readfirmwareversionECAM51 = (ReadfirmwareversionECAM51_t)GetProcAddress(hinstLibExt, "GetFirmwareVersionECAM51A");
				initextensionunitExt = (InitextensionunitExt_t)GetProcAddress(hinstLibExt, "InitExtensionUniteCAM51A");
				deinitextensionunit = (Deinitextensionunit_t)GetProcAddress(hinstLibExt, "DeinitExtUnit");
			}
			else if ((vid == "2560") && (pid == "c05c")) // eCAM51b
			{
				readfirmwareversionECAM51 = (ReadfirmwareversionECAM51_t)GetProcAddress(hinstLibExt, "GetFirmwareVersionECAM51B");
				initextensionunitExt = (InitextensionunitExt_t)GetProcAddress(hinstLibExt, "InitExtensionUniteCAM51B");
				deinitextensionunit = (Deinitextensionunit_t)GetProcAddress(hinstLibExt, "DeinitExtUnit");
			}
			else if ((vid == "2560") && (pid == "c181")) // eCAM82
			{
				readfirmwareversion82USB = (Readfirmwareversion82USB_t)GetProcAddress(hinstLibExt, "GetFirmwareVersioneCAM82");
				initextensionunitExt = (InitextensionunitExt_t)GetProcAddress(hinstLibExt, "InitExtensionUnitECAM82");
				deinitextensionunit = (Deinitextensionunit_t)GetProcAddress(hinstLibExt, "DeinitExtUnit");
			}
			else if ((vid == "2560") && (pid == "c184")) // eCAM83
			{
				readfirmwareversion83USB = (Readfirmwareversion83USB_t)GetProcAddress(hinstLibExt, "GetFirmwareVersioneCAM83");
				initextensionunitExt = (InitextensionunitExt_t)GetProcAddress(hinstLibExt, "InitExtensionUnitECAM83");
				deinitextensionunit = (Deinitextensionunit_t)GetProcAddress(hinstLibExt, "DeinitExtUnit");
			}			
		}

#endif

		if (cap.isOpened())
		{
			cap.release();
		}

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
			if (camdevices == camId)
				break;
		}
		udev_enumerate_unref(enumerate);

		udev_unref(udev);

#endif

#ifdef _WIN32
		if (cap.open(camId - 1))
#elif __linux__
		if (cap.open(camId - 1))
#endif
		{
			if (!cap.isOpened())
			{
				cout << endl << "\t Camera Device not Initialised Successfully \n\n Press any Key to exit the application\n";
				return 0;
			}
		}

		if (pid == "c188")
		{
			if (width == 4440 && height == 2160)
			{
				cap.set(CAP_PROP_CONVERT_RGB, false);
			}
			else
			{
				cap.set(CAP_PROP_CONVERT_RGB, false);
			}
		}
		else if ((vid == "2560") && (pid == "c140") || (pid == "c152") || (pid == "0121") || (pid == "c113") || (pid == "c111") || (pid == "c155") || (pid == "c124") || (pid == "c1d8" || pid == "c1d7" || pid == "c0d7"))
		{
			cap.set(CAP_PROP_CONVERT_RGB/*CV_CAP_PROP_CONVERT_RGB*/, false);
		}

#ifdef _WIN32

		tDevicePath = new TCHAR[devicePath.size() + 1];
		copy(devicePath.begin(), devicePath.end(), tDevicePath);
		if (IsExtDevice) 
		{
			deinitextensionunit(handle);
			bOpenHID = initextensionunitExt(&handle , tDevicePath);
		}
		else
		{
			deinitextensionunit(handle);
			bOpenHID = initextensionunit(tDevicePath, &handle);
		}

		if (!bOpenHID || handle == nullptr) 
		{
			cout << "\n\tThis camera doesnt support extension unit HID." << endl;
		}

#elif __linux__
		bool isHidAvailable = false;

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
				continue;
			}

			VID = udev_device_get_sysattr_value(dev, "idVendor");
			PID = udev_device_get_sysattr_value(dev, "idProduct");

			if ((vid == VID) && (pid == PID))
			{
				isHidAvailable = true;
				devicePath = hidPath;
				udev_device_unref(dev);
				break;
			}
			udev_device_unref(dev);
		}

		udev_enumerate_unref(enumerate);
		udev_unref(udev);

		if (hid_fd > 0)
			closeHID();

		if (!dev)
			hid_fd = -1;
		else
			if (isHidAvailable)
				hid_fd = ::open(devicePath.c_str(), O_RDWR | O_NONBLOCK, 0);
			else
				hid_fd = -1;

		if (hid_fd < 0)
			bOpenHID = false;
		else
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
	while ((dilemma == 'y') || (dilemma == 'Y'))
	{
		int index = -1;

		if (!(cap.getFormats(formats)))
		{
			cout << endl << "Get Total number of Formats Failed" << endl;
			return false;
		}

		cout << endl << "Total Number of Formats Supported by the Camera:  " << '\t' << formats << endl;

		cout << '\t' << "0 - Exit" << endl;
		cout << '\t' << "1 - Back" << endl;
		cout << '\t' << "2 - Main Menu" << endl;
		int option = 3;

		for (int formatList = 0; formatList < formats; formatList++)
		{
			if (!(cap.getFormatType(formatList, formatType, width, height, fps)))
			{
				cout << endl << "Camera Get Format Type Failed" << endl;
				return false;
			}
			cout << '\t' << option << " . " << "FormatType: " << formatType << " Width: " << width << " Height: " << height << " Fps: " << fps << endl;
			option++;
			checkFormat = true; // included to check format while streaming after changing formats/resolution
		}
		while ((index < 0) || (index >= option))
		{
			printf("\nPick a choice to set a Particular Preview Format: \t");
			scanf("%d", &index);
			while (getchar() != '\n' && getchar() != EOF)
			{
			}
		}

		switch (index)
		{
		case EXIT:
#ifdef _WIN32
			bPreviewSet(1, false);
			if (handle != NULL) {
				if (deinitextensionunit(handle))
				{

				}
				handle = nullptr;
			}
			if(t.joinable())
				t.detach();
			bSwitch = true;
			if (cap.isOpened())
				cap.release();

#elif __linux__
			bSwitch = true;
			bPreviewSet(1, false);
			if (closeHID())
				destroyAllWindows();
			if (cap.isOpened())
				cap.release();

#endif
			exit(0);

		case 1:
		case 2:
			exploreCam();
			break;
		default:
			bSwitch = true;
			bPreviewSet(1, false);
			bReadSet(1, false);

			index = index - 3;

			if (index == -1)
			{
				cout << endl << "Invalid index value to configure the formats" << endl;
				return false;
			}

			if (!(cap.setFormatType(index)))
			{
				cout << endl << "Camera Set Format Type Failed" << endl;
				return false;
			}

			if (cap.getFormatType(index, formatType, width, height, fps))
			{
				cout << "\n\t **** The Current set Format type = " << formatType << " Width = " << width << " Height = " << height << " FPS = " << fps << " ****\n\n";
			}
			else {
				cout << "\n Not Printed";
			}
			if ((_CU55M || _50CUG_M || _37CUG || _512M))
				PixelBuff = new uchar[width * height];

			bReadSet(1, true);
			bPreviewSet(1, true);

			curWidth = width;
			curHeight = height;

#ifdef _WIN32
			if (formatType == "Y12")
				Y12Format = true;
			else
				Y12Format = false;
			if (formatType == "Y16")
				Y16Format = true;
			else
				Y16Format = false;
#elif __linux__
			if (formatType == "Y12 ")
				Y12Format = true;
			else
				Y12Format = false;

			if (formatType == "Y16 ")
			{
				cout << "\nY16True";
				Y16Format = true;
			}
			else
			{
				cout << "\nY16False";
				Y16Format = false;
			}
#endif
			formatType = '\0';
			checkFormat = true; // included to check format while streaming after changing formats/resolution
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
	while ((dilemma == 'y') || (dilemma == 'Y'))
	{
		int mode = 0;

		if (!(cap.get(Property, minimum, maximum, steppingDelta, supportedMode, currentValue, currentMode, defaultValue)))
		{
			cout << endl << PropStr << " Properties Couldn't be Retrieved for the Camera Connected" << endl;
			return false;
		}

		cout << endl << "Camera " << PropStr << " Values:: " << endl;

		if (supportedMode != AUTO)
		{
			cout << '\t' << "Minimum Value: " << minimum << endl;
			cout << '\t' << "Maximum Value: " << maximum << endl;
			cout << '\t' << "SteppingDelta: " << steppingDelta << endl;    //incrementing scale Value between min and max value
			cout << '\t' << "Default Value: " << defaultValue << endl;
		}

		((supportedMode < AUTOANDMANUAL) ? ((supportedMode == AUTO) ? (cout << '\t' << "Supported Mode: Auto" << endl) : (cout << '\t' << "Supported Mode: Manual" << endl)) : (cout << '\t' << "Supported Mode: Auto/Manual" << endl));

		if (currentMode == MANUAL)
			cout << '\t' << "Current Value: " << currentValue << endl;
		((currentMode == AUTO) ? (cout << '\t' << "Current Mode: Auto" << endl << endl) : (cout << '\t' << "Current Mode: Manual" << endl << endl));

		switch ((long)supportedMode)
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
			while ((mode <= 0) || (mode > 2))
			{
				printf("\n Enter a Valid mode to get selected: 1. Auto 2. Manual \n");
				scanf("%d", &mode);
				while (getchar() != '\n' && getchar() != EOF)
				{
				}
			}
			break;
		}

		while (mode == MANUAL)
		{
			cout << endl << "Enter a Valid value to Set " << PropStr << " : " << '\t';
			scanf("%d", &value);
			while (getchar() != '\n' && getchar() != EOF)
			{
			}
			if ((value >= minimum) && (value <= maximum) && (((value) % (steppingDelta)) == 0))
			{
				break;
			}
			cout << endl << "Certain Conditions Not met; Please ";
		}

		if (!(cap.set(Property, value, mode)))
		{
			cout << endl << "Camera " << PropStr << " is Not Set" << endl;
			return false;
		}

		((mode == AUTO) ? (cout << endl << "Auto " << PropStr << " Mode is Set" << endl) : (cout << endl << "Manual " << PropStr << " Mode is set with the Value : " << value << endl));

		cout << endl << "Camera " << PropStr << " Exploration: " << '\n';
		while (true)
		{
			cout << endl << "Enter y/Y to Continue or n/N to dis-Continue: " << '\t';
			scanf("%c", &dilemma);
			while (getchar() != '\n' && getchar() != EOF)
			{
			}
			if (dilemma == 'y' || dilemma == 'Y' || dilemma == 'n' || dilemma == 'N')
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

	//int vid[PROPERTY] = { EXIT, 1, 2, CV_CAP_PROP_BRIGHTNESS, CV_CAP_PROP_CONTRAST, CV_CAP_PROP_SATURATION, CV_CAP_PROP_HUE, CV_CAP_PROP_GAIN, CV_CAP_PROP_EXPOSURE, CV_CAP_PROP_WHITE_BALANCE_BLUE_U, CV_CAP_PROP_SHARPNESS, CV_CAP_PROP_GAMMA, CV_CAP_PROP_ZOOM, CV_CAP_PROP_FOCUS, CV_CAP_PROP_BACKLIGHT, CV_CAP_PROP_PAN, CV_CAP_PROP_TILT, CV_CAP_PROP_ROLL, CV_CAP_PROP_IRIS };
	//changed as per Opencv 4.5.3
	int vid[PROPERTY] = { EXIT, 1, 2,CAP_PROP_BRIGHTNESS, CAP_PROP_CONTRAST, CAP_PROP_SATURATION,
		CAP_PROP_HUE, CAP_PROP_GAIN, CAP_PROP_EXPOSURE, CAP_PROP_WHITE_BALANCE_BLUE_U,
		CAP_PROP_SHARPNESS,CAP_PROP_GAMMA, CAP_PROP_ZOOM, CAP_PROP_FOCUS, CAP_PROP_BACKLIGHT,
		CAP_PROP_PAN, CAP_PROP_TILT, CAP_PROP_ROLL, CAP_PROP_IRIS };
	string vidStr[PROPERTY] = { "Exit", "Back", "Main Menu", "Brightness", "Contrast", "Saturation", "Hue", "Gain", "Exposure", "White Balance", "Sharpness", "Gamma", "Zoom", "Focus", "Backlight", "Pan", "Tilt", "Roll", "Iris" };

#elif __linux__

	//int vid[PROPERTY] = { EXIT, 1, 2, CV_CAP_PROP_BRIGHTNESS, CV_CAP_PROP_CONTRAST, CV_CAP_PROP_SATURATION, CV_CAP_PROP_HUE, CV_CAP_PROP_GAIN, CV_CAP_PROP_EXPOSURE, CV_CAP_PROP_WHITE_BALANCE_BLUE_U, CV_CAP_PROP_SHARPNESS, CV_CAP_PROP_GAMMA, CV_CAP_PROP_ZOOM, CV_CAP_PROP_FOCUS, CV_CAP_PROP_BACKLIGHT, CV_CAP_PROP_PAN, CV_CAP_PROP_TILT };
	//changed as per Opencv 4.5.3
	int vid[PROPERTY] = { EXIT, 1, 2,CAP_PROP_BRIGHTNESS, CAP_PROP_CONTRAST, CAP_PROP_SATURATION,
		CAP_PROP_HUE, CAP_PROP_GAIN, CAP_PROP_EXPOSURE, CAP_PROP_WHITE_BALANCE_BLUE_U,
		CAP_PROP_SHARPNESS,CAP_PROP_GAMMA, CAP_PROP_ZOOM, CAP_PROP_FOCUS, CAP_PROP_BACKLIGHT,
		CAP_PROP_PAN, CAP_PROP_TILT };
	string vidStr[PROPERTY] = { "Exit", "Back", "Main Menu", "Brightness", "Contrast", "Saturation", "Hue", "Gain", "Exposure", "White Balance", "Sharpness", "Gamma", "Zoom", "Focus", "Backlight", "Pan", "Tilt" };

#endif

	while (true)
	{
		int choice = -1, settings = 0;

		for (int eachVideoSetting = 0; eachVideoSetting < PROPERTY; eachVideoSetting++)
		{
			if (!((eachVideoSetting >= 0) && (eachVideoSetting < 3)))
			{
				//Checks whether the Specific UVC Property is supported or not
				if (cap.get(vid[eachVideoSetting], minimum, maximum, steppingDelta, supportedMode, currentValue, currentMode, defaultValue))
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

		while ((choice < 0) || (choice >= settings))
		{
			printf("\n Pick a Choice to Configure UVC Settings : \t");
			scanf("%d", &choice);
			while (getchar() != '\n' && getchar() != EOF)
			{
			}
		}

		dilemma = 'y';

		switch (uvcProperty[choice].first)
		{
		case EXIT:
#ifdef _WIN32

			if (deinitextensionunit(handle))
			{
				handle = nullptr;
				t.detach();
			}
			bSwitch = true;
			if (cap.isOpened())
				cap.release();

#elif __linux__

			bPreviewSet(1, false);
			if (closeHID())
				destroyAllWindows();

#endif

			exit(0);

		case 1:
		case 2:
			if (!(exploreCam()))
			{
				cout << endl << "Camera Exploration Failed" << endl;
				return false;
			}
			cout << endl << "Camera Exploration is done" << endl;
			break;

			// changed Opencv 4.5.3
		case CAP_PROP_BRIGHTNESS/*CV_CAP_PROP_BRIGHTNESS*/:
		case CAP_PROP_CONTRAST/*CV_CAP_PROP_CONTRAST*/:
		case CAP_PROP_HUE/*CV_CAP_PROP_HUE*/:
		case CAP_PROP_SATURATION/*CV_CAP_PROP_SATURATION*/:
		case CAP_PROP_SHARPNESS/*CV_CAP_PROP_SHARPNESS*/:
		case CAP_PROP_GAMMA/*CV_CAP_PROP_GAMMA*/:
		case CAP_PROP_WHITE_BALANCE_BLUE_U/*CV_CAP_PROP_WHITE_BALANCE_BLUE_U*/:
		case CAP_PROP_BACKLIGHT/*CV_CAP_PROP_BACKLIGHT*/:
		case CAP_PROP_GAIN/*CV_CAP_PROP_GAIN*/:
		case CAP_PROP_PAN/*CV_CAP_PROP_PAN*/:
		case CAP_PROP_TILT/*CV_CAP_PROP_TILT*/:
		case CAP_PROP_ROLL/*CV_CAP_PROP_ROLL*/:
		case CAP_PROP_ZOOM/*CV_CAP_PROP_ZOOM*/:
		case CAP_PROP_EXPOSURE/*CV_CAP_PROP_EXPOSURE*/:
		case CAP_PROP_IRIS/*CV_CAP_PROP_IRIS*/:
		case CAP_PROP_FOCUS/*CV_CAP_PROP_FOCUS*/:
			if (!(setVidProp(uvcProperty[choice].first, uvcProperty[choice].second)))
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
	Mat stillFrame, RsltFrame;

	bool IsRAWSelected = false;

#ifdef _WIN32
	struct tm tm;
	localtime_s(&tm, &t);

	if (IsRawSaveSupport())
	{
		int choice = -1;
		cout << endl << '\t' << "0 - Back" << endl;
		cout << '\t' << "1 - RAW Format" << endl;
		cout << '\t' << "2 - RGB Format" << endl;


		while ((choice < 0) || (choice > 2))
		{
			printf("\n Pick a Relevant Choice to Configure Particular Still capture mode: \t");
			scanf("%d", &choice);
			while (getchar() != '\n' && getchar() != EOF)
			{
			}
		}

		switch (choice)
		{
		case 0:
			return false;
		case 1:
			IsRAWSelected = true;

			bPreviewSet(1, false);
			cap.set(CAP_PROP_CONVERT_RGB, false);
			//num = sprintf_s(buf, "OpenCVCam_%dx%d_%d%d%d_%d%d%d.raw", curWidth, curHeight, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

			goto CAPTURE;
		case 2:
			IsRAWSelected = false;
			//num = sprintf_s(buf, "OpenCVCam_%dx%d_%d%d%d_%d%d%d.jpeg", curWidth, curHeight, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

			goto CAPTURE;
		}
	}

CAPTURE:

	if (((_CU55M || _50CUG_M || _37CUG || _512M) && Y12Format) || ((_CU135 || _20CUG || _16CUG) && Y16Format) || (Y10Format))
	{
		if (IsRAWSelected == true)
		{
			num = sprintf_s(buf, "OpenCVCam_%dx%d_%d%d%d_%d%d%d.raw", curWidth, curHeight, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
		}
		else if((IsRAWSelected == false) && Y16Format)
		{
			num = sprintf_s(buf, "OpenCVCam_%dx%d_%d%d%d_%d%d%d.jpeg", curWidth, curHeight, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
		}
	}
	else if (_CU40)
	{
		num = sprintf_s(buf, "OpenCVCamBGR_%dx%d_%d%d%d%d%d%d.jpeg", curWidth, curHeight, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
		num = sprintf_s(buf1, "OpenCVCamIR_%dx%d_%d%d%d%d%d%d.jpeg", curWidth, curHeight, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
	else if ((_CU83 && Y16Format && Frame.cols == 4440 && Frame.rows == 2160) 
				|| (_CU83 && Y16Format && Frame.cols == 3120 && Frame.rows == 1080)
				|| _27CUG 
				|| (_CU83_H03R1 && Y16Format && Frame.cols == 4440 && Frame.rows == 2160)
				|| (_CU83_H03R1 && Y16Format && Frame.cols == 3120 && Frame.rows == 1080))
	{
		num = sprintf_s(buf, "OpenCVCamBGR_%dx%d_%d%d%d%d%d%d.jpeg", curWidth, curHeight, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
		num = sprintf_s(buf1, "OpenCVCamIR_%dx%d_%d%d%d%d%d%d.raw", curWidth, curHeight, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
	else
	{
		if (IsRAWSelected == true) 
		{
			num = sprintf_s(buf, "OpenCVCam_%dx%d_%d%d%d_%d%d%d.raw", curWidth, curHeight, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
		}
		else
		{
			num = sprintf_s(buf, "OpenCVCam_%dx%d_%d%d%d_%d%d%d.jpeg", curWidth, curHeight, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

		}
	}

#elif __linux__

	struct tm *tm;
	tm = localtime(&t);

	char cwd[256];
	getcwd(cwd, sizeof(cwd));


	if (IsRawSaveSupport())
	{
		int choice = -1;
		cout << endl << '\t' << "0 - Back" << endl;
		cout << '\t' << "1 - RAW Format" << endl;
		cout << '\t' << "2 - RGB Format" << endl;


		while ((choice < 0) || (choice > 2))
		{
			printf("\n Pick a Relevant Choice to Configure Particular Still capture mode: \t");
			scanf("%d", &choice);
			while (getchar() != '\n' && getchar() != EOF)
			{
			}
		}

		switch (choice)
		{
		case 0:
			return false;
		case 1:
			IsRAWSelected = true;

			bPreviewSet(1, false);
			cap.set(CAP_PROP_CONVERT_RGB, false);

			sprintf(buf, "%s/OpenCVCam_%dx%d_%d%d%d_%d%d%d.raw", cwd, curWidth, curHeight, tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);

			goto CAPTURE;
		case 2:
			IsRAWSelected = false;

			sprintf(buf, "%s/OpenCVCam_%dx%d_%d%d%d_%d%d%d.jpeg", cwd, curWidth, curHeight, tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);

			goto CAPTURE;
		}
	}

CAPTURE:

	if (_CU40)
	{
		sprintf(buf, "%s/OpenCVCamBGR_%dx%d_%d%d%d_%d%d%d.jpeg", cwd, curWidth, curHeight, tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
		sprintf(buf1, "%s/OpenCVCamIR_%dx%d_%d%d%d%d%d%d.jpeg", cwd, curWidth, curHeight, tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
	}
	else if (((_CU55M || _50CUG_M || _37CUG || _512M) && Y12Format) || ((_CU135 || _20CUG || _16CUG) && Y16Format) || (Y10Format)) {
		sprintf(buf, "%s/OpenCVCam_%dx%d_%d%d%d_%d%d%d.raw", cwd, curWidth, curHeight, tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
	}
	else if ((_CU83 && Y16Format && Frame.cols == 4440 && Frame.rows == 2160)
		|| (_CU83 && Y16Format && Frame.cols == 3120 && Frame.rows == 1080)
		|| _27CUG
		|| (_CU83_H03R1 && Y16Format && Frame.cols == 4440 && Frame.rows == 2160)
		|| (_CU83_H03R1 && Y16Format && Frame.cols == 3120 && Frame.rows == 1080))
	{
		num = sprintf(buf, "OpenCVCamBGR_%dx%d_%d%d%d%d%d%d.jpeg", curWidth, curHeight, tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
		num = sprintf(buf1, "OpenCVCamIR_%dx%d_%d%d%d%d%d%d.raw", curWidth, curHeight, tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
	}
	else 
		sprintf(buf, "%s/OpenCVCam_%dx%d_%d%d%d_%d%d%d.jpeg", cwd, curWidth, curHeight, tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);


#endif

	// commented by Rishap
//#ifdef __linux__
//	bReadSet(1, false);
//#endif

	bReadSet(1, false);
	if (cap.read(stillFrame))
	{
		if (!stillFrame.empty())
		{
			//Changed TRUE & FALSE to true & false respectively - commented by Sushanth
			//Reason - compatible for both linux & windows
			if (IsRAWSelected)
			{
				if(_CU83 == true && formatType.substr(0, 4) == "UYVY")
				{
					SaveInRAW(stillFrame.data, buf, (stillFrame.cols * stillFrame.rows * 2));
				}
				else if((_CU83 == true && formatType.substr(0, 2) == "Y8"))
				{
					SaveInRAW(stillFrame.data, buf, (stillFrame.cols * stillFrame.rows));
				}
				else if (formatType.substr(0, 2) == "Y8")
				{
					SaveInRAW(stillFrame.data, buf, (stillFrame.cols * stillFrame.rows));
				}
				else
				{
					SaveInRAW(stillFrame.data, buf, (stillFrame.cols * stillFrame.rows * 2));
				}
#ifdef __linux__
				if (((_CU135 || _20CUG || _16CUG) && Y16Format) || (Y10Format))
					cap.set(CAP_PROP_CONVERT_RGB, false);
				else if (_27CUG || _CU83 || _CU83_H03R1)
					cap.set(CAP_PROP_CONVERT_RGB, false);
				else
					cap.set(CAP_PROP_CONVERT_RGB, true);

				bPreviewSet(1, true);
#endif
			}
			else
			{
				if ((_12CUNIR) || (_CU51))
				{
					//Convert to 8 Bit:
					//Scale the 12 Bit (4096) Pixels into 8 Bit(255) (255/4096)= 0.06226
					convertScaleAbs(stillFrame, ResultImage, 0.06226);
					imwrite(buf, ResultImage);
				}
				else if (_CU40)
				{
					//Convert to 8 Bit:
					//Scale the 10 Bit (1024) Pixels into 8 Bit(255) (255/1024)= 0.249023
					convertScaleAbs(stillFrame, BayerFrame8, 0.249023);

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
				else if (_CU83 && Y16Format || _CU83_H03R1 && Y16Format)
				{
					if (stillFrame.cols == 4440 && stillFrame.rows == 2160)
					{
						RGBStillImageCU83 = Mat(2160, 3840, CV_8UC2); //allocation
						IRStillImageCU83 = Mat(675, 1920, CV_8UC2);
						if (!stillFrame.empty())
							SeparatingRGBIRBufferStillCapture(stillFrame, &IRStillImageCU83, &RGBStillImageCU83, &RGBStillBufferSizeCU83, &IRStillBufferSizeCU83);

						if (!RGBStillImageCU83.empty())
						{
							cvtColor(RGBStillImageCU83, ResultImage, COLOR_YUV2BGR_UYVY);
							imwrite(buf, ResultImage);
						}
						if (!IRStillImageCU83.empty())
						{
							Mat m_RAW10BitDataOutputBuffer = Mat(1080, 1920, CV_8UC2);
							RAW10ConversionStillCU83(IRStillImageCU83.data, &m_RAW10BitDataOutputBuffer, IRStillBufferSizeCU83);
							SaveInRAW(m_RAW10BitDataOutputBuffer.data, buf1, IRStillBufferSizeCU83);
							cout << endl << '\t' << buf1 << " image is saved " << endl << endl;
						}

					}
					else if ((stillFrame.cols == 3120 && stillFrame.rows == 1080))
					{
						RGBStillImageCU83 = Mat(1080, 1920, CV_8UC2); //allocation
						IRStillImageCU83 = Mat(675, 1920, CV_8UC2);
						if (!stillFrame.empty())
							SeparatingRGBIRBufferStillCapture(stillFrame, &IRStillImageCU83, &RGBStillImageCU83, &RGBStillBufferSizeCU83, &IRStillBufferSizeCU83);

						if (!RGBStillImageCU83.empty())
						{
							cvtColor(RGBStillImageCU83, ResultImage, COLOR_YUV2BGR_UYVY);
							imwrite(buf, ResultImage);
						}
						if (!IRStillImageCU83.empty())
						{
							Mat m_RAW10BitDataOutputBuffer = Mat(1080, 1920, CV_8UC2);
							RAW10ConversionStillCU83(IRStillImageCU83.data, &m_RAW10BitDataOutputBuffer, IRStillBufferSizeCU83);
							SaveInRAW(m_RAW10BitDataOutputBuffer.data, buf1, IRStillBufferSizeCU83);
							cout << endl << '\t' << buf1 << " image is saved " << endl << endl;
						}
					}
					else
					{
						if (stillFrame.cols == 3840 && stillFrame.rows == 1350)
						{
							int modifiedCols = 2160;
							int size = (stillFrame.cols * modifiedCols * stillFrame.channels());
							if (IRImageCU83.empty())
								IRImageCU83 = Mat(2160, 3840, CV_8UC1);
							ConvertRAW10toRAW8(stillFrame, IRImageCU83);

							if (IsRAWSelected)
								SaveInRAW(IRImageCU83.data, buf, size);
							else
								imwrite(buf, IRImageCU83);
						}
						else if ((stillFrame.cols == 1920 && stillFrame.rows == 675))
						{
							int modifiedCols = 1080;
							int size = (stillFrame.cols * modifiedCols * stillFrame.channels());
							if (IRImageCU83.empty())
								IRImageCU83 = Mat(1080, 1920, CV_8UC1);
							IRImageCU83 = Mat(1080, 1920, CV_8UC1);
							ConvertRAW10toRAW8(stillFrame, IRImageCU83);

							if (IsRAWSelected)
								SaveInRAW(IRImageCU83.data, buf, size);
							else
								imwrite(buf, IRImageCU83);
						}
					}

				}
				else if ((_CU55M || _50CUG_M || _37CUG || _512M) && Y12Format)
				{
					StillBuff = new uchar[stillFrame.cols * stillFrame.rows * 2];
					ConvertY12forStill(stillFrame, StillBuff);
					SaveInRAW(StillBuff, buf, (stillFrame.cols * stillFrame.rows * 2));
				}
				else if (_27CUG)
				{
					cvtColor(stillFrame, RsltFrame, COLOR_YUV2BGR_UYVY);
					imwrite(buf, RsltFrame);
					SaveInRAW(stillFrame.data, buf1, Frame.rows*Frame.cols * 2);
					cout << endl << '\t' << buf1 << " image is saved " << endl << endl;
				}
				else if (((_CU135 || _20CUG || _16CUG) && Y16Format) || (Y10Format))
				{
					convertScaleAbs(stillFrame, ResultImage, 0.2490234375);
					
					if (IsRAWSelected || Y10Format)
						SaveInRAW(stillFrame.data, buf, (Frame.rows*Frame.cols * 2));
					else
						imwrite(buf, ResultImage);
					
				}
#ifdef _WIN32
				else if (formatType.substr(0, 4) == "YUY2") {

					/*cap.set(CAP_PROP_CONVERT_RGB, 1);*/
					cvtColor(stillFrame, RsltFrame, COLOR_YUV2BGR_YUY2);
					imwrite(buf, RsltFrame);
				}
				else if (formatType.substr(0, 4) == "UYVY")
				{
					cvtColor(stillFrame, RsltFrame, COLOR_YUV2BGR_UYVY);
					imwrite(buf, RsltFrame);
				}
#endif
				else
				{
					imwrite(buf, stillFrame);
					//commented by rishap
					//#ifdef __linux__
					//					bReadSet(1, true);
					//#endif
				}

			}
			bReadSet(1, true);
			bPreviewSet(1, true);
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

	if (hid_fd < 0)
		result = false;

	int ret = 0;
	ret = ::write(hid_fd, inputBuffer, BUFFERLENGTH);

	if (ret < 0)
	{
		cout << endl << "Writing Data to the UVC Extension is Failed" << endl;
		return false;
	}

	//Reads Data From the Device
	if (hid_fd < 0)
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
	if (0 > select(1, &rfds, NULL, NULL, &tv))
	{
		perror("select");
		return false;
	}

	ret = ::read(hid_fd, outputBuffer, BUFFERLENGTH);
	if ((ret < 0) || (outputBuffer[0] != READFIRMWAREVERSION))
		result = false;
	else
		result = true;

	if (!result)
	{
		cout << endl << "Reading Data from the UVC Extension is Failed" << endl;
		return false;
	}

	int pMajorVersion, pMinorVersion1, pMinorVersion2, pMinorVersion3, SDK_VER, SVN_VER;

	SDK_VER = (outputBuffer[3] << 8) + outputBuffer[4];
	SVN_VER = (outputBuffer[5] << 8) + outputBuffer[6];
	pMajorVersion = outputBuffer[1];
	pMinorVersion1 = outputBuffer[2];

	pMinorVersion2 = SDK_VER;
	pMinorVersion3 = SVN_VER;

#elif _WIN32

	uint8_t pMajorVersion = 0;
	uint8_t pMinorVersion1 = 0;
	uint16_t pMinorVersion2 = 0;
	uint16_t pMinorVersion3 = 0;

	if (!IsExtDevice)
	{
		BOOL result = readfirmwareversion(handle, &pMajorVersion, &pMinorVersion1, &pMinorVersion2, &pMinorVersion3);
		if (!result)
		{
			cout << endl << "Reading Data from the UVC Extension is Failed" << endl;
			return false;
		}
		printf("\nFirmWareVersion Number = %d.%d.%d.%d \n\n", pMajorVersion, pMinorVersion1, pMinorVersion2, pMinorVersion3);
	}
	else
	{
		if ((vid == "2560") && (pid == "0035")) // tania
		{ 
			CHAR g_pValue[41]; // Buffer to hold firmware version data
			CHAR Buffer[41];   // Buffer for formatted output
			BOOL result = readfirmwareversionTANIA(handle, g_pValue);

			if (!result) {
				cout << endl << "Reading Data from the UVC Extension is Failed" << endl;
				return false;
			}

			// Clear the Buffer to avoid garbage data
			memset(Buffer, 0x00, sizeof(Buffer));

			// Format the firmware version from g_pValue
			StringCbPrintfA(Buffer, sizeof(Buffer), "0x%x.0x%x (%d.%d)", g_pValue[27], g_pValue[28], g_pValue[27], g_pValue[28]);

			// Print the formatted firmware version
			printf("\nFirmware Version (TANIA) = %s\n", Buffer);
		}
		else if ((vid == "2560") && (pid == "0102" || pid == "c123" || pid == "c129")) // eCAM22
		{
			uint8_t uFirmwareValue[4]; // Buffer to hold firmware version values
			BOOL result = readfirmwareversionECAM22(handle, 0x07, uFirmwareValue);

			if (!result) {
				cout << endl << "Failed to read firmware version for eCAM22." << endl;
				return false;
			}

			// Format firmware version message
			WCHAR firmwareMessage[MAX_PATH];
			StringCchPrintf(firmwareMessage, MAX_PATH, L"Firmware Version: %d.%d.%d.%d",
				uFirmwareValue[0], uFirmwareValue[1], uFirmwareValue[2], uFirmwareValue[3]);

			// Print firmware version to console
			wprintf(L"\nFirmware Version (eCAM22) = %s\n", firmwareMessage);
		}
		else if ((vid == "2560") && (pid == "c05a" || pid == "c05c")) // eCAM51A and eCAM51B
		{
			uint8_t uFirmwareValue[4]; // Buffer to hold firmware version values
			BOOL result = readfirmwareversionECAM51(handle, uFirmwareValue);

			if (!result) {
				cout << endl << "Failed to read firmware version for eCAM51A." << endl;
				return false;
			}

			// Format firmware version message
			WCHAR firmwareMessage[MAX_PATH];
			StringCchPrintf(firmwareMessage, MAX_PATH, L"Firmware Version: 20%d.%d.%d Ver %d",
				uFirmwareValue[0], uFirmwareValue[1], uFirmwareValue[2], uFirmwareValue[3]);

			// Print the firmware version to the console
			wprintf(L"\nFirmware Version (eCAM51A) = %s\n", firmwareMessage);
		}	
		else if ((vid == "2560") && (pid == "c181")) // eCAM82
		{
			uint8_t FirmwareVersion1 = 0;
			uint8_t FirmwareVersion2 = 0;
			uint16_t FirmwareVersion3 = 0;

			BOOL result = readfirmwareversion82USB(handle, &FirmwareVersion1, &FirmwareVersion2, &FirmwareVersion3);

			if (!result) {
				cout << endl << "Failed to read firmware version for eCAM82." << endl;
				return false;
			}

			// Format firmware version message
			WCHAR firmwareMessage[MAX_PATH];
			StringCchPrintf(firmwareMessage, MAX_PATH, L"Firmware Version: %d.%d.%d",
				FirmwareVersion1, FirmwareVersion2, FirmwareVersion3);

			// Print the firmware version to the console
			wprintf(L"\nFirmware Version (eCAM82) = %s\n", firmwareMessage);
		}
		else if ((vid == "2560") && (pid == "c184")) // eCAM83
		{
			uint8_t MajorVersion = 0;
			uint8_t MinorVersion = 0;
			uint16_t MinorVersion1 = 0;
			uint32_t CommitId = 0;

			BOOL result = readfirmwareversion83USB(handle, &MajorVersion, &MinorVersion, &MinorVersion1, &CommitId);

			if (!result) {
				cout << endl << "Failed to read firmware version for eCAM83." << endl;
				return false;
			}

			// Format firmware version message
			WCHAR firmwareMessage[MAX_PATH * 2];
			StringCbPrintf(firmwareMessage, sizeof(firmwareMessage), L"Firmware Version: %d.%d.%d.%x",
				MajorVersion, MinorVersion, MinorVersion1, CommitId);

			// Print the firmware version to the console
			wprintf(L"\nFirmware Version (eCAM83) = %s\n", firmwareMessage);
		}

	}

#endif


	return true;
}


//HID Settings
//
bool hidProp()
{
	while (true)
	{
		int choice = -1;
		cout << endl << '\t' << "0 - Exit" << endl;
		cout << '\t' << "1 - Back" << endl;
		cout << '\t' << "2 - Main Menu" << endl;
		cout << '\t' << "3 - GetFirmware Version Number" << endl;

		while ((choice < 0) || (choice > 3))
		{
			printf("\n Pick a Relevant Choice to Configure Particular Camera Properties: \t");
			scanf("%d", &choice);
			while (getchar() != '\n' && getchar() != EOF)
			{
			}
		}

		switch (choice)
		{
		case EXIT:
#ifdef _WIN32

			if (deinitextensionunit(handle))
			{
				handle = nullptr;
				t.detach();
			}
			bSwitch = true;
			if (cap.isOpened())
				cap.release();

#elif __linux__

			bPreviewSet(1, false);
			if (closeHID())
				destroyAllWindows();

#endif

			exit(0);

		case 1:
		case 2:
			if (!(exploreCam()))
				cout << "Camera Exploration is Failed" << endl;
			break;

		case 3:
			if (!configExtUVCSettings())
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
	while (true)
	{
		int choice = -1;
		cout << endl << '\t' << "0 - Exit" << endl;
		cout << '\t' << "1 - Back" << endl;
		cout << '\t' << "2 - Configure Camera Format/Resolution" << endl;
		cout << '\t' << "3 - Configure UVC Settings" << endl;
		cout << '\t' << "4 - Capture Still Images" << endl;

		if (bOpenHID == 1)
		{
			cout << '\t' << "5 - HID Properties" << endl;
			while ((choice < 0) || (choice > 5))
			{
				printf("\n Pick a Relevant Choice to Configure Particular Camera Properties : \t");
				scanf("%d", &choice);
				while (getchar() != '\n' && getchar() != EOF)
				{
				}
			}
		}
		else
		{
			while ((choice < 0) || (choice >= 5))
			{
				printf("\n Pick a Relevant Choice to Configure Particular Camera Properties : \t");
				scanf("%d", &choice);
				while (getchar() != '\n' && getchar() != EOF)
				{
				}
			}
		}

		switch (choice)
		{
		case EXIT:
		{
#ifdef _WIN32
			{
				if (handle != NULL)
				{
					if (deinitextensionunit(handle))
					{
					}

					handle = nullptr;
				}

				if (t.joinable())
				{
					t.detach();
				}

				//t.detach();
				bSwitch = true;
				if (cap.isOpened())
				{
					cap.release();
				}

#elif __linux__

			bPreviewSet(1, false);
			if (closeHID())
				destroyAllWindows();

#endif

			exit(0);
			}
		}
		case 1:
		{
			bPreviewSet(1, false);
			handle = nullptr;
			
			cameraswitch = true;
			if (!listDevices())
			{
				cout << endl << "List Devices Information failed" << endl;
				return false;
			}
			cout << endl << "Connected Devices were Listed" << endl;
			break;
		}
		case 2:
			if (!configFormats())
			{
				cout << endl << "Format Configuration is Failed" << endl;
				return false;
			}
			cout << endl << "Format Configuration is done" << endl;
			break;

		case 3:
			/** cout<<"\n configUVCSettings"; **/
			if (!configUVCSettings())
			{
				cout << endl << "UVC Settings Configuration is Failed" << endl;
				return false;
			}
			cout << endl << "UVC Settings Configuration is Done" << endl;
			break;

		case 4:
			if (!captureStill())
			{
				cout << endl << "Still Capture Failed" << endl;
			}
			else
				cout << endl << "Still Capture is Done" << endl;
			break;

		case 5:
			if (!hidProp())
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
