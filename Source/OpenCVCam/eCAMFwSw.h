// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the ECAMFWSW_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// ECAMFWSW_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef ECAMFWSW_EXPORTS
#define ECAMFWSW_API __declspec(dllexport)
#else
#define ECAMFWSW_API __declspec(dllimport)
#endif

BOOL InitExtensionUnit(TCHAR *USBInstanceID);

BOOL DeinitExtensionUnit();

BOOL ReadFirmwareVersion (UINT8 *pMajorVersion, UINT8 *pMinorVersion1, UINT16 *pMinorVersion2, UINT16 *pMinorVersion3);

BOOL GetCameraUniqueID(TCHAR *szUniqueID);
