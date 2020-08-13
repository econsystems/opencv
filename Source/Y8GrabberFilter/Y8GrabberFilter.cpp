// Y8GrabberFilter.cpp : Defines the entry point for the DLL application.
//

#include <windows.h>
#include <streams.h>
#include <initguid.h>

#if (1100 > _MSC_VER)
#include <olectlid.h>
#else
#include <olectl.h>
#endif
#include <Aviriff.h>
#include <strsafe.h>

#include "AuxFunctions.h"
#include "Y8GrabberFilter.h"

#define	DEBUG_ENABLED	0X00
void PrintDebug(BOOL bEnable, LPTSTR szFormat,...);
CAUXFUNCT *pAuxilaryFuncts;

HANDLE m_frameCopiedEvent = NULL;

BOOL bIsFrameCopyEnabled = false;

// Media Types
const AMOVIESETUP_MEDIATYPE sudInputPinTypes[] =   
{ 
	{
		&MEDIATYPE_Video,
		&MEDIASUBTYPE_Y8
	}
};
const AMOVIESETUP_MEDIATYPE sudOutptuPinTypes[] =   
{ 
	{
		&MEDIATYPE_Video, 
		&MEDIASUBTYPE_YUY2
	}
};

// Pins
const AMOVIESETUP_PIN psudPins[] = 
{ 
	{ 
		L"Input", 
		FALSE,
		FALSE, 
		FALSE, 
		FALSE, 
		&CLSID_NULL, 
		NULL,
		1, 
		sudInputPinTypes
	}, 
	{ 
		L"Output", 
		FALSE, 
		TRUE, 
		FALSE, 
		FALSE, 
		&CLSID_NULL,
		NULL, 
		1, 
		sudOutptuPinTypes
	} 
};   

// Filters
const AMOVIESETUP_FILTER sudGrayScaleFilter =
{
    &CLSID_Y8GrabberFilter,		// Filter CLSID
    Y8_GRABBER_NAME ,					// String name
    MERIT_UNLIKELY,							// Filter merit
    2,										// Number of pins
    psudPins								// Pin information
};                   

// Templates
CFactoryTemplate g_Templates[]=
{
	{ 
		Y8_GRABBER_NAME , 
		&CLSID_Y8GrabberFilter,
		CY8GrabberFilter::CreateInstance, 
		NULL, 
		&sudGrayScaleFilter
	}
};

int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);

//
// DllRegisterServer
//
// Handles sample registry and unregistry
//
STDAPI DllRegisterServer()
{
	PrintDebug(DEBUG_ENABLED, L"DllRegisterServer\n");
    return AMovieDllRegisterServer2( TRUE );

}

//
// DllUnregisterServer
//
STDAPI DllUnregisterServer()
{
	PrintDebug(DEBUG_ENABLED, L"DllUnregisterServer\n");
    return AMovieDllRegisterServer2( FALSE );

}


//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), ul_reason_for_call, lpReserved);
}

//
//	Contructor
//
CY8GrabberFilter::CY8GrabberFilter(LPUNKNOWN pUnk, HRESULT *phr):
	CTransformFilter(Y8_GRABBER_NAME , pUnk, CLSID_Y8GrabberFilter)
{
	m_CallBack = NULL;

	if(pAuxilaryFuncts != NULL)
	{
		delete pAuxilaryFuncts;
		pAuxilaryFuncts = NULL;
	}

	pAuxilaryFuncts = new CAuxilaryFunctions();

	m_frameCopiedEvent = CreateEvent(NULL, FALSE, FALSE, L"FrameCopyEvent");
}

CY8GrabberFilter::~CY8GrabberFilter(void)
{
	if(pAuxilaryFuncts != NULL)
	{
		PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter Destructor\n");
		delete pAuxilaryFuncts;
		pAuxilaryFuncts = NULL;
	}
}

CUnknown *CY8GrabberFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr)
{
	ASSERT(pHr);
	CY8GrabberFilter *pNewObject = new CY8GrabberFilter(pUnk, pHr);
	if (pNewObject == NULL)
	{
		if (pHr)
			*pHr = E_OUTOFMEMORY;
	}
	return pNewObject;
}

