#include <initguid.h>
#define Y8_GRABBER_NAME 	L"Y8 Grabber Filter"


// {8203D87A-A020-45a3-8F9A-B221AA719E50}
DEFINE_GUID(IID_Y8GrabberControl,
	0x8203d87a, 0xa020, 0x45a3, 0x8f, 0x9a, 0xb2, 0x21, 0xaa, 0x71, 0x9e, 0x50);

// {D855CBA3-0DA4-4ef2-8099-FAC83B70D9C4}
DEFINE_GUID(CLSID_Y8GrabberFilter, 
0xd855cba3, 0xda4, 0x4ef2, 0x80, 0x99, 0xfa, 0xc8, 0x3b, 0x70, 0xd9, 0xc4);

// {B47B6524-63DB-4d75-960C-036C79E90530}
DEFINE_GUID(MEDIASUBTYPE_Y8, 
0x20203859, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

typedef void(*Y8GrabberCB)(IMediaSample * pSample);

MIDL_INTERFACE("0602BE57-C5A3-46cc-89B7-89B0F116D6B7")
Y8GrabberControl : public IUnknown
{
public:
	virtual HRESULT GetCurrentBuffer(
		LONG *pBufferSize, unsigned char *pBuffer) = 0;

	virtual HRESULT SetCallBack(
		Y8GrabberCB CallBack) = 0;
};


class CY8GrabberFilter : public CTransformFilter,
						public Y8GrabberControl
{
public:

	DECLARE_IUNKNOWN;

	CY8GrabberFilter(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CY8GrabberFilter(void);

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr);

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	HRESULT GetCurrentBuffer(LONG *pBufferSize, unsigned char *pBuffer);
	HRESULT SetCallBack(Y8GrabberCB CallBack);

	// CTransfromFilter overrides
	virtual HRESULT CheckInputType(const CMediaType* mtIn);
	virtual HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
	virtual HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
	virtual HRESULT DecideBufferSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES *pProp);
	virtual HRESULT Transform(IMediaSample * pIn, IMediaSample *pOut);
	virtual HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt);

	HRESULT CopySample(IMediaSample * InSample);
private:
	Y8GrabberCB m_CallBack;
	CCritSec    m_BayerProcessLock;          // Private play critical section
	LONG		m_currentBufferLength;
	unsigned char* m_currentBufferPtr = NULL;

	VIDEOINFOHEADER m_VihIn;
	VIDEOINFOHEADER m_VihOut;
}; // CY8GrabberFilter