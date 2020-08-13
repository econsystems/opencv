#include <windows.h>
#include <malloc.h>
#include "AuxFunctions.h"

CAuxilaryFunctions::CAuxilaryFunctions()
{
	m_dwImgCounttoSave = 0;
	m_uiCurImgWidth = 0;
	m_uiCurImgHeight = 0;
}

CAuxilaryFunctions::~CAuxilaryFunctions()
{
}

BOOL CAuxilaryFunctions::ConvertYtoYVY2(BYTE *pBufIn, BYTE *pBufOut, UINT32 uiImgWidth, UINT32 uiImgHeight, DWORD dwDestStride)
{
	DWORD i = 0, j = 0, m = 0;

	DWORD stride = dwDestStride * 2;

	m_uiCurImgWidth = uiImgWidth;
	m_uiCurImgHeight = uiImgHeight;
	
	//PrintDebug(1, L"Width %d Height %d Stride %d\n", uiImgWidth, uiImgHeight, stride);

	if((pBufIn != NULL) && (pBufOut != NULL))
	{
		for(j = 0; j < uiImgHeight; j ++)
		{
			for(i = 0; i < stride; i += 4)
			{
				if(i < (uiImgWidth * 2))
				{
					pBufOut[(stride * j) + i] = pBufIn[m];
					pBufOut[(stride * j) + i + 1]  = 0X80;
					pBufOut[(stride * j) + i + 2] = pBufIn[m + 1];
					pBufOut[(stride * j) + i + 3] = 0X80;
					m += 2;
				}
			}
		}
	}
	return TRUE;
}

