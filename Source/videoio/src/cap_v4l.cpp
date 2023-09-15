/* This is the contributed code:

File:             cvcap_v4l.cpp
Current Location: ../opencv-0.9.6/otherlibs/videoio

Original Version: 2003-03-12  Magnus Lundin lundin@mlu.mine.nu
Original Comments:

ML:This set of files adds support for firevre and usb cameras.
First it tries to install a firewire camera,
if that fails it tries a v4l/USB camera
It has been tested with the motempl sample program

First Patch:  August 24, 2004 Travis Wood   TravisOCV@tkwood.com
For Release:  OpenCV-Linux Beta4  opencv-0.9.6
Tested On:    LMLBT44 with 8 video inputs
Patched Comments:

TW: The cv cam utils that came with the initial release of OpenCV for LINUX Beta4
were not working.  I have rewritten them so they work for me. At the same time, trying
to keep the original code as ML wrote it as unchanged as possible.  No one likes to debug
someone elses code, so I resisted changes as much as possible.  I have tried to keep the
same "ideas" where applicable, that is, where I could figure out what the previous author
intended. Some areas I just could not help myself and had to "spiffy-it-up" my way.

These drivers should work with other V4L frame capture cards other then my bttv
driven frame capture card.

Re Written driver for standard V4L mode. Tested using LMLBT44 video capture card.
Standard bttv drivers are on the LMLBT44 with up to 8 Inputs.

This utility was written with the help of the document:
http://pages.cpsc.ucalgary.ca/~sayles/VFL_HowTo
as a general guide for interfacing into the V4l standard.

Made the index value passed for icvOpenCAM_V4L(index) be the number of the
video device source in the /dev tree. The -1 uses original /dev/video.

Index  Device
  0    /dev/video0
  1    /dev/video1
  2    /dev/video2
  3    /dev/video3
  ...
  7    /dev/video7
with
  -1   /dev/video

TW: You can select any video source, but this package was limited from the start to only
ONE camera opened at any ONE time.
This is an original program limitation.
If you are interested, I will make my version available to other OpenCV users.  The big
difference in mine is you may pass the camera number as part of the cv argument, but this
convention is non standard for current OpenCV calls and the camera number is not currently
passed into the called routine.

Second Patch:   August 28, 2004 Sfuncia Fabio fiblan@yahoo.it
For Release:  OpenCV-Linux Beta4 Opencv-0.9.6

FS: this patch fix not sequential index of device (unplugged device), and real numCameras.
    for -1 index (icvOpenCAM_V4L) I don't use /dev/video but real device available, because
    if /dev/video is a link to /dev/video0 and i unplugged device on /dev/video0, /dev/video
    is a bad link. I search the first available device with indexList.

Third Patch:   December 9, 2004 Frederic Devernay Frederic.Devernay@inria.fr
For Release:  OpenCV-Linux Beta4 Opencv-0.9.6

[FD] I modified the following:
 - handle YUV420P, YUV420, and YUV411P palettes (for many webcams) without using floating-point
 - cvGrabFrame should not wait for the end of the first frame, and should return quickly
   (see videoio doc)
 - cvRetrieveFrame should in turn wait for the end of frame capture, and should not
   trigger the capture of the next frame (the user choses when to do it using GrabFrame)
   To get the old behavior, re-call cvRetrieveFrame just after cvGrabFrame.
 - having global bufferIndex and FirstCapture variables makes the code non-reentrant
 (e.g. when using several cameras), put these in the CvCapture struct.
 - according to V4L HowTo, incrementing the buffer index must be done before VIDIOCMCAPTURE.
 - the VID_TYPE_SCALES stuff from V4L HowTo is wrong: image size can be changed
   even if the hardware does not support scaling (e.g. webcams can have several
   resolutions available). Just don't try to set the size at 640x480 if the hardware supports
   scaling: open with the default (probably best) image size, and let the user scale it
   using SetProperty.
 - image size can be changed by two subsequent calls to SetProperty (for width and height)
 - bug fix: if the image size changes, realloc the new image only when it is grabbed
 - issue errors only when necessary, fix error message formatting.

Fourth Patch: Sept 7, 2005 Csaba Kertesz sign@freemail.hu
For Release:  OpenCV-Linux Beta5 OpenCV-0.9.7

I modified the following:
  - Additional Video4Linux2 support :)
  - Use mmap functions (v4l2)
  - New methods are internal:
    try_palette_v4l2 -> rewrite try_palette for v4l2
    mainloop_v4l2, read_image_v4l2 -> this methods are moved from official v4l2 capture.c example
    try_init_v4l -> device v4l initialisation
    try_init_v4l2 -> device v4l2 initialisation
    autosetup_capture_mode_v4l -> autodetect capture modes for v4l
    autosetup_capture_mode_v4l2 -> autodetect capture modes for v4l2
  - Modifications are according with Video4Linux old codes
  - Video4Linux handling is automatically if it does not recognize a Video4Linux2 device
  - Tested successfully with Logitech Quickcam Express (V4L), Creative Vista (V4L) and Genius VideoCam Notebook (V4L2)
  - Correct source lines with compiler warning messages
  - Information message from v4l/v4l2 detection

Fifth Patch: Sept 7, 2005 Csaba Kertesz sign@freemail.hu
For Release:  OpenCV-Linux Beta5 OpenCV-0.9.7

I modified the following:
  - SN9C10x chip based webcams support
  - New methods are internal:
    bayer2rgb24, sonix_decompress -> decoder routines for SN9C10x decoding from Takafumi Mizuno <taka-qce@ls-a.jp> with his pleasure :)
  - Tested successfully with Genius VideoCam Notebook (V4L2)

Sixth Patch: Sept 10, 2005 Csaba Kertesz sign@freemail.hu
For Release:  OpenCV-Linux Beta5 OpenCV-0.9.7

I added the following:
  - Add capture control support (hue, saturation, brightness, contrast, gain)
  - Get and change V4L capture controls (hue, saturation, brightness, contrast)
  - New method is internal:
    icvSetControl -> set capture controls
  - Tested successfully with Creative Vista (V4L)

Seventh Patch: Sept 10, 2005 Csaba Kertesz sign@freemail.hu
For Release:  OpenCV-Linux Beta5 OpenCV-0.9.7

I added the following:
  - Detect, get and change V4L2 capture controls (hue, saturation, brightness, contrast, gain)
  - New methods are internal:
    v4l2_scan_controls_enumerate_menu, v4l2_scan_controls -> detect capture control intervals
  - Tested successfully with Genius VideoCam Notebook (V4L2)

8th patch: Jan 5, 2006, Olivier.Bornet@idiap.ch
Add support of V4L2_PIX_FMT_YUYV and V4L2_PIX_FMT_MJPEG.
With this patch, new webcams of Logitech, like QuickCam Fusion works.
Note: For use these webcams, look at the UVC driver at
http://linux-uvc.berlios.de/

9th patch: Mar 4, 2006, Olivier.Bornet@idiap.ch
- try V4L2 before V4L, because some devices are V4L2 by default,
  but they try to implement the V4L compatibility layer.
  So, I think this is better to support V4L2 before V4L.
- better separation between V4L2 and V4L initialization. (this was needed to support
  some drivers working, but not fully with V4L2. (so, we do not know when we
  need to switch from V4L2 to V4L.

10th patch: July 02, 2008, Mikhail Afanasyev fopencv@theamk.com
Fix reliability problems with high-resolution UVC cameras on linux
the symptoms were damaged image and 'Corrupt JPEG data: premature end of data segment' on stderr
- V4L_ABORT_BADJPEG detects JPEG warnings and turns them into errors, so bad images
  could be filtered out
- USE_TEMP_BUFFER fixes the main problem (improper buffer management) and
  prevents bad images in the first place

11th patch: April 2, 2013, Forrest Reiling forrest.reiling@gmail.com
Added v4l2 support for getting capture property CV_CAP_PROP_POS_MSEC.
Returns the millisecond timestamp of the last frame grabbed or 0 if no frames have been grabbed
Used to successfully synchronize 2 Logitech C310 USB webcams to within 16 ms of one another

12th patch: March 9, 2018, Taylor Lanclos <tlanclos@live.com>
 added support for CV_CAP_PROP_BUFFERSIZE

make & enjoy!

*/

/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "precomp.hpp"

#if !defined _WIN32 && (defined HAVE_CAMV4L2 || defined HAVE_VIDEOIO)

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <libudev.h>
#include <limits>

#include <poll.h>

#ifdef HAVE_CAMV4L2
#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>
#endif

#ifdef HAVE_VIDEOIO
// NetBSD compatibility layer with V4L2
#include <sys/videoio.h>
#endif

#ifdef __OpenBSD__
typedef uint32_t __u32;
#endif

// https://github.com/opencv/opencv/issues/13335
#ifndef V4L2_CID_ISO_SENSITIVITY
#define V4L2_CID_ISO_SENSITIVITY (V4L2_CID_CAMERA_CLASS_BASE+23)
#endif

// https://github.com/opencv/opencv/issues/13929
#ifndef V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_HEIGHT
#define V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_HEIGHT (V4L2_CID_MPEG_BASE+364)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_WIDTH
#define V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_WIDTH (V4L2_CID_MPEG_BASE+365)
#endif

#ifndef V4L2_CID_ROTATE
#define V4L2_CID_ROTATE (V4L2_CID_BASE+34)
#endif
#ifndef V4L2_CID_IRIS_ABSOLUTE
#define V4L2_CID_IRIS_ABSOLUTE (V4L2_CID_CAMERA_CLASS_BASE+17)
#endif

#ifndef V4L2_PIX_FMT_Y10
#define V4L2_PIX_FMT_Y10 v4l2_fourcc('Y', '1', '0', ' ')
#endif

#ifndef V4L2_PIX_FMT_Y12
#define V4L2_PIX_FMT_Y12 v4l2_fourcc('Y', '1', '2', ' ')
#endif

#ifndef V4L2_PIX_FMT_ABGR32
#define V4L2_PIX_FMT_ABGR32  v4l2_fourcc('A', 'R', '2', '4')
#endif
#ifndef V4L2_PIX_FMT_XBGR32
#define V4L2_PIX_FMT_XBGR32  v4l2_fourcc('X', 'R', '2', '4')
#endif

/* Defaults - If your board can do better, set it here.  Set for the most common type inputs. */
#define DEFAULT_V4L_WIDTH  640
#define DEFAULT_V4L_HEIGHT 480
#define DEFAULT_V4L_FPS 30

#define MAX_CAMERAS 8

// default and maximum number of V4L buffers, not including last, 'special' buffer
#define MAX_V4L_BUFFERS 10
#define DEFAULT_V4L_BUFFERS 4
#define MAX_DEVICE_DRIVER_NAME 80 

// if enabled, then bad JPEG warnings become errors and cause NULL returned instead of image
//#define V4L_ABORT_BADJPEG

namespace cv {

static const char* decode_ioctl_code(unsigned long ioctlCode)
{
    switch (ioctlCode)
    {
#define CV_ADD_IOCTL_CODE(id) case id: return #id
    CV_ADD_IOCTL_CODE(VIDIOC_G_FMT);
    CV_ADD_IOCTL_CODE(VIDIOC_S_FMT);
    CV_ADD_IOCTL_CODE(VIDIOC_REQBUFS);
    CV_ADD_IOCTL_CODE(VIDIOC_DQBUF);
    CV_ADD_IOCTL_CODE(VIDIOC_QUERYCAP);
    CV_ADD_IOCTL_CODE(VIDIOC_S_PARM);
    CV_ADD_IOCTL_CODE(VIDIOC_G_PARM);
    CV_ADD_IOCTL_CODE(VIDIOC_QUERYBUF);
    CV_ADD_IOCTL_CODE(VIDIOC_QBUF);
    CV_ADD_IOCTL_CODE(VIDIOC_STREAMON);
    CV_ADD_IOCTL_CODE(VIDIOC_STREAMOFF);
    CV_ADD_IOCTL_CODE(VIDIOC_ENUMINPUT);
    CV_ADD_IOCTL_CODE(VIDIOC_G_INPUT);
    CV_ADD_IOCTL_CODE(VIDIOC_S_INPUT);
    CV_ADD_IOCTL_CODE(VIDIOC_G_CTRL);
    CV_ADD_IOCTL_CODE(VIDIOC_S_CTRL);
#undef CV_ADD_IOCTL_CODE
    }
    return "unknown";
}

/* Device Capture Objects */
/* V4L2 structure */
struct Buffer
{
    void *  start;
    size_t  length;
    // This is dequeued buffer. It used for to put it back in the queue.
    // The buffer is valid only if capture->bufferIndex >= 0
    v4l2_buffer buffer;

    Buffer() : start(NULL), length(0)
    {
        buffer = v4l2_buffer();
    }
};

static int numCameras = 0;
static int indexList = 0;
bool bCurrMode1, bCurrMode2;

struct CvCaptureCAM_V4L CV_FINAL : public CvCapture
{
    int getCaptureDomain() /*const*/ CV_OVERRIDE { return cv::CAP_V4L; }