//CTransformFilter::CheckInputType method is called when the upstream filter 
//proposes a media type to the transform filter
HRESULT CY8GrabberFilter::CheckInputType(const CMediaType* mtIn)
{
	PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter::CheckInputType\n");
	if(((mtIn->majortype != MEDIATYPE_Video) ||
		(mtIn->subtype != MEDIASUBTYPE_Y8) ||
		(mtIn->formattype != FORMAT_VideoInfo) ||
		(mtIn->cbFormat < sizeof(VIDEOINFOHEADER))))
	{
		PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter::CheckInputType: Invalid Media types\n");
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(mtIn->pbFormat);
	DWORD dwImgSize = pVih->bmiHeader.biWidth * pVih->bmiHeader.biHeight;	//Y Component 8 bit image has total size of width x height
	
	if(((pVih->bmiHeader.biBitCount != 8) || ((pVih->bmiHeader.biWidth * pVih->bmiHeader.biHeight) > dwImgSize)))
	{
		PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter::CheckInputType: Invalid Image size\n");
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	return S_OK;
}

//CTransformFilter::GetMediaType method returns one of the filter's 
//preferred output types, referenced by index number.
HRESULT CY8GrabberFilter::GetMediaType(int iPosition, CMediaType *pMediaType)
{
	ASSERT(m_pInput->IsConnected());

	HRESULT hr = S_OK;
	if (iPosition < 0)
	{
		return E_INVALIDARG;
	}

	//Supports YUY2 output.
	if (iPosition == 0)
	{
		PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter::GetMediaType\n");
		hr = m_pInput->ConnectionMediaType(pMediaType);
		if(FAILED(hr))
		{
			PrintDebug(DEBUG_ENABLED, L"ConnectionMediaType Failed\n");
			return hr;
		}
		GUID fccYUYV = FOURCCMap(FCC('YUY2'));
        pMediaType->subtype = fccYUYV;
		pMediaType->SetTemporalCompression(FALSE);

		if(IsEqualGUID(pMediaType->formattype, FORMAT_VideoInfo))
		{
			VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(pMediaType->pbFormat);

			pVih->bmiHeader.biCompression = MAKEFOURCC('Y','U','Y','2');
			pVih->bmiHeader.biBitCount = 16;
			pVih->bmiHeader.biSizeImage = (pVih->bmiHeader.biWidth * abs(pVih->bmiHeader.biHeight) * pVih->bmiHeader.biBitCount) / 8;
			pMediaType->lSampleSize = pVih->bmiHeader.biSizeImage;
		}

		return hr;
	}

	return VFW_S_NO_MORE_ITEMS;
}

//CTransformFilter::CheckTransform method checks if a proposed 
//output type is compatible with the current input type.
HRESULT CY8GrabberFilter::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
	HRESULT hr = S_OK;

	PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter::CheckTransform\n");

	hr = CheckInputType(mtIn);
	if(FAILED(hr))
	{
		PrintDebug(DEBUG_ENABLED, L"Check Input Type Failed\n");
		return hr;
	}

	//Input and Output MAJORTYPE are video.
	if(!IsEqualGUID(mtIn->majortype, mtOut->majortype))
	{
		PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter::CheckTransform: Major Type Invalid\n");
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	//Input format - Y8 or YUY2 and output format - YUYV or RGB24
	if(((mtIn->subtype != MEDIASUBTYPE_Y8) || (mtOut->subtype != MEDIASUBTYPE_YUY2)))
	{
		PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter::CheckTransform: sub Type Invalid\n");
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	//Both input and output format type are FORMAT_VideoInfo
	if(!IsEqualGUID(mtIn->formattype, mtOut->formattype))
	{
		PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter::CheckTransform: Format Type Invalid\n");
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	//Following functions are to be used for Bayer conversion purpose
	SetMediaType(PINDIR_INPUT, mtIn);
	SetMediaType(PINDIR_OUTPUT, mtOut);

	return hr;
}

HRESULT CY8GrabberFilter::DecideBufferSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES *pProp)
{
	PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter::DecideBufferSize\n");
	if (!m_pInput->IsConnected()) 
	{
		return E_UNEXPECTED;
	}

	ALLOCATOR_PROPERTIES InputProps;
	IMemAllocator *pAllocInput = 0;
	HRESULT hr = m_pInput->GetAllocator(&pAllocInput);

	if(FAILED(hr))
	{
		return hr;
	}

	hr = pAllocInput->GetProperties(&InputProps);
	pAllocInput->Release();

	if (FAILED(hr)) 
	{
		return hr;
	}

	if (pProp->cbAlign == 0)
	{
		pProp->cbAlign = 1;
	}

	if (pProp->cbBuffer == 0)
	{
		pProp->cBuffers = 1;
	}

	//Following code is used to get and set the output pin buffer size.
	AM_MEDIA_TYPE mt;
    hr = m_pOutput->ConnectionMediaType(&mt);
    if (FAILED(hr))
    {
        return hr;
    }
    ASSERT(mt.formattype == FORMAT_VideoInfo);
    BITMAPINFOHEADER *pbmi = HEADER(mt.pbFormat);
	pProp->cbBuffer = (pbmi->biWidth * abs(pbmi->biHeight) * pbmi->biBitCount) / 8;
	// Release the format block.
    FreeMediaType(mt);

	ALLOCATOR_PROPERTIES Actual;
	hr = pAlloc->SetProperties(pProp, &Actual);
	if (FAILED(hr)) 
	{
		return hr;
	}
	
	PrintDebug(DEBUG_ENABLED, L"pProp->cbAlign %d, pProp->cbBuffer %d, pProp->cbPrefix %d, pProp->cBuffers %d\n", pProp->cbAlign, pProp->cbBuffer, pProp->cbPrefix, pProp->cBuffers);
	PrintDebug(DEBUG_ENABLED, L"Actual->cbAlign %d, Actual->cbBuffer %d, Actual->cbPrefix %d, Actual->cBuffers %d\n", Actual.cbAlign, Actual.cbBuffer, Actual.cbPrefix, Actual.cBuffers);

	if (InputProps.cbBuffer > Actual.cbBuffer) 
	{
		return E_FAIL;
	}

	return S_OK;
}

//CTransformFilter::Transform takes a pointer to the input sample and 
//a pointer to the output sample. Before the filter calls the method, 
//it copies the sample properties from the input sample to the output sample, including the time stamps.
HRESULT CY8GrabberFilter::Transform(IMediaSample* pIn, IMediaSample* pOut)
{
	HRESULT hr = S_OK;

	LONG len = 0;
	unsigned char *buf = NULL;
	   
	PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter::Transform\n");

	BYTE *pBufIn, *pBufOut, *MyBuff;
	DWORD sizeOut = (m_VihOut.bmiHeader.biWidth * abs(m_VihOut.bmiHeader.biHeight) * 2);

	if (m_CallBack)
	{
		m_CallBack(pIn);
	}

	hr = pIn->GetPointer(&pBufIn);
	if(FAILED(hr))
	{
		return hr;
	}

	PrintDebug(DEBUG_ENABLED, L"Input Sample size : %d", pIn->GetActualDataLength());

	hr = CopySample(pIn);
	if (FAILED(hr))
	{
		PrintDebug(DEBUG_ENABLED, L"Transform :: Sample copy failed.....\n");
	}

	buf = (unsigned char *)malloc(sizeof(unsigned char) *pIn->GetActualDataLength());
	hr = GetCurrentBuffer(&len, buf);
	if (FAILED(hr))
	{
		PrintDebug(DEBUG_ENABLED, L"Transform :: Get current buffer failed.....\n");
	}
	else
	{
		PrintDebug(DEBUG_ENABLED, L"Transform : Copied Buff Length - %d \n", len);
		free(buf);
		buf = NULL;
	}


	hr = pOut->GetPointer(&pBufOut);
	if(FAILED(hr))
	{
		return hr;
	}

	if(m_VihOut.bmiHeader.biCompression == MAKEFOURCC('Y','U','Y','2'))
	{
		//We will receive Y input buffer. Copy Y to output buffer with U and V as 80.
		PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter: YUV Output required\n");
		if(pAuxilaryFuncts == NULL)
			pAuxilaryFuncts = new CAuxilaryFunctions();
		
		pAuxilaryFuncts->ConvertYtoYVY2(pBufIn, pBufOut, m_VihIn.bmiHeader.biWidth, m_VihIn.bmiHeader.biHeight, m_VihOut.bmiHeader.biWidth);
	}

	hr = pOut->SetActualDataLength(sizeOut);
	if(FAILED(hr))
	{
		PrintDebug(1, L"Setting output pin buffer size Failed\n");
		return hr;
	}

	PrintDebug(DEBUG_ENABLED, L"Output Sample size : %d", pOut->GetActualDataLength());


	hr = pOut->SetSyncPoint(TRUE);
	return hr;
}


HRESULT CY8GrabberFilter::SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt)
{
	ASSERT(pmt->formattype == FORMAT_VideoInfo);

	if (direction == PINDIR_INPUT)
	{
		VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmt->pbFormat;
		CopyMemory(&m_VihIn, pVih, sizeof(VIDEOINFOHEADER));
	}
	else   // Output pin
	{
		VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmt->pbFormat;
		CopyMemory(&m_VihOut, pVih, sizeof(VIDEOINFOHEADER));
	}

	return S_OK;
}


HRESULT CY8GrabberFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_Y8GrabberControl)
		return GetInterface((Y8GrabberControl *)this, ppv);
	else
		return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
}

