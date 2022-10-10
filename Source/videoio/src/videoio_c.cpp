// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.

#include "precomp.hpp"
#include "opencv2/videoio/registry.hpp"
#include "videoio_registry.hpp"

using namespace cv;

// Legacy C-like API

CV_IMPL CvCapture* cvCreateCameraCapture(int)
{
    CV_LOG_WARNING(NULL, "cvCreateCameraCapture doesn't support legacy API anymore.")
    return NULL;
}

CV_IMPL CvCapture* cvCreateFileCaptureWithPreference(const char*, int)
{
    CV_LOG_WARNING(NULL, "cvCreateFileCaptureWithPreference doesn't support legacy API anymore.")
    return NULL;
}

CV_IMPL CvCapture* cvCreateFileCapture(const char * filename)
{
    return cvCreateFileCaptureWithPreference(filename, CAP_ANY);
}

CV_IMPL CvVideoWriter* cvCreateVideoWriter(const char*, int, double, CvSize, int)
{
    CV_LOG_WARNING(NULL, "cvCreateVideoWriter doesn't support legacy API anymore.")
    return NULL;
}

CV_IMPL int cvWriteFrame(CvVideoWriter* writer, const IplImage* image)
{
    return writer ? writer->writeFrame(image) : 0;
}

CV_IMPL void cvReleaseVideoWriter(CvVideoWriter** pwriter)
{
    if( pwriter && *pwriter )
    {
        delete *pwriter;
        *pwriter = 0;
    }
}

CV_IMPL void cvReleaseCapture(CvCapture** pcapture)
{
    if (pcapture && *pcapture)
    {
        delete *pcapture;
        *pcapture = 0;
    }
}

CV_IMPL IplImage* cvQueryFrame(CvCapture* capture)
{
    if (!capture)
        return 0;
    if (!capture->grabFrame())
        return 0;
    return capture->retrieveFrame(0);
}

CV_IMPL int cvGrabFrame(CvCapture* capture)
{
    return capture ? capture->grabFrame() : 0;
  }

  /*!!!!!!!-------------------ADDED BY E-CON SYSTEMS----------!!!!!!!! */


  CV_IMPL bool cvGetFormats(CvCapture* capture, int &formats)
  {
      return capture ? capture->getFormats(formats) : 0;
  }

  CV_IMPL bool cvGetFormatType(CvCapture* capture, int formats, String &formatType, int &width, int &height, int &fps)
  {
      return capture ? capture->getFormatType(formats, formatType, width, height, fps) : 0;
  }

  CV_IMPL bool cvSetFormatType(CvCapture* capture, int index)
  {
      return capture ? capture->setFormatType(index) : 0;
  }

  CV_IMPL bool cvSetVideoProperty(CvCapture* capture, int id, int value, int mode)
  {
      return capture ? capture->setProperty(id, value, mode) : 0;
  }


  CV_IMPL CvCapture * cvGetDevices(CvCapture* capture,int &devices)
  {
      int pref = 0;
      // CvCapture *capture = 0;

      switch (pref)
      {
      default:
          if (pref)
              break;

      case CAP_VFW: // or CAP_V4L or CAP_V4L2

  #if defined HAVE_LIBV4L || defined HAVE_CAMV4L || defined HAVE_CAMV4L2 || defined HAVE_VIDEOIO
          if(capture)
          {
            capture->getDevices(devices);
          }
          return capture;
  #endif
              if (pref) break; // CAP_VFW or CAP_V4L or CAP_V4L2
      }
      return capture;
  }

  CV_IMPL CvCapture * cvGetDeviceInfo(CvCapture* capture,int index, String &deviceName, String &vid, String &pid, String &devicePath)
  {

      int pref = 0;
      // CvCapture *capture = 0;
      switch (pref)
      {
      default:
          if (pref)    break;

      case CAP_VFW: // or CAP_V4L or CAP_V4L2
  #if defined HAVE_LIBV4L || defined HAVE_CAMV4L || defined HAVE_CAMV4L2 || defined HAVE_VIDEOIO
          if(capture)
            capture->getDeviceInfo(index, deviceName, vid, pid, devicePath);
          return capture;
  #endif
              if (pref)    break; //CAP_VFW  or CAP_V4L or CAP_V4L2
      }
      return capture;
      //return capture ? capture->getDeviceInfo(index, deviceName, vid, pid, devicePath) : 0;
  }

  /*!!!!!!!---------------------------END-----------------------!!!!!!!! */

CV_IMPL IplImage* cvRetrieveFrame(CvCapture* capture, int idx)
{
    return capture ? capture->retrieveFrame(idx) : 0;
}

CV_IMPL double cvGetCaptureProperty(CvCapture* capture, int id)
{
    return capture ? capture->getProperty(id) : 0;
}

CV_IMPL int cvSetCaptureProperty(CvCapture* capture, int id, double value)
{
    return capture ? capture->setProperty(id, value) : 0;
}

CV_IMPL int cvGetCaptureDomain(CvCapture* capture)
{
    return capture ? capture->getCaptureDomain() : 0;
}