    int deviceHandle;
    bool v4l_buffersRequested;
    bool v4l_streamStarted;

    int bufferIndex;
    bool FirstCapture;
    String deviceName;

    IplImage frame;

    __u32 palette;
    int width, height;
    int width_set, height_set;
    int bufferSize;
    __u32 fps;
    bool convert_rgb;
    bool frame_allocated;
    bool returnFrame;
    // To select a video input set cv::CAP_PROP_CHANNEL to channel number.
    // If the new channel number is than 0, then a video input will not change
    int channelNumber;
    // Normalize properties. If set parameters will be converted to/from [0,1) range.
    // Enabled by default (as OpenCV 3.x does).
    // Value is initialized from the environment variable `OPENCV_VIDEOIO_V4L_RANGE_NORMALIZED`:
    // To select real parameters mode after devise is open set cv::CAP_PROP_MODE to 0
    // any other value revert the backward compatibility mode (with normalized properties).
    // Range normalization affects the following parameters:
    // cv::CAP_PROP_*: BRIGHTNESS,CONTRAST,SATURATION,HUE,GAIN,EXPOSURE,FOCUS,AUTOFOCUS,AUTO_EXPOSURE.
    bool normalizePropRange;

    /* V4L2 variables */
    Buffer buffers[MAX_V4L_BUFFERS + 1];
    v4l2_capability capability;
    v4l2_input videoInput;
    v4l2_format form;
    v4l2_requestbuffers req;
    v4l2_buf_type type;

    timeval timestamp;

    bool open(int _index);
    bool open(const char* deviceName);
    bool isOpened() const;

    void closeDevice();

    virtual double getProperty(int) const CV_OVERRIDE;
    virtual bool getProperty(int propId, int &min, int &max, int &steppingDelta, int &supportedMode, int &currentValue, int &currentMode, int &defaultValue) CV_OVERRIDE;
    virtual bool setProperty(int, int) CV_OVERRIDE;
    virtual bool setProperty(int propId, int value, int mode);
    virtual bool grabFrame() CV_OVERRIDE;
    virtual bool getDevices(int &) CV_OVERRIDE;
    virtual bool getDeviceInfo(int index, cv::String &deviceName, cv::String& vid, cv::String &pid,cv::String &devicePath) CV_OVERRIDE;
    virtual bool getFormats(int &) CV_OVERRIDE;
    virtual bool getFormatType(int formats, cv::String &formatType, int &width, int &height, int &fps) CV_OVERRIDE;
    virtual bool setFormatType(int) CV_OVERRIDE;
    virtual IplImage* retrieveFrame(int) CV_OVERRIDE;

    CvCaptureCAM_V4L();
    virtual ~CvCaptureCAM_V4L();
    bool requestBuffers();
    bool requestBuffers(unsigned int buffer_number);
    bool createBuffers();
    void releaseBuffers();
    bool initCapture();
    bool streaming(bool startStream);
    bool setFps(int value);
    bool tryIoctl(unsigned long ioctlCode, void *parameter, bool failIfBusy = true, int attempts = 10) const;
    bool controlInfo(int property_id, __u32 &v4l2id, cv::Range &range) const;
    bool icvControl(__u32 v4l2id, int &value, bool isSet) const;

    bool icvSetFrameSize(int _width, int _height);
    bool v4l2_reset();
    bool setVideoInputChannel();
    bool try_palette_v4l2();
    bool try_init_v4l2();
    bool autosetup_capture_mode_v4l2();
    void v4l2_create_frame();
    bool read_frame_v4l2();
    bool convertableToRgb() const;
    void convertToRgb(const Buffer &currentBuffer);
    void releaseFrame();

    bool havePendingFrame;  // true if next .grab() should be noop, .retrive() resets this flag
};

/***********************   Implementations  ***************************************/

CvCaptureCAM_V4L::CvCaptureCAM_V4L() :
    deviceHandle(-1),
    v4l_buffersRequested(false),
    v4l_streamStarted(false),
    bufferIndex(-1),
    FirstCapture(true),
    palette(0),
    width(0), height(0), width_set(0), height_set(0),
    bufferSize(DEFAULT_V4L_BUFFERS),
    fps(0), convert_rgb(0), frame_allocated(false), returnFrame(false),
    channelNumber(-1), normalizePropRange(false),
    type(V4L2_BUF_TYPE_VIDEO_CAPTURE),
    havePendingFrame(false)
{
    frame = cvIplImage();
    memset(&timestamp, 0, sizeof(timestamp));
}

CvCaptureCAM_V4L::~CvCaptureCAM_V4L()
{
    try
    {
        closeDevice();
    }
    catch (...)
    {
        CV_LOG_WARNING(NULL, "VIDEOIO(V4L2): unable properly close device: " << deviceName);
        if (deviceHandle != -1)
            close(deviceHandle);
    }
}

void CvCaptureCAM_V4L::closeDevice()
{
    if (v4l_streamStarted)
        streaming(false);
    if (v4l_buffersRequested)
        releaseBuffers();
    if(deviceHandle != -1)
    {
        CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): close(" << deviceHandle << ")");
        close(deviceHandle);
    }
    deviceHandle = -1;
}

bool CvCaptureCAM_V4L::isOpened() const
{
    return deviceHandle != -1;
}

bool CvCaptureCAM_V4L::try_palette_v4l2()
{
    form = v4l2_format();
    form.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    form.fmt.pix.pixelformat = palette;
    form.fmt.pix.field       = V4L2_FIELD_ANY;
    form.fmt.pix.width       = width;
    form.fmt.pix.height      = height;
    if (!tryIoctl(VIDIOC_S_FMT, &form, true))
    {
        return false;
    }
    return palette == form.fmt.pix.pixelformat;
}

bool CvCaptureCAM_V4L::setVideoInputChannel()
{
    if(channelNumber < 0)
        return true;
    /* Query channels number */
    int channel = 0;
    if (!tryIoctl(VIDIOC_G_INPUT, &channel))
        return false;

    if(channel == channelNumber)
        return true;

    /* Query information about new input channel */
    videoInput = v4l2_input();
    videoInput.index = channelNumber;
    if (!tryIoctl(VIDIOC_ENUMINPUT, &videoInput))
        return false;

    //To select a video input applications store the number of the desired input in an integer
    // and call the VIDIOC_S_INPUT ioctl with a pointer to this integer. Side effects are possible.
    // For example inputs may support different video standards, so the driver may implicitly
    // switch the current standard.
    // It is good practice to select an input before querying or negotiating any other parameters.
    return tryIoctl(VIDIOC_S_INPUT, &channelNumber);
}

bool CvCaptureCAM_V4L::try_init_v4l2()
{
    /* The following code sets the CHANNEL_NUMBER of the video input.  Some video sources
    have sub "Channel Numbers".  For a typical V4L TV capture card, this is usually 1.
    I myself am using a simple NTSC video input capture card that uses the value of 1.
    If you are not in North America or have a different video standard, you WILL have to change
    the following settings and recompile/reinstall.  This set of settings is based on
    the most commonly encountered input video source types (like my bttv card) */

    // The cv::CAP_PROP_MODE used for set the video input channel number
    if (!setVideoInputChannel())
    {
        CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): Unable to set Video Input Channel");
        return false;
    }

    // Test device for V4L2 compatibility
    capability = v4l2_capability();
    if (!tryIoctl(VIDIOC_QUERYCAP, &capability))
    {
        CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): Unable to query capability");
        return false;
    }

    if ((capability.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0)
    {
        /* Nope. */
        CV_LOG_INFO(NULL, "VIDEOIO(V4L2:" << deviceName << "): not supported - device is unable to capture video (missing V4L2_CAP_VIDEO_CAPTURE)");
        return false;
    }
    return true;
}

bool CvCaptureCAM_V4L::autosetup_capture_mode_v4l2()
{
    //in case palette is already set and works, no need to setup.
    if (palette != 0)
    {
        if (try_palette_v4l2())
        {
            return true;
        }
        else if (errno == EBUSY)
        {
            CV_LOG_INFO(NULL, "VIDEOIO(V4L2:" << deviceName << "): device is busy");
            closeDevice();
            return false;
        }
    }
    __u32 try_order[] = {
            V4L2_PIX_FMT_BGR24,
            V4L2_PIX_FMT_RGB24,
            V4L2_PIX_FMT_YVU420,
            V4L2_PIX_FMT_YUV420,
            V4L2_PIX_FMT_YUV411P,
            V4L2_PIX_FMT_YUYV,
            V4L2_PIX_FMT_UYVY,
            V4L2_PIX_FMT_NV12,
            V4L2_PIX_FMT_NV21,
            V4L2_PIX_FMT_SBGGR8,
            V4L2_PIX_FMT_SGBRG8,
            V4L2_PIX_FMT_XBGR32,
            V4L2_PIX_FMT_ABGR32,
            V4L2_PIX_FMT_SN9C10X,
#ifdef HAVE_JPEG
            V4L2_PIX_FMT_MJPEG,
            V4L2_PIX_FMT_JPEG,
#endif
            V4L2_PIX_FMT_Y16,
            V4L2_PIX_FMT_Y12,
            V4L2_PIX_FMT_Y10,
            V4L2_PIX_FMT_GREY,
            V4L2_PIX_FMT_H264      //Included by e-con
    };

    for (size_t i = 0; i < sizeof(try_order) / sizeof(__u32); i++) {
        palette = try_order[i];
        if (try_palette_v4l2()) {
            return true;
        } else if (errno == EBUSY) {
            CV_LOG_INFO(NULL, "VIDEOIO(V4L2:" << deviceName << "): device is busy");
            closeDevice();
            return false;
        }
    }
    return false;
}

bool CvCaptureCAM_V4L::setFps(int value)
{
    if (!isOpened())
        return false;

    v4l2_streamparm streamparm = v4l2_streamparm();
    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    streamparm.parm.capture.timeperframe.numerator = 1;
    streamparm.parm.capture.timeperframe.denominator = __u32(value);
    if (!tryIoctl(VIDIOC_S_PARM, &streamparm) || !tryIoctl(VIDIOC_G_PARM, &streamparm))
    {
        CV_LOG_INFO(NULL, "VIDEOIO(V4L2:" << deviceName << "): can't set FPS: " << value);
        return false;
    }

    CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): FPS="
            << streamparm.parm.capture.timeperframe.denominator << "/"
            << streamparm.parm.capture.timeperframe.numerator);
    fps = streamparm.parm.capture.timeperframe.denominator;  // TODO use numerator
    return true;
}

bool CvCaptureCAM_V4L::convertableToRgb() const
{
    switch (palette) {
    case V4L2_PIX_FMT_YVU420:
    case V4L2_PIX_FMT_YUV420:
    case V4L2_PIX_FMT_NV12:
    case V4L2_PIX_FMT_NV21:
    case V4L2_PIX_FMT_YUV411P:
#ifdef HAVE_JPEG
    case V4L2_PIX_FMT_MJPEG:
    case V4L2_PIX_FMT_JPEG:
#endif
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_UYVY:
    case V4L2_PIX_FMT_SBGGR8:
    case V4L2_PIX_FMT_SN9C10X:
    case V4L2_PIX_FMT_SGBRG8:
    case V4L2_PIX_FMT_RGB24:
    case V4L2_PIX_FMT_Y16:
    case V4L2_PIX_FMT_Y10:
    case V4L2_PIX_FMT_GREY:
    case V4L2_PIX_FMT_BGR24:
    case V4L2_PIX_FMT_XBGR32:
    case V4L2_PIX_FMT_ABGR32:
        return true;
    default:
        break;
    }
    return false;
}

void CvCaptureCAM_V4L::v4l2_create_frame()
{
    CV_Assert(form.fmt.pix.width <= (uint)std::numeric_limits<int>::max());
    CV_Assert(form.fmt.pix.height <= (uint)std::numeric_limits<int>::max());
    CvSize size = {(int)form.fmt.pix.width, (int)form.fmt.pix.height};
    int channels = 3;
    int depth = IPL_DEPTH_8U;

    if (!convert_rgb) {
        switch (palette) {
        case V4L2_PIX_FMT_BGR24:
        case V4L2_PIX_FMT_RGB24:
        case V4L2_PIX_FMT_XBGR32:
        case V4L2_PIX_FMT_ABGR32:
            break;
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_UYVY:
            channels = 2;
            break;
        case V4L2_PIX_FMT_YVU420:
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_NV21:
            channels = 1;
            size.height = size.height * 3 / 2; // "1.5" channels
            break;
        case V4L2_PIX_FMT_Y16:
        case V4L2_PIX_FMT_Y12:
        case V4L2_PIX_FMT_Y10:
            depth = IPL_DEPTH_16U;
            /* fallthru */
        case V4L2_PIX_FMT_GREY:
            channels = 1;
            break;
        case V4L2_PIX_FMT_H264: //Implemented by econsys : for getting variable mat.
            channels = 1;
            size = cvSize(buffers[bufferIndex].buffer.bytesused, 1);
            break;
        case V4L2_PIX_FMT_MJPEG:
        case V4L2_PIX_FMT_JPEG:
        default:
            channels = 1;
            if(bufferIndex < 0)
                size = cvSize(buffers[MAX_V4L_BUFFERS].length, 1);
            else
                size = cvSize(buffers[bufferIndex].buffer.bytesused, 1);
            break;
        }
    }

    /* Set up Image data */
    cvInitImageHeader(&frame, size, depth, channels);

    /* Allocate space for pixelformat we convert to.
     * If we do not convert frame is just points to the buffer
     */
    releaseFrame();
    // we need memory iff convert_rgb is true
    if (convert_rgb) {
        frame.imageData = (char *)cvAlloc(frame.imageSize);
        frame_allocated = true;
    }
}