//----------------------------------------------------------------------------
// GetCurrentBuffer - added by Murali.
//----------------------------------------------------------------------------
HRESULT CY8GrabberFilter::SetCallBack(Y8GrabberCB CallBack)
{
	PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter::SetCallBack\n");

	if (CallBack == NULL)
	{
		PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter::SetCallBack Failed..\n");
		return E_FAIL;
	}
	m_CallBack = CallBack;
	return S_OK;
}

//----------------------------------------------------------------------------
// GetCurrentBuffer - added by Murali.
//----------------------------------------------------------------------------
HRESULT CY8GrabberFilter::GetCurrentBuffer(LONG *pBufferSize, unsigned char *pBuffer)
{
	PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter::GetCurrentBuffer\n");

	bIsFrameCopyEnabled = true;

	DWORD dWait = WaitForSingleObject(m_frameCopiedEvent, 50);

	if (dWait == WAIT_TIMEOUT)
	{
		return E_FAIL;
	}
	else
	{
		if (m_currentBufferPtr)
		{

			*pBufferSize = m_currentBufferLength;

			memcpy(pBuffer, m_currentBufferPtr, m_currentBufferLength);

			PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter::  Inside buffer clear\n");

			free(m_currentBufferPtr);
			m_currentBufferPtr = NULL;

			return S_OK;
		}
	}
	return S_OK;
}

