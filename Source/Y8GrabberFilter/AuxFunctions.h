typedef class CAuxilaryFunctions
{
public:
	CAuxilaryFunctions();
	~CAuxilaryFunctions();
	BOOL ConvertYtoYVY2(BYTE *pBufIn, BYTE *pBufOut, UINT32 uiImgWidth, UINT32 uiImgHeight, DWORD dwDestStride);
private:
	DWORD m_dwImgCounttoSave;
	DWORD m_uiCurImgWidth;
	DWORD m_uiCurImgHeight;
}CAUXFUNCT, *PCAUXFUNCT;