bool CvCaptureCAM_V4L::initCapture()
{
    if (!isOpened())
        return false;

    if (!try_init_v4l2())
    {
        CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): init failed: errno=" << errno << " (" << strerror(errno) << ")");
        return false;
    }

    /* Find Window info */
    form = v4l2_format();
    form.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (!tryIoctl(VIDIOC_G_FMT, &form))
    {
        CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): Could not obtain specifics of capture window (VIDIOC_G_FMT): errno=" << errno << " (" << strerror(errno) << ")");
        return false;
    }

    if (!autosetup_capture_mode_v4l2())
    {
        if (errno != EBUSY)
        {
            CV_LOG_INFO(NULL, "VIDEOIO(V4L2:" << deviceName << "): Pixel format of incoming image is unsupported by OpenCV");
        }
        return false;
    }
    if(palette==V4L2_PIX_FMT_H264 || palette == V4L2_PIX_FMT_Y12) //Implemented by econsys : Inorder to get variable mat size
         convert_rgb = false;
    /* try to set framerate */
    setFps(fps);

    unsigned int min;

    /* Buggy driver paranoia. */
    min = form.fmt.pix.width * 2;

    if (form.fmt.pix.bytesperline < min)
        form.fmt.pix.bytesperline = min;

    min = form.fmt.pix.bytesperline * form.fmt.pix.height;

    if (form.fmt.pix.sizeimage < min)
        form.fmt.pix.sizeimage = min;

    if (!requestBuffers())
        return false;

    if (!createBuffers()) {
        /* free capture, and returns an error code */
        releaseBuffers();
        return false;
    }

    v4l2_create_frame();

    // reinitialize buffers
    FirstCapture = true;

    return true;
};

bool CvCaptureCAM_V4L::requestBuffers()
{
    unsigned int buffer_number = bufferSize;
    while (buffer_number > 0) {
        if (requestBuffers(buffer_number) && req.count >= buffer_number)
        {
            break;
        }

        buffer_number--;
        CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): Insufficient buffer memory -- decreasing buffers: " << buffer_number);
    }
    if (buffer_number < 1) {
        CV_LOG_WARNING(NULL, "VIDEOIO(V4L2:" << deviceName << "): Insufficient buffer memory");
        return false;
    }
    bufferSize = req.count;
    return true;
}

bool CvCaptureCAM_V4L::requestBuffers(unsigned int buffer_number)
{
    if (!isOpened())
        return false;

    req = v4l2_requestbuffers();
    req.count = buffer_number;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (!tryIoctl(VIDIOC_REQBUFS, &req)) {
        int err = errno;
        if (EINVAL == err)
        {
            CV_LOG_WARNING(NULL, "VIDEOIO(V4L2:" << deviceName << "): no support for memory mapping");
        }
        else
        {
            CV_LOG_WARNING(NULL, "VIDEOIO(V4L2:" << deviceName << "): failed VIDIOC_REQBUFS: errno=" << err << " (" << strerror(err) << ")");
        }
        return false;
    }
    v4l_buffersRequested = true;
    return true;
}

bool CvCaptureCAM_V4L::createBuffers()
{
    size_t maxLength = 0;
    for (unsigned int n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        v4l2_buffer buf = v4l2_buffer();
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        if (!tryIoctl(VIDIOC_QUERYBUF, &buf)) {
            CV_LOG_WARNING(NULL, "VIDEOIO(V4L2:" << deviceName << "): failed VIDIOC_QUERYBUF: errno=" << errno << " (" << strerror(errno) << ")");
            return false;
        }

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start =
            mmap(NULL /* start anywhere */,
                buf.length,
                PROT_READ /* required */,
                MAP_SHARED /* recommended */,
                deviceHandle, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start) {
            CV_LOG_WARNING(NULL, "VIDEOIO(V4L2:" << deviceName << "): failed mmap(" << buf.length << "): errno=" << errno << " (" << strerror(errno) << ")");
            return false;
        }
        maxLength = maxLength > buf.length ? maxLength : buf.length;
    }
    if (maxLength > 0) {
        buffers[MAX_V4L_BUFFERS].start = malloc(maxLength);
        buffers[MAX_V4L_BUFFERS].length = maxLength;
    }
    return buffers[MAX_V4L_BUFFERS].start != 0;
}

/**
 * some properties can not be changed while the device is in streaming mode.
 * this method closes and re-opens the device to re-start the stream.
 * this also causes buffers to be reallocated if the frame size was changed.
 */
bool CvCaptureCAM_V4L::v4l2_reset()
{
    streaming(false);
    releaseBuffers();
    return initCapture();
}

static bool checkForValidNode(String device_node_name)
{
    int cam_fd;
    struct v4l2_capability cam_cap;
    if ((cam_fd = open(device_node_name.c_str(), O_RDWR|O_NONBLOCK, 0)) < 0) {
      perror("Can't open camera device ");
      return false;
    }
    /* Check if the device is capable of streaming */
    if(ioctl(cam_fd, VIDIOC_QUERYCAP, &cam_cap) < 0) {
      perror(" VIDIOC_QUERYCAP failure");
      close(cam_fd);
      return false;
    }
    close(cam_fd);
    if (cam_cap.device_caps & V4L2_CAP_META_CAPTURE) {
      return false;
    }else {
      return true;
    }
}

static void icvInitCapture_V4L() {
    int deviceHandle;
    int CameraNumber;
    char deviceName[MAX_DEVICE_DRIVER_NAME];

    CameraNumber = 0;
    numCameras = 0;
    indexList = 0;
    while(CameraNumber < MAX_CAMERAS) {
      /* Print the CameraNumber at the end of the string with a width of one character */
    sprintf(deviceName, "/dev/video%1d", CameraNumber);
      /* Test using an open to see if this new device name really does exists. */
    deviceHandle = open(deviceName, O_RDONLY);
    if (deviceHandle != -1) {
         /* This device does indeed exist - add it to the total so far */
         // add indexList
         // Checking whether the node is valid node
        if(checkForValidNode(deviceName)){  //!!!!!!!-------------------MODIFIED BY E-CON SYSTEMS----------!!!!!!
            indexList|=(1 << CameraNumber);
            numCameras++;
        }
    }
    if (deviceHandle != -1)
        close(deviceHandle);
    /* Set up to test the next /dev/video source in line */
    CameraNumber++;
    } /* End while */

}; /* End icvInitCapture_V4L */

// bool CvCaptureCAM_V4L::open(int _index)
// {
//     cv::String name;
//     /* Select camera, or rather, V4L video source */
//     if (_index < 0) // Asking for the first device available
//     {
//         for (int autoindex = 0; autoindex < MAX_CAMERAS; ++autoindex)
//         {
//             name = cv::format("/dev/video%d", autoindex);
//             /* Test using an open to see if this new device name really does exists. */
//             int h = ::open(name.c_str(), O_RDONLY);
//             if (h != -1)
//             {
//                 ::close(h);
//                 _index = autoindex;
//                 break;
//             }
//         }
//         if (_index < 0)
//         {
//             CV_LOG_WARNING(NULL, "VIDEOIO(V4L2): can't find camera device");
//             name.clear();
//             return false;
//         }
//     }
//     else
//     {
//         name = cv::format("/dev/video%d", _index);
//     }
//
//     bool res = open(name.c_str());
//     if (!res)
//     {
//         CV_LOG_WARNING(NULL, "VIDEOIO(V4L2:" << deviceName << "): can't open camera by index");
//     }
//     return res;
// }

bool CvCaptureCAM_V4L::open(int _index)
{
   int autoindex = 0;
   char _deviceName[MAX_DEVICE_DRIVER_NAME];
   int count = 0;
   icvInitCapture_V4L(); /* Havent called icvInitCapture yet - do it now! */
   if (!numCameras)
     return false; /* Are there any /dev/video input sources? */

   //search index in indexList
   while(autoindex < 8){
     if((1<<autoindex) & indexList){
        if(count == _index){
          _index = autoindex;
          break;
        }
        count++;
     }
     autoindex++;
   }

   if ( (_index>-1) && ! ((1 << _index) & indexList) )
   {
     _index++;
     if((_index>-1) && ! ((1 << _index) & indexList)){
     fprintf( stderr, "VIDEOIO ERROR: V4L: index %d is not correct!\n",_index);
     return false; /* Did someone ask for not correct video source number? */
     }
   }

   /* Select camera, or rather, V4L video source */
   if (_index<0) { // Asking for the first device available
     for (autoindex = 0; autoindex<MAX_CAMERAS;autoindex++)
    if (indexList & (1<<autoindex))
        break;
     if (autoindex==MAX_CAMERAS)
    return false;
     _index=autoindex;
     autoindex++;// i can recall icvOpenCAM_V4l with index=-1 for next camera
   }

   /* Print the CameraNumber at the end of the string with a width of one character */
   sprintf(_deviceName, "/dev/video%1d", _index);
   return open(_deviceName);
}

bool CvCaptureCAM_V4L::open(const char* _deviceName)
{
    CV_Assert(_deviceName);
    CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << _deviceName << "): opening...");
    FirstCapture = true;
    width = DEFAULT_V4L_WIDTH;
    height = DEFAULT_V4L_HEIGHT;
    width_set = height_set = 0;
    bufferSize = DEFAULT_V4L_BUFFERS;
    fps = DEFAULT_V4L_FPS;
    convert_rgb = true;
    frame_allocated = false;
    deviceName = _deviceName;
    returnFrame = true;
    normalizePropRange = utils::getConfigurationParameterBool("OPENCV_VIDEOIO_V4L_RANGE_NORMALIZED", false);
    channelNumber = -1;
    bufferIndex = -1;

    deviceHandle = ::open(deviceName.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);
    CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << _deviceName << "): deviceHandle=" << deviceHandle);
    if (deviceHandle == -1)
        return false;

    return initCapture();
}

bool CvCaptureCAM_V4L::read_frame_v4l2()
{
    v4l2_buffer buf = v4l2_buffer();
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    while (!tryIoctl(VIDIOC_DQBUF, &buf)) {
        int err = errno;
        if (err == EIO && !(buf.flags & (V4L2_BUF_FLAG_QUEUED | V4L2_BUF_FLAG_DONE))) {
            // Maybe buffer not in the queue? Try to put there
            if (!tryIoctl(VIDIOC_QBUF, &buf))
                return false;
            continue;
        }
        /* display the error and stop processing */
        returnFrame = false;
        CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): can't read frame (VIDIOC_DQBUF): errno=" << err << " (" << strerror(err) << ")");
        return false;
    }

    CV_Assert(buf.index < req.count);
    CV_Assert(buffers[buf.index].length == buf.length);

    //We shouldn't use this buffer in the queue while not retrieve frame from it.
    buffers[buf.index].buffer = buf;
    bufferIndex = buf.index;

    //set timestamp in capture struct to be timestamp of most recent frame
    timestamp = buf.timestamp;
    return true;
}

bool CvCaptureCAM_V4L::tryIoctl(unsigned long ioctlCode, void *parameter, bool failIfBusy, int attempts) const
{
    CV_Assert(attempts > 0);
    CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): tryIoctl(" << deviceHandle << ", "
            << decode_ioctl_code(ioctlCode) << "(" << ioctlCode << "), failIfBusy=" << failIfBusy << ")"
    );
    while (true)
    {
        errno = 0;
        int result = ioctl(deviceHandle, ioctlCode, parameter);
        int err = errno;
        CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): call ioctl(" << deviceHandle << ", "
                << decode_ioctl_code(ioctlCode) << "(" << ioctlCode << "), ...) => "
                << result << "    errno=" << err << " (" << strerror(err) << ")"
        );

        if (result != -1)
            return true;  // success

        const bool isBusy = (err == EBUSY);
        if (isBusy && failIfBusy)
        {
            CV_LOG_INFO(NULL, "VIDEOIO(V4L2:" << deviceName << "): ioctl returns with errno=EBUSY");
            return false;
        }
        if (!(isBusy || errno == EAGAIN))
            return false;

        if (--attempts == 0) {
            return false;
        }

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(deviceHandle, &fds);

        /* Timeout. */
        static int param_v4l_select_timeout = (int)utils::getConfigurationParameterSizeT("OPENCV_VIDEOIO_V4L_SELECT_TIMEOUT", 10);
        struct timeval tv;
        tv.tv_sec = param_v4l_select_timeout;
        tv.tv_usec = 0;

        errno = 0;
        result = select(deviceHandle + 1, &fds, NULL, NULL, &tv);
        err = errno;

        if (0 == result)
        {
            CV_LOG_WARNING(NULL, "VIDEOIO(V4L2:" << deviceName << "): select() timeout.");
            return false;
        }

        CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): select(" << deviceHandle << ") => "
                << result << "   errno = " << err << " (" << strerror(err) << ")"
        );

        if (EINTR == err)  // don't loop if signal occurred, like Ctrl+C
        {
            return false;
        }
    }
    return true;
}