//----------------------------------------------------------------------------
// CopySample - added by Murali.
//----------------------------------------------------------------------------
HRESULT CY8GrabberFilter::CopySample(IMediaSample * InSample)
{
	HRESULT hr = E_FAIL;
	
	PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter::  Inside copy sample\n");

	if (!InSample)
		return E_FAIL;

	if (bIsFrameCopyEnabled)
	{
		bIsFrameCopyEnabled = false;

		PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter::  Inside buffer fill\n");

		unsigned char* buffer = NULL;

		m_currentBufferLength = InSample->GetActualDataLength();

		PrintDebug(DEBUG_ENABLED, L"CY8GrabberFilter::  m_currentBufferLength : %d --\n", m_currentBufferLength);

		m_currentBufferPtr = (unsigned char *)malloc(sizeof(unsigned char) *m_currentBufferLength);

		hr = InSample->GetPointer(&buffer);

		memcpy(m_currentBufferPtr, buffer, m_currentBufferLength);

		SetEvent(m_frameCopiedEvent);

		return hr;
	}
	else
		return S_FALSE;
}


void PrintDebug(BOOL bEnable, LPTSTR szFormat,...)
{
	if(bEnable)
	{
		static TCHAR szBuffer[2048]={0};
		const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
		const int LASTCHAR = NUMCHARS - 1;

		// Format the input string
		va_list pArgs;
		va_start(pArgs, szFormat);

		// Use a bounded buffer size to prevent buffer overruns.  Limit count to
		// character size minus one to allow for a NULL terminating character.
		HRESULT hr = StringCchVPrintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
		va_end(pArgs);

		// Ensure that the formatted string is NULL-terminated
		szBuffer[LASTCHAR] = TEXT('\0');
		
		OutputDebugStringW(szBuffer);
	}
}