bool CvCaptureCAM_V4L::getDevices(int &devices)
{
    devices = 0;
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *device, *dev_list_entry;
    struct udev_device *dev;
    udev = udev_new();
    if(!udev)
        return false;

    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "video4linux");
    udev_enumerate_scan_devices(enumerate);
    device = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, device)
    {
        const char *path;
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);
        if(checkForValidNode(udev_device_get_devnode(dev)))
        {
            devices++;
        }
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    return true;
}

bool CvCaptureCAM_V4L::getDeviceInfo(int index, String &gdeviceName, String &vid, String &pid, String &devicePath)
{
    int hDescriptor, ret, autoindex=0,count=0;
    struct v4l2_capability querycap;
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev, *pdev;
    udev = udev_new();
    if(!udev)
	       return false;
    icvInitCapture_V4L(); /* Havent called icvInitCapture yet - do it now! */
    while(autoindex < 8){
        if((1<<autoindex) & indexList){
            if(count == index){
                index = autoindex;
                break;
            }
        count++;
        }
        autoindex++;
    }
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "video4linux");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char *path;
  	    path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);
	      pdev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
        if (!pdev)
            return false;
	      devicePath = udev_device_get_devnode(dev);

    	  vid = udev_device_get_sysattr_value(pdev, "idVendor");
    	  pid = udev_device_get_sysattr_value(pdev, "idProduct");
        udev_device_unref(dev);
        if(!(hDescriptor = ::open(devicePath.c_str(), O_RDWR | O_NONBLOCK)))
        {
            return false;
        }

        if((ret = ioctl(hDescriptor, VIDIOC_QUERYCAP, &querycap)) < 0)
        {
            return false;
        }
        if (!(querycap.device_caps & V4L2_CAP_META_CAPTURE)) {
            gdeviceName = (char*)querycap.card;
        }
        if( (devicePath[10] - '0') == index)
            break;
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    return true;
}

bool CvCaptureCAM_V4L::getFormats(int &formats)
{
    int ret;
    formats = 0;

    struct v4l2_fmtdesc fmt;
    struct v4l2_frmsizeenum frmsize;
    struct v4l2_frmivalenum frmival;
    memset(&fmt, 0, sizeof(fmt));
    memset(&frmsize, 0, sizeof(frmsize));
    memset(&frmival, 0, sizeof(frmival));
    fmt.index = 0;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while((ret = ioctl(deviceHandle, VIDIOC_ENUM_FMT, &fmt)) == 0)
    {
        fmt.index++;
        frmsize.index = 0;
        frmsize.pixel_format = fmt.pixelformat;
        while((ret = ioctl(deviceHandle, VIDIOC_ENUM_FRAMESIZES, &frmsize)) == 0)
        {
            if(frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
            {
                frmsize.index++;
                frmival.index = 0;
                frmival.pixel_format = frmsize.pixel_format;
                frmival.width = frmsize.discrete.width;
                frmival.height = frmsize.discrete.height;

                while((ret = ioctl(deviceHandle, VIDIOC_ENUM_FRAMEINTERVALS, &frmival)) == 0)
                {
                    if(frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE)
                    {
                        formats++;
                        frmival.index++;
                    }
                }
            }
        }
    }
    return true;
}

bool CvCaptureCAM_V4L::getFormatType(int formats, cv::String &formatType, int &width, int &height, int &fps)
{
    int ret, vidfmt = 0;
    char pix_fmt[5];
    struct v4l2_fmtdesc fmt;
    struct v4l2_frmsizeenum frmsize;
    struct v4l2_frmivalenum frmival;
    width = height = fps = 0;
    memset(&fmt, 0, sizeof(fmt));
    memset(&frmsize, 0, sizeof(frmsize));
    memset(&frmival, 0, sizeof(frmival));
    fmt.index = 0;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while((ret = ioctl(deviceHandle, VIDIOC_ENUM_FMT, &fmt)) == 0)
    {
        memset(pix_fmt, 0, 5);
        fmt.index++;
        frmsize.index = 0;
        frmsize.pixel_format = fmt.pixelformat;
        pix_fmt[0] = (char)(fmt.pixelformat & 0xff);
        pix_fmt[1] = (char)((fmt.pixelformat >> 8) & 0xff);
        pix_fmt[2] = (char)((fmt.pixelformat >> 16) & 0xff);
        pix_fmt[3] = (char)((fmt.pixelformat >> 24) & 0xff);
        pix_fmt[4] = '\0';
        formatType = pix_fmt;
        while((ret = ioctl(deviceHandle, VIDIOC_ENUM_FRAMESIZES, &frmsize)) == 0)
        {
            if(frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
            {
                width = frmsize.discrete.width;
                height = frmsize.discrete.height;
                frmsize.index++;
                frmival.index = 0;
                frmival.pixel_format = frmsize.pixel_format;
                frmival.width = frmsize.discrete.width;
                frmival.height = frmsize.discrete.height;
                while((ret = ioctl(deviceHandle, VIDIOC_ENUM_FRAMEINTERVALS, &frmival)) == 0)
                {
                    if(frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE)
                    {
                        vidfmt++;
                        frmival.index++;
                        fps = (double)frmival.discrete.denominator / frmival.discrete.numerator;
                        if(formats == vidfmt)
                            goto skipstat;
                    }
                }
            }
        }
    }
    skipstat:    return true;
}

bool CvCaptureCAM_V4L::setFormatType(int index)
{
    int ret, vidfmt = 0, formatId = 0, width = 0, height = 0, fps = 0;
    struct v4l2_fmtdesc fmt;
    struct v4l2_frmsizeenum frmsize;
    struct v4l2_frmivalenum frmival;
    memset(&fmt, 0, sizeof(fmt));
    memset(&frmsize, 0, sizeof(frmsize));
    memset(&frmival, 0, sizeof(frmival));
    fmt.index = 0;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while((ret = ioctl(deviceHandle, VIDIOC_ENUM_FMT, &fmt)) == 0)
    {
        fmt.index++;
        frmsize.index = 0;
        frmsize.pixel_format = fmt.pixelformat;
        formatId = fmt.pixelformat;
        while((ret = ioctl(deviceHandle, VIDIOC_ENUM_FRAMESIZES, &frmsize)) == 0)
        {
            if(frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
            {
                width = frmsize.discrete.width;
                height = frmsize.discrete.height;
                frmsize.index++;
                frmival.index = 0;
                frmival.pixel_format = frmsize.pixel_format;
                frmival.width = frmsize.discrete.width;
                frmival.height = frmsize.discrete.height;
                while((ret = ioctl(deviceHandle, VIDIOC_ENUM_FRAMEINTERVALS, &frmival)) == 0)
                {
                    if(frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE)
                    {
                        vidfmt++;
                        frmival.index++;
                        fps = (double)frmival.discrete.denominator / frmival.discrete.numerator;
                        if(index == vidfmt)
                            goto skipstat;
                    }
                }
            }
        }
    }
    skipstat:
       if(setProperty(CAP_PROP_FOURCC, formatId))
        {
           if(setProperty(CAP_PROP_FRAME_WIDTH, width))
            {
               if(setProperty(CAP_PROP_FRAME_HEIGHT, height))
                {
                   return setProperty(CAP_PROP_FPS, fps);
                }
            }
        }
        return true;
    return false;
}

bool CvCaptureCAM_V4L::grabFrame()
{
    if (havePendingFrame)  // frame has been already grabbed during preroll
    {
        return true;
    }

    if (FirstCapture)
    {
        /* Some general initialization must take place the first time through */

        /* This is just a technicality, but all buffers must be filled up before any
         staggered SYNC is applied.  SO, filler up. (see V4L HowTo) */
        bufferIndex = -1;
        for (__u32 index = 0; index < req.count; ++index) {
            v4l2_buffer buf = v4l2_buffer();

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = index;

            if (!tryIoctl(VIDIOC_QBUF, &buf)) {
                CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): failed VIDIOC_QBUF (buffer=" << index << "): errno=" << errno << " (" << strerror(errno) << ")");
                return false;
            }
        }

        if (!streaming(true)) {
            return false;
        }

        // No need to skip this if the first read returns false
        /* preparation is ok */
        FirstCapture = false;

#if defined(V4L_ABORT_BADJPEG)
        // skip first frame. it is often bad -- this is unnotied in traditional apps,
        //  but could be fatal if bad jpeg is enabled
        if (!read_frame_v4l2())
            return false;
#endif
    }
    // In the case that the grab frame was without retrieveFrame
    if (bufferIndex >= 0)
    {
        if (!tryIoctl(VIDIOC_QBUF, &buffers[bufferIndex].buffer))
        {
            CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): failed VIDIOC_QBUF (buffer=" << bufferIndex << "): errno=" << errno << " (" << strerror(errno) << ")");
        }
    }
    return read_frame_v4l2();
}

/*
 * Turn a YUV4:2:0 block into an RGB block
 *
 * Video4Linux seems to use the blue, green, red channel
 * order convention-- rgb[0] is blue, rgb[1] is green, rgb[2] is red.
 *
 * Color space conversion coefficients taken from the excellent
 * http://www.inforamp.net/~poynton/ColorFAQ.html
 * In his terminology, this is a CCIR 601.1 YCbCr -> RGB.
 * Y values are given for all 4 pixels, but the U (Pb)
 * and V (Pr) are assumed constant over the 2x2 block.
 *
 * To avoid floating point arithmetic, the color conversion
 * coefficients are scaled into 16.16 fixed-point integers.
 * They were determined as follows:
 *
 *  double brightness = 1.0;  (0->black; 1->full scale)
 *  double saturation = 1.0;  (0->greyscale; 1->full color)
 *  double fixScale = brightness * 256 * 256;
 *  int rvScale = (int)(1.402 * saturation * fixScale);
 *  int guScale = (int)(-0.344136 * saturation * fixScale);
 *  int gvScale = (int)(-0.714136 * saturation * fixScale);
 *  int buScale = (int)(1.772 * saturation * fixScale);
 *  int yScale = (int)(fixScale);
 */

/* LIMIT: convert a 16.16 fixed-point value to a byte, with clipping. */
#define LIMIT(x) ((x)>0xffffff?0xff: ((x)<=0xffff?0:((x)>>16)))

static inline void
move_411_block(int yTL, int yTR, int yBL, int yBR, int u, int v,
        int /*rowPixels*/, unsigned char * rgb)
{
    const int rvScale = 91881;
    const int guScale = -22553;
    const int gvScale = -46801;
    const int buScale = 116129;
    const int yScale  = 65536;
    int r, g, b;

    g = guScale * u + gvScale * v;
    //  if (force_rgb) {
    //      r = buScale * u;
    //      b = rvScale * v;
    //  } else {
    r = rvScale * v;
    b = buScale * u;
    //  }

    yTL *= yScale; yTR *= yScale;
    yBL *= yScale; yBR *= yScale;

    /* Write out top two first pixels */
    rgb[0] = LIMIT(b+yTL); rgb[1] = LIMIT(g+yTL);
    rgb[2] = LIMIT(r+yTL);

    rgb[3] = LIMIT(b+yTR); rgb[4] = LIMIT(g+yTR);
    rgb[5] = LIMIT(r+yTR);

    /* Write out top two last pixels */
    rgb += 6;
    rgb[0] = LIMIT(b+yBL); rgb[1] = LIMIT(g+yBL);
    rgb[2] = LIMIT(r+yBL);

    rgb[3] = LIMIT(b+yBR); rgb[4] = LIMIT(g+yBR);
    rgb[5] = LIMIT(r+yBR);
}

// Consider a YUV411P image of 8x2 pixels.
//
// A plane of Y values as before.
//
// A plane of U values    1       2
//                        3       4
//
// A plane of V values    1       2
//                        3       4
//
// The U1/V1 samples correspond to the ABCD pixels.
//     U2/V2 samples correspond to the EFGH pixels.
//
/* Converts from planar YUV411P to RGB24. */
/* [FD] untested... */
static void
yuv411p_to_rgb24(int width, int height,
        unsigned char *pIn0, unsigned char *pOut0)
{
    const int numpix = width * height;
    const int bytes = 24 >> 3;
    int i, j, y00, y01, y10, y11, u, v;
    unsigned char *pY = pIn0;
    unsigned char *pU = pY + numpix;
    unsigned char *pV = pU + numpix / 4;
    unsigned char *pOut = pOut0;

    for (j = 0; j <= height; j++) {
        for (i = 0; i <= width - 4; i += 4) {
            y00 = *pY;
            y01 = *(pY + 1);
            y10 = *(pY + 2);
            y11 = *(pY + 3);
            u = (*pU++) - 128;
            v = (*pV++) - 128;

            move_411_block(y00, y01, y10, y11, u, v,
                    width, pOut);

            pY += 4;
            pOut += 4 * bytes;

        }
    }
}

/*
 * BAYER2RGB24 ROUTINE TAKEN FROM:
 *
 * Sonix SN9C10x based webcam basic I/F routines
 * Takafumi Mizuno <taka-qce@ls-a.jp>
 *
 */
static void bayer2rgb24(long int WIDTH, long int HEIGHT, unsigned char *src, unsigned char *dst)
{
    long int i;
    unsigned char *rawpt, *scanpt;
    long int size;

    rawpt = src;
    scanpt = dst;
    size = WIDTH*HEIGHT;

    for ( i = 0; i < size; i++ ) {
        if ( (i/WIDTH) % 2 == 0 ) {
            if ( (i % 2) == 0 ) {
                /* B */
                if ( (i > WIDTH) && ((i % WIDTH) > 0) ) {
                    *scanpt++ = (*(rawpt-WIDTH-1)+*(rawpt-WIDTH+1)+
                            *(rawpt+WIDTH-1)+*(rawpt+WIDTH+1))/4;  /* R */
                    *scanpt++ = (*(rawpt-1)+*(rawpt+1)+
                            *(rawpt+WIDTH)+*(rawpt-WIDTH))/4;      /* G */
                    *scanpt++ = *rawpt;                                     /* B */
                } else {
                    /* first line or left column */
                    *scanpt++ = *(rawpt+WIDTH+1);           /* R */
                    *scanpt++ = (*(rawpt+1)+*(rawpt+WIDTH))/2;      /* G */
                    *scanpt++ = *rawpt;                             /* B */
                }
            } else {
                /* (B)G */
                if ( (i > WIDTH) && ((i % WIDTH) < (WIDTH-1)) ) {
                    *scanpt++ = (*(rawpt+WIDTH)+*(rawpt-WIDTH))/2;  /* R */
                    *scanpt++ = *rawpt;                                     /* G */
                    *scanpt++ = (*(rawpt-1)+*(rawpt+1))/2;          /* B */
                } else {
                    /* first line or right column */
                    *scanpt++ = *(rawpt+WIDTH);     /* R */
                    *scanpt++ = *rawpt;             /* G */
                    *scanpt++ = *(rawpt-1); /* B */
                }
            }
        } else {
            if ( (i % 2) == 0 ) {
                /* G(R) */
                if ( (i < (WIDTH*(HEIGHT-1))) && ((i % WIDTH) > 0) ) {
                    *scanpt++ = (*(rawpt-1)+*(rawpt+1))/2;          /* R */
                    *scanpt++ = *rawpt;                                     /* G */
                    *scanpt++ = (*(rawpt+WIDTH)+*(rawpt-WIDTH))/2;  /* B */
                } else {
                    /* bottom line or left column */
                    *scanpt++ = *(rawpt+1);         /* R */
                    *scanpt++ = *rawpt;                     /* G */
                    *scanpt++ = *(rawpt-WIDTH);             /* B */
                }
            } else {
                /* R */
                if ( i < (WIDTH*(HEIGHT-1)) && ((i % WIDTH) < (WIDTH-1)) ) {
                    *scanpt++ = *rawpt;                                     /* R */
                    *scanpt++ = (*(rawpt-1)+*(rawpt+1)+
                            *(rawpt-WIDTH)+*(rawpt+WIDTH))/4;      /* G */
                    *scanpt++ = (*(rawpt-WIDTH-1)+*(rawpt-WIDTH+1)+
                            *(rawpt+WIDTH-1)+*(rawpt+WIDTH+1))/4;  /* B */
                } else {
                    /* bottom line or right column */
                    *scanpt++ = *rawpt;                             /* R */
                    *scanpt++ = (*(rawpt-1)+*(rawpt-WIDTH))/2;      /* G */
                    *scanpt++ = *(rawpt-WIDTH-1);           /* B */
                }
            }
        }
        rawpt++;
    }

}

// SGBRG to RGB24
// for some reason, red and blue needs to be swapped
// at least for  046d:092f Logitech, Inc. QuickCam Express Plus to work
//see: http://www.siliconimaging.com/RGB%20Bayer.htm
//and 4.6 at http://tldp.org/HOWTO/html_single/libdc1394-HOWTO/
static void sgbrg2rgb24(long int WIDTH, long int HEIGHT, unsigned char *src, unsigned char *dst)
{
    long int i;
    unsigned char *rawpt, *scanpt;
    long int size;

    rawpt = src;
    scanpt = dst;
    size = WIDTH*HEIGHT;

    for ( i = 0; i < size; i++ )
    {
        if ( (i/WIDTH) % 2 == 0 ) //even row
        {
            if ( (i % 2) == 0 ) //even pixel
            {
                if ( (i > WIDTH) && ((i % WIDTH) > 0) )
                {
                    *scanpt++ = (*(rawpt-1)+*(rawpt+1))/2;       /* R */
                    *scanpt++ = *(rawpt);                        /* G */
                    *scanpt++ = (*(rawpt-WIDTH) + *(rawpt+WIDTH))/2;      /* B */
                } else
                {
                    /* first line or left column */

                    *scanpt++ = *(rawpt+1);           /* R */
                    *scanpt++ = *(rawpt);             /* G */
                    *scanpt++ =  *(rawpt+WIDTH);      /* B */
                }
            } else //odd pixel
            {
                if ( (i > WIDTH) && ((i % WIDTH) < (WIDTH-1)) )
                {
                    *scanpt++ = *(rawpt);       /* R */
                    *scanpt++ = (*(rawpt-1)+*(rawpt+1)+*(rawpt-WIDTH)+*(rawpt+WIDTH))/4; /* G */
                    *scanpt++ = (*(rawpt-WIDTH-1) + *(rawpt-WIDTH+1) + *(rawpt+WIDTH-1) + *(rawpt+WIDTH+1))/4;      /* B */
                } else
                {
                    /* first line or right column */

                    *scanpt++ = *(rawpt);       /* R */
                    *scanpt++ = (*(rawpt-1)+*(rawpt+WIDTH))/2; /* G */
                    *scanpt++ = *(rawpt+WIDTH-1);      /* B */
                }
            }
        } else
        { //odd row
            if ( (i % 2) == 0 ) //even pixel
            {
                if ( (i < (WIDTH*(HEIGHT-1))) && ((i % WIDTH) > 0) )
                {
                    *scanpt++ =  (*(rawpt-WIDTH-1)+*(rawpt-WIDTH+1)+*(rawpt+WIDTH-1)+*(rawpt+WIDTH+1))/4;          /* R */
                    *scanpt++ =  (*(rawpt-1)+*(rawpt+1)+*(rawpt-WIDTH)+*(rawpt+WIDTH))/4;      /* G */
                    *scanpt++ =  *(rawpt); /* B */
                } else
                {
                    /* bottom line or left column */

                    *scanpt++ =  *(rawpt-WIDTH+1);          /* R */
                    *scanpt++ =  (*(rawpt+1)+*(rawpt-WIDTH))/2;      /* G */
                    *scanpt++ =  *(rawpt); /* B */
                }
            } else
            { //odd pixel
                if ( i < (WIDTH*(HEIGHT-1)) && ((i % WIDTH) < (WIDTH-1)) )
                {
                    *scanpt++ = (*(rawpt-WIDTH)+*(rawpt+WIDTH))/2;  /* R */
                    *scanpt++ = *(rawpt);      /* G */
                    *scanpt++ = (*(rawpt-1)+*(rawpt+1))/2; /* B */
                } else
                {
                    /* bottom line or right column */

                    *scanpt++ = (*(rawpt-WIDTH));  /* R */
                    *scanpt++ = *(rawpt);      /* G */
                    *scanpt++ = (*(rawpt-1)); /* B */
                }
            }
        }
        rawpt++;
    }
}

#define CLAMP(x)        ((x)<0?0:((x)>255)?255:(x))

typedef struct {
    int is_abs;
    int len;
    int val;
} code_table_t;


/* local storage */
static code_table_t table[256];
static int init_done = 0;


/*
  sonix_decompress_init
  =====================
    pre-calculates a locally stored table for efficient huffman-decoding.

  Each entry at index x in the table represents the codeword
  present at the MSB of byte x.

 */
static void sonix_decompress_init(void)
{
    int i;
    int is_abs, val, len;

    for (i = 0; i < 256; i++) {
        is_abs = 0;
        val = 0;
        len = 0;
        if ((i & 0x80) == 0) {
            /* code 0 */
            val = 0;
            len = 1;
        }
        else if ((i & 0xE0) == 0x80) {
            /* code 100 */
            val = +4;
            len = 3;
        }
        else if ((i & 0xE0) == 0xA0) {
            /* code 101 */
            val = -4;
            len = 3;
        }
        else if ((i & 0xF0) == 0xD0) {
            /* code 1101 */
            val = +11;
            len = 4;
        }
        else if ((i & 0xF0) == 0xF0) {
            /* code 1111 */
            val = -11;
            len = 4;
        }
        else if ((i & 0xF8) == 0xC8) {
            /* code 11001 */
            val = +20;
            len = 5;
        }
        else if ((i & 0xFC) == 0xC0) {
            /* code 110000 */
            val = -20;
            len = 6;
        }
        else if ((i & 0xFC) == 0xC4) {
            /* code 110001xx: unknown */
            val = 0;
            len = 8;
        }
        else if ((i & 0xF0) == 0xE0) {
            /* code 1110xxxx */
            is_abs = 1;
            val = (i & 0x0F) << 4;
            len = 8;
        }
        table[i].is_abs = is_abs;
        table[i].val = val;
        table[i].len = len;
    }

    init_done = 1;
}


/*
  sonix_decompress
  ================
    decompresses an image encoded by a SN9C101 camera controller chip.

  IN    width
    height
    inp         pointer to compressed frame (with header already stripped)
  OUT   outp    pointer to decompressed frame

  Returns 0 if the operation was successful.
  Returns <0 if operation failed.

 */
static int sonix_decompress(int width, int height, unsigned char *inp, unsigned char *outp)
{
    int row, col;
    int val;
    int bitpos;
    unsigned char code;
    unsigned char *addr;

    if (!init_done) {
        /* do sonix_decompress_init first! */
        return -1;
    }

    bitpos = 0;
    for (row = 0; row < height; row++) {

        col = 0;



        /* first two pixels in first two rows are stored as raw 8-bit */
        if (row < 2) {
            addr = inp + (bitpos >> 3);
            code = (addr[0] << (bitpos & 7)) | (addr[1] >> (8 - (bitpos & 7)));
            bitpos += 8;
            *outp++ = code;

            addr = inp + (bitpos >> 3);
            code = (addr[0] << (bitpos & 7)) | (addr[1] >> (8 - (bitpos & 7)));
            bitpos += 8;
            *outp++ = code;

            col += 2;
        }

        while (col < width) {
            /* get bitcode from bitstream */
            addr = inp + (bitpos >> 3);
            code = (addr[0] << (bitpos & 7)) | (addr[1] >> (8 - (bitpos & 7)));

            /* update bit position */
            bitpos += table[code].len;

            /* calculate pixel value */
            val = table[code].val;
            if (!table[code].is_abs) {
                /* value is relative to top and left pixel */
                if (col < 2) {
                    /* left column: relative to top pixel */
                    val += outp[-2*width];
                }
                else if (row < 2) {
                    /* top row: relative to left pixel */
                    val += outp[-2];
                }
                else {
                    /* main area: average of left pixel and top pixel */
                    val += (outp[-2] + outp[-2*width]) / 2;
                }
            }

            /* store pixel */
            *outp++ = CLAMP(val);
            col++;
        }
    }

    return 0;
}

void CvCaptureCAM_V4L::convertToRgb(const Buffer &currentBuffer)
{
    cv::Size imageSize(form.fmt.pix.width, form.fmt.pix.height);
    // Not found conversion
    switch (palette)
    {
    case V4L2_PIX_FMT_YUV411P:
        yuv411p_to_rgb24(imageSize.width, imageSize.height,
                (unsigned char*)(currentBuffer.start),
                (unsigned char*)frame.imageData);
        return;
    case V4L2_PIX_FMT_SBGGR8:
        bayer2rgb24(imageSize.width, imageSize.height,
                (unsigned char*)currentBuffer.start,
                (unsigned char*)frame.imageData);
        return;

    case V4L2_PIX_FMT_SN9C10X:
        sonix_decompress_init();
        sonix_decompress(imageSize.width, imageSize.height,
                (unsigned char*)currentBuffer.start,
                (unsigned char*)buffers[MAX_V4L_BUFFERS].start);

        bayer2rgb24(imageSize.width, imageSize.height,
                (unsigned char*)buffers[MAX_V4L_BUFFERS].start,
                (unsigned char*)frame.imageData);
        return;
    case V4L2_PIX_FMT_SGBRG8:
        sgbrg2rgb24(imageSize.width, imageSize.height,
                (unsigned char*)currentBuffer.start,
                (unsigned char*)frame.imageData);
        return;
    default:
        break;
    }
    // Converted by cvtColor or imdecode
    cv::Mat destination(imageSize, CV_8UC3, frame.imageData);
    switch (palette) {
    case V4L2_PIX_FMT_YVU420:
        cv::cvtColor(cv::Mat(imageSize.height * 3 / 2, imageSize.width, CV_8U, currentBuffer.start), destination,
                     COLOR_YUV2BGR_YV12);
        return;
    case V4L2_PIX_FMT_YUV420:
        cv::cvtColor(cv::Mat(imageSize.height * 3 / 2, imageSize.width, CV_8U, currentBuffer.start), destination,
                     COLOR_YUV2BGR_IYUV);
        return;
    case V4L2_PIX_FMT_NV12:
        cv::cvtColor(cv::Mat(imageSize.height * 3 / 2, imageSize.width, CV_8U, currentBuffer.start), destination,
                     COLOR_YUV2RGB_NV12);
        return;
    case V4L2_PIX_FMT_NV21:
        cv::cvtColor(cv::Mat(imageSize.height * 3 / 2, imageSize.width, CV_8U, currentBuffer.start), destination,
                     COLOR_YUV2RGB_NV21);
        return;
#ifdef HAVE_JPEG
    case V4L2_PIX_FMT_MJPEG:
    case V4L2_PIX_FMT_JPEG:
        CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): decoding JPEG frame: size=" << currentBuffer.buffer.bytesused);
        if(currentBuffer.buffer.bytesused > 0)
        cv::imdecode(Mat(1, currentBuffer.buffer.bytesused, CV_8U, currentBuffer.start), IMREAD_COLOR, &destination);
        return;
#endif
    case V4L2_PIX_FMT_YUYV:
        cv::cvtColor(cv::Mat(imageSize, CV_8UC2, currentBuffer.start), destination, COLOR_YUV2BGR_YUYV);
        return;
    case V4L2_PIX_FMT_UYVY:
        cv::cvtColor(cv::Mat(imageSize, CV_8UC2, currentBuffer.start), destination, COLOR_YUV2BGR_UYVY);
        return;
    case V4L2_PIX_FMT_RGB24:
        cv::cvtColor(cv::Mat(imageSize, CV_8UC3, currentBuffer.start), destination, COLOR_RGB2BGR);
        return;
    case V4L2_PIX_FMT_Y16:
    {
        cv::Mat temp(imageSize, CV_8UC1, buffers[MAX_V4L_BUFFERS].start);
        cv::Mat(imageSize, CV_16UC1, currentBuffer.start).convertTo(temp, CV_8U, 1.0 / 256);
        cv::cvtColor(temp, destination, COLOR_GRAY2BGR);
        return;
    }
    case V4L2_PIX_FMT_Y12:
    {
        cv::Mat temp(imageSize, CV_8UC1, buffers[MAX_V4L_BUFFERS].start);
        cv::Mat(imageSize, CV_16UC1, currentBuffer.start).convertTo(temp, CV_8U, 1.0 / 16);
        cv::cvtColor(temp, destination, COLOR_GRAY2BGR);
        return;
    }
    case V4L2_PIX_FMT_Y10:
    {
        cv::Mat temp(imageSize, CV_8UC1, buffers[MAX_V4L_BUFFERS].start);
        cv::Mat(imageSize, CV_16UC1, currentBuffer.start).convertTo(temp, CV_8U, 1.0 / 4);
        cv::cvtColor(temp, destination, COLOR_GRAY2BGR);
        return;
    }
    case V4L2_PIX_FMT_GREY:
        cv::cvtColor(cv::Mat(imageSize, CV_8UC1, currentBuffer.start), destination, COLOR_GRAY2BGR);
        break;
    case V4L2_PIX_FMT_H264:
        memcpy((char *)frame.imageData, (char *)currentBuffer.start,
               (int)currentBuffer.buffer.bytesused);
        break;
    case V4L2_PIX_FMT_XBGR32:
    case V4L2_PIX_FMT_ABGR32:
        cv::cvtColor(cv::Mat(imageSize, CV_8UC4, currentBuffer.start), destination, COLOR_BGRA2BGR);
        break;
    case V4L2_PIX_FMT_BGR24:
    default:
        memcpy((char *)frame.imageData, (char *)currentBuffer.start,
               std::min(frame.imageSize, (int)currentBuffer.buffer.bytesused));
        break;
    }
}

static inline cv::String capPropertyName(int prop)
{
    switch (prop) {
    case cv::CAP_PROP_POS_MSEC:
        return "pos_msec";
    case cv::CAP_PROP_POS_FRAMES:
        return "pos_frames";
    case cv::CAP_PROP_POS_AVI_RATIO:
        return "pos_avi_ratio";
    case cv::CAP_PROP_FRAME_COUNT:
        return "frame_count";
    case cv::CAP_PROP_FRAME_HEIGHT:
        return "height";
    case cv::CAP_PROP_FRAME_WIDTH:
        return "width";
    case cv::CAP_PROP_CONVERT_RGB:
        return "convert_rgb";
    case cv::CAP_PROP_FORMAT:
        return "format";
    case cv::CAP_PROP_MODE:
        return "mode";
    case cv::CAP_PROP_FOURCC:
        return "fourcc";
    case cv::CAP_PROP_AUTO_EXPOSURE:
        return "auto_exposure";
    case cv::CAP_PROP_EXPOSURE:
        return "exposure";
    case cv::CAP_PROP_TEMPERATURE:
        return "temperature";
    case cv::CAP_PROP_FPS:
        return "fps";
    case cv::CAP_PROP_BRIGHTNESS:
        return "brightness";
    case cv::CAP_PROP_CONTRAST:
        return "contrast";
    case cv::CAP_PROP_SATURATION:
        return "saturation";
    case cv::CAP_PROP_HUE:
        return "hue";
    case cv::CAP_PROP_GAIN:
        return "gain";
    case cv::CAP_PROP_RECTIFICATION:
        return "rectification";
    case cv::CAP_PROP_MONOCHROME:
        return "monochrome";
    case cv::CAP_PROP_SHARPNESS:
        return "sharpness";
    case cv::CAP_PROP_GAMMA:
        return "gamma";
    case cv::CAP_PROP_TRIGGER:
        return "trigger";
    case cv::CAP_PROP_TRIGGER_DELAY:
        return "trigger_delay";
    case cv::CAP_PROP_WHITE_BALANCE_RED_V:
        return "white_balance_red_v";
    case cv::CAP_PROP_ZOOM:
        return "zoom";
    case cv::CAP_PROP_FOCUS:
        return "focus";
    case cv::CAP_PROP_GUID:
        return "guid";
    case cv::CAP_PROP_ISO_SPEED:
        return "iso_speed";
    case cv::CAP_PROP_BACKLIGHT:
        return "backlight";
    case cv::CAP_PROP_PAN:
        return "pan";
    case cv::CAP_PROP_TILT:
        return "tilt";
    case cv::CAP_PROP_ROLL:
        return "roll";
    case cv::CAP_PROP_IRIS:
        return "iris";
    case cv::CAP_PROP_SETTINGS:
        return "dialog_settings";
    case cv::CAP_PROP_BUFFERSIZE:
        return "buffersize";
    case cv::CAP_PROP_AUTOFOCUS:
        return "autofocus";
    case cv::CAP_PROP_WHITE_BALANCE_BLUE_U:
        return "white_balance_blue_u";
    case cv::CAP_PROP_SAR_NUM:
        return "sar_num";
    case cv::CAP_PROP_SAR_DEN:
        return "sar_den";
    case CAP_PROP_AUTO_WB:
        return "auto wb";
    case CAP_PROP_WB_TEMPERATURE:
        return "wb temperature";
    default:
        return "unknown";
    }
}

static inline int capPropertyToV4L2(int prop)
{
    switch (prop) {
    case cv::CAP_PROP_FPS:
        return -1;
    case cv::CAP_PROP_FOURCC:
        return -1;
    case cv::CAP_PROP_FRAME_COUNT:
        return V4L2_CID_MPEG_VIDEO_B_FRAMES;
    case cv::CAP_PROP_FORMAT:
        return -1;
    case cv::CAP_PROP_MODE:
        return -1;
    case cv::CAP_PROP_BRIGHTNESS:
        return V4L2_CID_BRIGHTNESS;
    case cv::CAP_PROP_CONTRAST:
        return V4L2_CID_CONTRAST;
    case cv::CAP_PROP_SATURATION:
        return V4L2_CID_SATURATION;
    case cv::CAP_PROP_HUE:
        return V4L2_CID_HUE;
    case cv::CAP_PROP_GAIN:
        return V4L2_CID_GAIN;
    case cv::CAP_PROP_EXPOSURE:
        return V4L2_CID_EXPOSURE_ABSOLUTE;
    case cv::CAP_PROP_CONVERT_RGB:
        return -1;
    case cv::CAP_PROP_WHITE_BALANCE_BLUE_U:
       return V4L2_CID_WHITE_BALANCE_TEMPERATURE;
    case cv::CAP_PROP_RECTIFICATION:
        return -1;
    case cv::CAP_PROP_MONOCHROME:
        return -1;
    case cv::CAP_PROP_SHARPNESS:
        return V4L2_CID_SHARPNESS;
    case cv::CAP_PROP_AUTO_EXPOSURE:
        return V4L2_CID_EXPOSURE_AUTO;
    case cv::CAP_PROP_GAMMA:
        return V4L2_CID_GAMMA;
    case cv::CAP_PROP_TEMPERATURE:
        return V4L2_CID_WHITE_BALANCE_TEMPERATURE;
    case cv::CAP_PROP_TRIGGER:
        return -1;
    case cv::CAP_PROP_TRIGGER_DELAY:
        return -1;
    case cv::CAP_PROP_WHITE_BALANCE_RED_V:
        return V4L2_CID_RED_BALANCE;
    case cv::CAP_PROP_ZOOM:
        return V4L2_CID_ZOOM_ABSOLUTE;
    case cv::CAP_PROP_FOCUS:
        return V4L2_CID_FOCUS_ABSOLUTE;
    case cv::CAP_PROP_GUID:
        return -1;
    case cv::CAP_PROP_ISO_SPEED:
        return V4L2_CID_ISO_SENSITIVITY;
    case cv::CAP_PROP_BACKLIGHT:
        return V4L2_CID_BACKLIGHT_COMPENSATION;
    case cv::CAP_PROP_PAN:
        return V4L2_CID_PAN_ABSOLUTE;
    case cv::CAP_PROP_TILT:
        return V4L2_CID_TILT_ABSOLUTE;
    case cv::CAP_PROP_ROLL:
        return V4L2_CID_ROTATE;
    case cv::CAP_PROP_IRIS:
        return V4L2_CID_IRIS_ABSOLUTE;
    case cv::CAP_PROP_SETTINGS:
        return -1;
    case cv::CAP_PROP_BUFFERSIZE:
        return -1;
    case cv::CAP_PROP_AUTOFOCUS:
        return V4L2_CID_FOCUS_AUTO;
    case cv::CAP_PROP_SAR_NUM:
        return V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_HEIGHT;
    case cv::CAP_PROP_SAR_DEN:
        return V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_WIDTH;
    case CAP_PROP_AUTO_WB:
        return V4L2_CID_AUTO_WHITE_BALANCE;
    case CAP_PROP_WB_TEMPERATURE:
        return V4L2_CID_WHITE_BALANCE_TEMPERATURE;
    default:
        break;
    }
    return -1;
}

static inline bool compatibleRange(int property_id)
{
    switch (property_id) {
    case cv::CAP_PROP_BRIGHTNESS:
    case cv::CAP_PROP_CONTRAST:
    case cv::CAP_PROP_SATURATION:
    case cv::CAP_PROP_HUE:
    case cv::CAP_PROP_GAIN:
    case cv::CAP_PROP_EXPOSURE:
    case cv::CAP_PROP_FOCUS:
    case cv::CAP_PROP_SHARPNESS:
    case cv::CAP_PROP_GAMMA:
    case cv::CAP_PROP_WHITE_BALANCE_BLUE_U:
    case cv::CAP_PROP_BACKLIGHT:
    case cv::CAP_PROP_ZOOM:
    case cv::CAP_PROP_PAN:
    case cv::CAP_PROP_TILT:
    case cv::CAP_PROP_AUTOFOCUS:
    case cv::CAP_PROP_AUTO_EXPOSURE:
    case cv::CAP_PROP_AUTO_WB:
        return true;
    default:
        break;
    }
    return false;
}

bool CvCaptureCAM_V4L::controlInfo(int property_id, __u32 &_v4l2id, cv::Range &range) const
{
    /* initialisations */
    int v4l2id = capPropertyToV4L2(property_id);
    v4l2_queryctrl queryctrl = v4l2_queryctrl();
    queryctrl.id = __u32(v4l2id);
    if (v4l2id == -1 || !tryIoctl(VIDIOC_QUERYCTRL, &queryctrl)) {
        CV_LOG_INFO(NULL, "VIDEOIO(V4L2:" << deviceName << "): property " << capPropertyName(property_id) << " is not supported");
        return false;
    }
    _v4l2id = __u32(v4l2id);
    range = cv::Range(queryctrl.minimum, queryctrl.maximum);
    if (normalizePropRange) {
        switch(property_id)
        {
        case CAP_PROP_WB_TEMPERATURE:
        case CAP_PROP_AUTO_WB:
        case CAP_PROP_AUTOFOCUS:
            range = Range(0, 1); // do not convert
            break;
        case CAP_PROP_AUTO_EXPOSURE:
            range = Range(0, 4);
        default:
            break;
        }
    }
    return true;
}

bool CvCaptureCAM_V4L::icvControl(__u32 v4l2id, int &value, bool isSet) const
{
    /* set which control we want to set */
    v4l2_control control = v4l2_control();
    control.id = v4l2id;
    control.value = value;

    /* The driver may clamp the value or return ERANGE, ignored here */
    if (!tryIoctl(isSet ? VIDIOC_S_CTRL : VIDIOC_G_CTRL, &control)) {
        int err = errno;
        CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): failed " << (isSet ? "VIDIOC_S_CTRL" : "VIDIOC_G_CTRL") << ": errno=" << err << " (" << strerror(err) << ")");
        switch (err) {
#ifndef NDEBUG
        case EINVAL:
            fprintf(stderr,
                "The struct v4l2_control id is invalid or the value is inappropriate for the given control (i.e. "
                "if a menu item is selected that is not supported by the driver according to VIDIOC_QUERYMENU).");
            break;
        case ERANGE:
            fprintf(stderr, "The struct v4l2_control value is out of bounds.");
            break;
        case EACCES:
            fprintf(stderr, "Attempt to set a read-only control or to get a write-only control.");
            break;
#endif
        default:
            break;
        }
        return false;
    }
    if (!isSet)
    {
        value = control.value;
    }    
    return true;
}

double CvCaptureCAM_V4L::getProperty(int property_id) const
{
    switch (property_id) {
    case cv::CAP_PROP_FRAME_WIDTH:
        return form.fmt.pix.width;
    case cv::CAP_PROP_FRAME_HEIGHT:
        return form.fmt.pix.height;
    case cv::CAP_PROP_FOURCC:
        return palette;
    case cv::CAP_PROP_FORMAT:
        return CV_MAKETYPE(IPL2CV_DEPTH(frame.depth), frame.nChannels);
    case cv::CAP_PROP_MODE:
        if (normalizePropRange)
            return palette;
        return normalizePropRange;
    case cv::CAP_PROP_CONVERT_RGB:
        return convert_rgb;
    case cv::CAP_PROP_BUFFERSIZE:
        return bufferSize;
    case cv::CAP_PROP_FPS:
    {
        v4l2_streamparm sp = v4l2_streamparm();
        sp.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (!tryIoctl(VIDIOC_G_PARM, &sp)) {
            CV_LOG_WARNING(NULL, "VIDEOIO(V4L2:" << deviceName << "): Unable to get camera FPS");
            return -1;
        }
        return sp.parm.capture.timeperframe.denominator / (double)sp.parm.capture.timeperframe.numerator;
    }
    case cv::CAP_PROP_POS_MSEC:
        if (FirstCapture)
            return 0;

        return 1000 * timestamp.tv_sec + ((double)timestamp.tv_usec) / 1000;
    case cv::CAP_PROP_CHANNEL:
        return channelNumber;
    default:
    {
        cv::Range range;
        __u32 v4l2id;
        if(!controlInfo(property_id, v4l2id, range))
            return -1.0;
        int value = 0;
        if(!icvControl(v4l2id, value, false))
            return -1.0;
        if (normalizePropRange && compatibleRange(property_id))
            return ((double)value - range.start) / range.size();
        return  value;
    }
    }
}

bool CvCaptureCAM_V4L::getProperty(int propId, int &min, int &max, int &steppingDelta, int &supportedMode, int &currentValue, int &currentMode, int &defaultValue)
{
  bool auto_mode = false, manual_mode = false;
  short int auto_mode_value = -1;
  v4l2_format form;
  v4l2_queryctrl queryctrl;
  memset(&form, 0, sizeof(v4l2_format));
  form.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == ioctl (deviceHandle, VIDIOC_G_FMT, &form))
  {
    perror ("VIDIOC_G_FMT");
    return false;
  }

  v4l2_control control;

  if(!((propId == CAP_PROP_WHITE_BALANCE_BLUE_U) || (propId == CAP_PROP_FOCUS) || (propId == CAP_PROP_EXPOSURE)))
  {
    __u32 v4l2id = capPropertyToV4L2(propId);

    if(v4l2id == __u32(-1))
    {
      fprintf(stderr, "VIDEOIO ERROR: V4L2: getting property #%d is not supported\n", propId);
      return false;
    }

    queryctrl = v4l2_queryctrl();
    queryctrl.id = v4l2id;

    if(0 != ioctl(deviceHandle, VIDIOC_QUERYCTRL, &queryctrl))
    {
        if (errno != EINVAL)
          perror ("VIDIOC_QUERYCTRL");
        return false;
    }

    control.id = v4l2id;
    if (-1 == ioctl (deviceHandle, VIDIOC_G_CTRL, &control))
    {
       switch (propId)
       {
          case CAP_PROP_BRIGHTNESS:
           break;
          case CAP_PROP_CONTRAST:
            break;
          case CAP_PROP_SATURATION:
            break;
          case CAP_PROP_HUE:
            break;
          case CAP_PROP_GAIN:
            break;
          case CAP_PROP_SHARPNESS:
            break;
          case CAP_PROP_GAMMA:
            break;
          case CAP_PROP_BACKLIGHT:
            break;
          case CAP_PROP_ZOOM:
            break;
          case CAP_PROP_PAN:
            break;
          case CAP_PROP_TILT:
            break;
       }
       return false;
   }
   manual_mode = true;
 }
 else
 {
   __u32 v4l2id1;
   __u32 v4l2id = capPropertyToV4L2(propId);

  if(propId == CAP_PROP_WHITE_BALANCE_BLUE_U)
    v4l2id1 = capPropertyToV4L2(CAP_PROP_AUTO_WB);
  else if(propId == CAP_PROP_FOCUS)
    v4l2id1 = capPropertyToV4L2(CAP_PROP_AUTOFOCUS);
  else
    v4l2id1 = capPropertyToV4L2(CAP_PROP_AUTO_EXPOSURE);

  if(v4l2id == __u32(-1))
  {
    fprintf(stderr, "VIDEOIO ERROR: V4L2: getting property #%d is not supported\n", propId);
    return false;
  }

  if(v4l2id1 == __u32(-1))
  {
     fprintf(stderr, "VIDEOIO ERROR: V4L2: getting property #%d is not supported\n", propId);
     return false;
  }

  queryctrl = v4l2_queryctrl();
  queryctrl.id = v4l2id1; //Auto control id
  if(0 != ioctl(deviceHandle, VIDIOC_QUERYCTRL, &queryctrl))//querying Auto controls
  {
    if (errno != EINVAL)
      perror ("VIDIOC_QUERYCTRL");
  }

  control.id = v4l2id1; //getting auto controls,if value =0 then manual mode.if value =1 then auto mode
  if (-1 != ioctl (deviceHandle, VIDIOC_G_CTRL, &control))
  {
    auto_mode = true;
    auto_mode_value = control.value;
  }
    queryctrl = v4l2_queryctrl();
    queryctrl.id = v4l2id;

    if(0 != ioctl(deviceHandle, VIDIOC_QUERYCTRL, &queryctrl))
    {
       if (errno != EINVAL)
           perror ("VIDIOC_QUERYCTRL");
    }

    control.id = v4l2id;
    if (-1 != ioctl (deviceHandle, VIDIOC_G_CTRL, &control))
    {
       manual_mode = true;
    }
}

min = queryctrl.minimum;
max = queryctrl.maximum;
currentValue = control.value;
steppingDelta = queryctrl.step;
defaultValue = queryctrl.default_value;

if((auto_mode == true) && (manual_mode == true))
{
supportedMode = 3;
switch(propId)
{
  case CAP_PROP_WHITE_BALANCE_BLUE_U  : (auto_mode_value == 1)?(currentMode = 1) : (currentMode = 2);
                                            break;
  case CAP_PROP_EXPOSURE:             (auto_mode_value == 0)?(currentMode = 1) : (currentMode = 2);
                                          break;
   case CAP_PROP_FOCUS:                (auto_mode_value == 1)?(currentMode = 1) : (currentMode = 2);
                                         break;
  default: printf("\nInvalid control %d",auto_mode_value);
           return false;

}
//(bCurrMode1 == true) ? (currentMode = 1) : (currentMode = 2);

}
else if(auto_mode == true)
{
 currentMode = 1;
 supportedMode = 1;
}
else if(manual_mode == true)
{
 currentMode = 2;
 supportedMode = 2;
}
else
{
  return false;
}
// printf("\nsupportedMode: %d currentMode: %d",supportedMode,currentMode );
    return true;
}

bool CvCaptureCAM_V4L::icvSetFrameSize(int _width, int _height)
{
    if (_width > 0)
        width_set = _width;

    if (_height > 0)
        height_set = _height;

    /* two subsequent calls setting WIDTH and HEIGHT will change
       the video size */
    if (width_set <= 0 || height_set <= 0)
        return true;

    width = width_set;
    height = height_set;
    width_set = height_set = 0;
    return v4l2_reset();
}

bool CvCaptureCAM_V4L::setProperty( int property_id, int _value )
{
    int value = cvRound(_value);
    switch (property_id) {
    case cv::CAP_PROP_FRAME_WIDTH:
        return icvSetFrameSize(value, 0);
    case cv::CAP_PROP_FRAME_HEIGHT:
        return icvSetFrameSize(0, value);
    case cv::CAP_PROP_FPS:
        if (fps == static_cast<__u32>(value))
            return true;
        return setFps(value);
    case cv::CAP_PROP_CONVERT_RGB:
        if (bool(value)) {
            convert_rgb = convertableToRgb();
            return convert_rgb;
        }else{
            convert_rgb = false;
            releaseFrame();
            return true;
        }
    case cv::CAP_PROP_FOURCC:
    {
        if (palette == static_cast<__u32>(value))
            return true;

        __u32 old_palette = palette;
        palette = static_cast<__u32>(value);
        if (v4l2_reset())
            return true;

        palette = old_palette;
        v4l2_reset();
        return false;
    }
    case cv::CAP_PROP_MODE:
        normalizePropRange = bool(value);
        return true;
    case cv::CAP_PROP_BUFFERSIZE:
        if (bufferSize == value)
            return true;

        if (value > MAX_V4L_BUFFERS || value < 1) {
            CV_LOG_WARNING(NULL, "VIDEOIO(V4L2:" << deviceName << "): Bad buffer size " << value << ", buffer size must be from 1 to " << MAX_V4L_BUFFERS);
            return false;
        }
        bufferSize = value;
        return v4l2_reset();
    case cv::CAP_PROP_CHANNEL:
    {
        if (value < 0) {
            channelNumber = -1;
            return true;
        }
        if (channelNumber == value)
            return true;

        int old_channel = channelNumber;
        channelNumber = value;
        if (v4l2_reset())
            return true;

        channelNumber = old_channel;
        v4l2_reset();
        return false;
    }
    default:
    {
        cv::Range range;
        __u32 v4l2id;
        if (!controlInfo(property_id, v4l2id, range))
            return false;
        if (normalizePropRange && compatibleRange(property_id))
            value = cv::saturate_cast<int>(_value * range.size() + range.start);
        return icvControl(v4l2id, value, true);
    }
    }
    return false;
}

bool CvCaptureCAM_V4L::setProperty(int propId, int _value, int mode)
{
    v4l2_control control;
    __u32 v4l2id;
    int value = cvRound(_value);

    if(mode == 2)
    {
      int min,max,steppingDelta,supportedMode,currentValue,currentMode,defaultValue;
      if(!getProperty(propId,min, max,steppingDelta,supportedMode,currentValue,currentMode,defaultValue))//Added by M.Vishnu Murali: if auto control is not supported skip and set manual control.
       {
         printf("\nFailed to get supportedModes.");
         return false;
       }
  	   if(propId == CAP_PROP_WHITE_BALANCE_BLUE_U && supportedMode !=2)
	     {
 	        v4l2id = capPropertyToV4L2(CAP_PROP_AUTO_WB);
	        v4l2_control wcontrol = {v4l2id, 0};

	        if (-1 == ioctl(deviceHandle, VIDIOC_S_CTRL, &wcontrol) && errno != ERANGE)
	        {
   		       perror ("VIDIOC_S_CTRL");
	       	    return false;
	        }
	     }

	     if(propId == CAP_PROP_FOCUS && supportedMode !=2)
	     {
	        v4l2id = capPropertyToV4L2(CAP_PROP_AUTOFOCUS);
	        v4l2_control fcontrol = {v4l2id, 0};

	        if (-1 == ioctl(deviceHandle, VIDIOC_S_CTRL, &fcontrol) && errno != ERANGE)
	        {
	    	      perror ("VIDIOC_S_CTRL");
	       	    return false;
	        }
	     }

	     if(propId == CAP_PROP_EXPOSURE && supportedMode !=2)
	     {
		       // Set Exposure mode as manual
           control.id = capPropertyToV4L2(CAP_PROP_AUTO_EXPOSURE);
           int value = 1;
           v4l2_queryctrl qctrl;
           v4l2_querymenu qmenu;
           qctrl.id = capPropertyToV4L2(CAP_PROP_AUTO_EXPOSURE);
           if (-1 == ioctl(deviceHandle,VIDIOC_QUERYCTRL, &qctrl) && errno != ERANGE)
           {
             perror ("VIDIOC_QUERYCTRL");
             return false;
           }
           int i;
           for (i = qctrl.minimum; i <= qctrl.maximum; i++) {
              qmenu.id = qctrl.id;
              qmenu.index = i;
              if (!(ioctl(deviceHandle,VIDIOC_QUERYMENU, &qmenu)>=0))
                continue;
              if (value-- == 0)
                break;
            }
            control.value = i; // 0 - auto mode , 1- manual mode , 2- shutter priority mode

		       if (-1 == ioctl(deviceHandle, VIDIOC_S_CTRL, &control) && errno != ERANGE)
		       {
			          perror ("VIDIOC_S_CTRL");
			          return false;
		       }

	      }


	       v4l2id = capPropertyToV4L2(propId);

	       if(v4l2id == __u32(-1))
	       {
	          fprintf(stderr, "VIDEOIO ERROR: V4L2: setting property #%d is not supported\n", propId);
	          return -1;
	       }

	       control.id = v4l2id;
         control.value = int(value);

	        if (-1 == ioctl(deviceHandle, VIDIOC_S_CTRL, &control) && errno != ERANGE)
	        {
	           perror ("VIDIOC_S_CTRL");
	           return false;
	        }
	        bCurrMode1 = false;
	        bCurrMode2 = true;
    }
    else
    {
	     if(propId == CAP_PROP_WHITE_BALANCE_BLUE_U)
	     {
     	    v4l2id = capPropertyToV4L2(CAP_PROP_AUTO_WB);
	     }
	     else if(propId == CAP_PROP_EXPOSURE)
	     {
	        v4l2id = capPropertyToV4L2(CAP_PROP_AUTO_EXPOSURE);
	     }
	     else if(propId == CAP_PROP_FOCUS)
	     {
	        v4l2id = capPropertyToV4L2(CAP_PROP_AUTOFOCUS);
	     }

           cv::Range range;
           __u32 v4l2id1;
           if (!controlInfo(propId, v4l2id1, range))
               return false;
           if (normalizePropRange && compatibleRange(propId))
               value = cv::saturate_cast<int>(_value * range.size() + range.start);
           // return icvControl(v4l2id, value, true);

	     if(v4l2id == __u32(-1))
	     {
	        fprintf(stderr, "VIDEOIO ERROR: V4L2: setting property #%d is not supported\n", propId);
	         return -1;
       }

   	/* scale the value we want to set */
    	//value = range.size() + range.start;

	      if((propId == CAP_PROP_WHITE_BALANCE_BLUE_U) || (propId == CAP_PROP_FOCUS))
	          value = 1;
        else
            value = 0 ;
	/* set which control we want to set */
	     v4l2_control bcontrol;
       bcontrol.id= v4l2id;
       bcontrol.value = int(value);

	     if ((-1 == ioctl(deviceHandle, VIDIOC_S_CTRL, &bcontrol) )&& (errno != ERANGE))
	     {
	        perror ("VIDIOC_S_CTRL");
	        return false;
	     }
	     bCurrMode1 = true;
	     bCurrMode2 = false;
    }
    if(control.id == V4L2_CID_EXPOSURE_AUTO && control.value == V4L2_EXPOSURE_MANUAL)
    {
        // update the control range for expose after disabling autoexposure
        // as it is not read correctly at startup
        // TODO check this again as it might be fixed with Linux 4.5
        cv::Range range;
        __u32 v4l2id;
        if(!controlInfo(propId, v4l2id, range))
            return -1.0;
        int value = 0;
        if(!icvControl(v4l2id, value, false))
            return -1.0;
        if (normalizePropRange && compatibleRange(propId))
            return ((double)value - range.start) / range.size();
        return  value;
    }

    return true;
}

void CvCaptureCAM_V4L::releaseFrame()
{
    if (frame_allocated && frame.imageData) {
        cvFree(&frame.imageData);
        frame_allocated = false;
    }
}

void CvCaptureCAM_V4L::releaseBuffers()
{
    releaseFrame();

    if (buffers[MAX_V4L_BUFFERS].start) {
        free(buffers[MAX_V4L_BUFFERS].start);
        buffers[MAX_V4L_BUFFERS].start = 0;
    }

    bufferIndex = -1;
    FirstCapture = true;

    if (!v4l_buffersRequested)
        return;
    v4l_buffersRequested = false;

    for (unsigned int n_buffers = 0; n_buffers < MAX_V4L_BUFFERS; ++n_buffers) {
        if (buffers[n_buffers].start) {
            if (-1 == munmap(buffers[n_buffers].start, buffers[n_buffers].length)) {
                CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): failed munmap(): errno=" << errno << " (" << strerror(errno) << ")");
            } else {
                buffers[n_buffers].start = 0;
            }
        }
    }
    //Applications can call ioctl VIDIOC_REQBUFS again to change the number of buffers,
    // however this cannot succeed when any buffers are still mapped. A count value of zero
    // frees all buffers, after aborting or finishing any DMA in progress, an implicit VIDIOC_STREAMOFF.
    requestBuffers(0);
};

bool CvCaptureCAM_V4L::streaming(bool startStream)
{
    if (startStream != v4l_streamStarted)
    {
        if (!isOpened())
        {
            CV_Assert(v4l_streamStarted == false);
            return !startStream;
        }

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        bool result = tryIoctl(startStream ? VIDIOC_STREAMON : VIDIOC_STREAMOFF, &type);
        if (result)
        {
            v4l_streamStarted = startStream;
            return true;
        }
        if (startStream)
        {
            CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): failed VIDIOC_STREAMON: errno=" << errno << " (" << strerror(errno) << ")");
        }
        return false;
    }
    return startStream;
}

IplImage *CvCaptureCAM_V4L::retrieveFrame(int)
{
    havePendingFrame = false;  // unlock .grab()

    if (bufferIndex < 0)
        return &frame;

    /* Now get what has already been captured as a IplImage return */
    const Buffer &currentBuffer = buffers[bufferIndex];
    if (convert_rgb) {
        if (!frame_allocated)
            v4l2_create_frame();

        convertToRgb(currentBuffer);
    } else {
        // for mjpeg streams the size might change in between, so we have to change the header
        // We didn't allocate memory when not convert_rgb, but we have to recreate the header
        CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): buffer input size=" << currentBuffer.buffer.bytesused);
        if (frame.imageSize != (int)currentBuffer.buffer.bytesused)
            v4l2_create_frame();

        frame.imageData = (char *)buffers[MAX_V4L_BUFFERS].start;
        memcpy(buffers[MAX_V4L_BUFFERS].start, currentBuffer.start,
               std::min(buffers[MAX_V4L_BUFFERS].length, (size_t)currentBuffer.buffer.bytesused));
    }
    //Revert buffer to the queue
    if (!tryIoctl(VIDIOC_QBUF, &buffers[bufferIndex].buffer))
    {
        CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): failed VIDIOC_QBUF: errno=" << errno << " (" << strerror(errno) << ")");
    }

    bufferIndex = -1;
    return &frame;
}

Ptr<IVideoCapture> create_V4L_capture_cam(int index)
{
    cv::CvCaptureCAM_V4L* capture = new cv::CvCaptureCAM_V4L();
    if(index==-1)
    	return makePtr<LegacyCapture>(capture);

    if (capture->open(index))
        return makePtr<LegacyCapture>(capture);

    delete capture;
    return NULL;
}

Ptr<IVideoCapture> create_V4L_capture_file(const std::string &filename)
{
    cv::CvCaptureCAM_V4L* capture = new cv::CvCaptureCAM_V4L();

    if (capture->open(filename.c_str()))
        return makePtr<LegacyCapture>(capture);

    delete capture;
    return NULL;
}

static
bool VideoCapture_V4L_deviceHandlePoll(const std::vector<int>& deviceHandles, std::vector<int>& ready, int64 timeoutNs)
{
    CV_Assert(!deviceHandles.empty());
    const size_t N = deviceHandles.size();

    ready.clear(); ready.reserve(N);

    const auto poll_flags = POLLIN | POLLRDNORM | POLLERR;

    std::vector<pollfd> fds; fds.reserve(N);

    for (size_t i = 0; i < N; ++i)
    {
        int handle = deviceHandles[i];
        CV_LOG_DEBUG(NULL, "camera" << i << ": handle = " << handle);
        CV_Assert(handle != 0);
        fds.push_back(pollfd{handle, poll_flags, 0});
    }

    int timeoutMs = -1;
    if (timeoutNs > 0)
    {
        timeoutMs = saturate_cast<int>((timeoutNs + 999999) / 1000000);
    }

    int ret = poll(fds.data(), N, timeoutMs);
    if (ret == -1)
    {
        perror("poll error");
        return false;
    }

    if (ret == 0)
        return 0;  // just timeout

    for (size_t i = 0; i < N; ++i)
    {
        const auto& fd = fds[i];
        CV_LOG_DEBUG(NULL, "camera" << i << ": fd.revents = 0x" << std::hex << fd.revents);
        if ((fd.revents & (POLLIN | POLLRDNORM)) != 0)
        {
            ready.push_back(i);
        }
        else if ((fd.revents & POLLERR) != 0)
        {
            CV_Error_(Error::StsError, ("Error is reported for camera stream: %d (handle = %d)", (int)i, deviceHandles[i]));
        }
        else
        {
            // not ready
        }
    }
    return true;
}

bool VideoCapture_V4L_waitAny(const std::vector<VideoCapture>& streams, CV_OUT std::vector<int>& ready, int64 timeoutNs)
{
    CV_Assert(!streams.empty());

    const size_t N = streams.size();

    // unwrap internal API
    std::vector<CvCaptureCAM_V4L*> capPtr(N, NULL);
    for (size_t i = 0; i < N; ++i)
    {
        IVideoCapture* iCap = internal::VideoCapturePrivateAccessor::getIVideoCapture(streams[i]);
        LegacyCapture* legacyCapture = dynamic_cast<LegacyCapture*>(iCap);
        CV_Assert(legacyCapture);
        CvCapture* cvCap = legacyCapture->getCvCapture();
        CV_Assert(cvCap);

        CvCaptureCAM_V4L *ptr_CvCaptureCAM_V4L = dynamic_cast<CvCaptureCAM_V4L*>(cvCap);
        CV_Assert(ptr_CvCaptureCAM_V4L);
        capPtr[i] = ptr_CvCaptureCAM_V4L;
    }

    // initialize cameras streams and get handles
    std::vector<int> deviceHandles; deviceHandles.reserve(N);
    for (size_t i = 0; i < N; ++i)
    {
        CvCaptureCAM_V4L *ptr = capPtr[i];
        if (ptr->FirstCapture)
        {
            ptr->havePendingFrame = ptr->grabFrame();
            CV_Assert(ptr->havePendingFrame);
            // TODO: Need to filter these cameras, because frame is available
        }
        CV_Assert(ptr->deviceHandle);
        deviceHandles.push_back(ptr->deviceHandle);
    }

    bool res = VideoCapture_V4L_deviceHandlePoll(deviceHandles, ready, timeoutNs);
    for (size_t i = 0; i < ready.size(); ++i)
    {
        int idx = ready[i];
        CvCaptureCAM_V4L *ptr = capPtr[idx];
        ptr->havePendingFrame = ptr->grabFrame();
        CV_Assert(ptr->havePendingFrame);
    }
    return res;
}

} // cv::

#endif
