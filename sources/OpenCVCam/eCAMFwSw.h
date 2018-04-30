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
			
#define VID					L"2560"
#define SEE3CAM_10CUG		L"C110"
#define SEE3CAM_10CUG_C		L"C111"
#define See3CAM_11CUG		L"C112"
#define See3CAM_AR0130		L"C113"
#define See3CAM_10633		L"D110"
#define See3CAM_80			L"C080"
#define See3CAM_81			L"C081" //See3Cam_81 included
#define See3CAM_CU50		L"C151"
#define See3CAM_CU51		L"C152"
#define See3CAM_CU130		L"C1D0"
#define OV4682_RGB_IR_PID	L"C140"
#define See3CAM_CU30		L"C130"
#define STEREO_CAMERA		L"C114"
#define eCAM511_USB			L"C05B"
#define See3CAM_130AF		L"C0D0"
#define See3CAM_30			L"C030"
#define See3CAM_CU135		L"C1D1"
#define See3CAM_CU20		L"C120"

enum e_GPIONumber
{
	Output80MP = 0,
	Input1_80MP = 1,
	Input2_80MP = 2,
	Output1CU50 = 3,
	Output2CU50 = 4,
	Input1CU50 = 5,
	Input2CU50 = 6,
	OutputAR0130 = 7,
	//See3CAM_81
	Output1_81 = 8,
	Output2_81 = 9,
	Input1_81 = 10,
	Input2_81 = 11
};

BOOL InitExtensionUnit(TCHAR *USBInstanceID);

BOOL DeinitExtensionUnit();

BOOL ReadFirmwareVersion (UINT8 *pMajorVersion, UINT8 *pMinorVersion1, UINT16 *pMinorVersion2, UINT16 *pMinorVersion3);

BOOL GetCameraUniqueID(TCHAR *szUniqueID);

BOOL EnableMasterMode();

BOOL EnableTriggerMode();

BOOL CameraRegWrite(UINT8 CamI2CDevice,UINT8 CamRegAddrSize, UINT8 *pCamRegAddr, UINT8 CamRegDataSize, UINT8 *pCamRegData);

BOOL CameraRegRead(UINT8 CamI2CDevice,UINT8 CamRegAddrSize, UINT8 *pCamRegAddr, UINT8 CamRegDataSize, UINT8 *pCamRegData);

BOOL GetFocusMode (UINT8 *FocusMode);

UINT8 SetFocusMode (UINT8 FocusMode);

BOOL GetFocusPosition (UINT16 *FocusPosition);

BOOL SetFocusPosition (UINT16 FocusPosition);

BOOL GetFocusStatus (UINT8 *FocusStatus);

BOOL GetGpioLevel (UINT8 GpioPin,UINT8 *GpioValue);

BOOL SetGpioLevel (UINT8 GpioPin,UINT8 GpioValue);

BOOL GetFlash(UINT8 *Value);

BOOL SetFlash(UINT8 Value);

BOOL GetTorch(UINT8 *Value);

BOOL SetTorch(UINT8 Value);

BOOL GetInputGpioLevel (UINT8 uGpioPin,UINT8 *uGpioValue);

BOOL GetWhiteBalanceMode (UINT8 *uWhiteBalanceMode);

UINT8 SetWhiteBalanceMode (UINT8 uWhiteBalanceMode);

BOOL GetManualWhiteBalanceGain(UINT8 uManualWhiteBalGain,UINT8 *uWhiteBalanceGain);

BOOL SetManualWhiteBalanceGain(UINT8 uManualWhiteBalGain,UINT8 WhiteBalanceGain);

BOOL GetDefaultWhiteBalance(UINT8 *uWhiteBalanceMode, UINT8 *uManualWhiteBalanceRedGain, UINT8 *uManualWhiteBalanceGreenGain, UINT8 *uManualWhiteBalanceBlueGain);

UINT8 EnableCroppedVGAMode();

UINT8 EnableBinnedVGAMode();


#pragma region See3CAM_CU50
//See3CAM_CU50
BOOL GetFlashCU50(UINT8 *uValue);

BOOL SetFlashCU50(UINT8 uValue);

BOOL GetTorchCU50(UINT8 *uValue);

BOOL SetTorchCU50(UINT8 uValue);

#pragma endregion See3CAM_CU50


#pragma region See3CAM_CU51
//See3CAM_CU51
BOOL GetTorchCU51(UINT8 *uValue);

BOOL SetTorchCU51(UINT8 uValue);

BOOL GetManualExposureValue(UINT16 *ManualExposureValue);

BOOL SetManualExposureValue(UINT16 ManualExposureValue);

#pragma endregion See3CAM_CU51

#pragma region See3CAM_CU30
//See3CAM_CU30
BOOL GetSpecialEffectsCU30 (UINT8 *uEffectsMode);

BOOL SetSpecialEffectsCU30 (UINT8 uEffectsMode);

BOOL GetDenoiseValueCU30 (UINT8 *uDenoiseValue);

BOOL SetDenoiseValueCU30 (UINT8 uDenoiseValue);

BOOL GetSceneModeCU30(UINT8 *uSceneMode);

BOOL SetSceneModeCU30(UINT8 uSceneMode);

BOOL GetExpRoiModeCU30(UINT8 *uExpRoiMode, UINT8 *uXPos, UINT8 *uYPos, UINT8 *uWindowSize);

BOOL SetExpRoiModeCU30(UINT8 uExpRoiMode, UINT8 uXPos, UINT8 uYPos, UINT8 uWindowSize);

BOOL GetBurstLengthCU30(UINT8 *uBurstLength);

BOOL SetBurstLengthCU30(UINT8 uBurstLength);

BOOL GetQFactorCU30(UINT8 *uQFactorVal);

BOOL SetQFactorCU30(UINT8 uQFactorVal);

BOOL RestoreDefaultCU30();

BOOL GetFlipModeCU30(UINT8 *uFlipMode);

BOOL SetFlipModeCU30(UINT8 uFlipMode);

BOOL GetFaceDetectionRectCU30(UINT8 *uState, UINT8 *uStatusStructState, UINT8 *uOverlayRectState);

BOOL SetFaceDetectionRectCU30(UINT8 uState, UINT8 uStatusStructState, UINT8 uOverlayRectState);

BOOL GetSmileDetectionStateCU30(UINT8 *uState, UINT8 *uThresholdVal, UINT8 *uEmbedDataState,UINT8 *uSmileTrigger);

BOOL SetSmileDetectionStateCU30(UINT8 uState, UINT8 uThresholdVal, UINT8 uEmbedDataState,UINT8 uSmileTrigger);

BOOL GetExposureCompensationCU30(UINT32 *iExposureComp);

BOOL SetExposureCompensationCU30(UINT32 iExposureComp);

BOOL GetFrameRateValueCU30(UINT8 *uFrameRateVal);

BOOL SetFrameRateValueCU30(UINT8 uFrameRateVal);

BOOL GetFlashCU30(UINT8 *uFlashValue);

BOOL SetFlashCU30(UINT8 uFlashValue);

#pragma endregion See3CAM_CU30

#pragma region See3CAM_CU40
//See3CAM_CU40
BOOL GetFlashCU40(UINT8 *uValue);

BOOL SetFlashCU40(UINT8 uValue);

#pragma endregion See3CAM_CU40

//ARO130
BOOL GetFlashAR0130(UINT8 *uValue);

BOOL SetFlashAR0130(UINT8 uValue);


#pragma region See3CAM_CU130
//See3CAM_CU130
BOOL GetSceneModeCU130 (UINT8 *uSceneMode);

BOOL SetSceneModeCU130 (UINT8 uSceneMode);

BOOL GetSpecialEffectsCU130 (UINT8 *uEffectsMode);

BOOL SetSpecialEffectsCU130 (UINT8 uEffectsMode);

BOOL GetDenoiseValueCU130 (UINT8 *uDenoiseValue);

BOOL SetDenoiseValueCU130 (UINT8 uDenoiseValue);

BOOL GetExpRoiModeCU130(UINT8 *uExpRoiMode, UINT8 *uXPos, UINT8 *uYPos, UINT8 *uWindowSize);

BOOL SetExpRoiModeCU130(UINT8 uExpRoiMode, UINT8 uXPos, UINT8 uYPos, UINT8 uWindowSize);

BOOL GetBurstLengthCU130(UINT8 *uBurstLength);

BOOL SetBurstLengthCU130(UINT8 uBurstLength);

BOOL GetQFactorCU130(UINT8 *uQFactorVal);

BOOL SetQFactorCU130(UINT8 uQFactorVal);

BOOL RestoreDefaultCU130();

BOOL GetFlipModeCU130(UINT8 *uFlipMode);

BOOL SetFlipModeCU130(UINT8 uFlipMode);

BOOL GetFaceDetectionRectCU130(UINT8 *uState, UINT8 *uEmbedDataState, UINT8 *uOverlayRectState);

BOOL SetFaceDetectionRectCU130(UINT8 uState, UINT8 uEmbedDataState, UINT8 uOverlayRectState);

BOOL GetSmileDetectionStateCU130(UINT8 *uState, UINT8 *uThresholdVal, UINT8 *uEmbedDataState,UINT8 *uSmileTrigger);

BOOL SetSmileDetectionStateCU130(UINT8 uState, UINT8 uThresholdVal, UINT8 uEmbedDataState,UINT8 uSmileTrigger);

BOOL GetExposureCompensationCU130(UINT32 *iExposureComp);

BOOL SetExposureCompensationCU130(UINT32 iExposureComp);

BOOL GetFrameRateValueCU130(UINT8 *uFrameRateVal);

BOOL SetFrameRateValueCU130(UINT8 uFrameRateVal);

BOOL SetResetRegCU130(UINT16 regval);

BOOL GetResetRegCU130(UINT16 *regdata);

#pragma endregion See3CAM_CU130

#pragma region Stereo
//Stereo camera
BOOL GetExposureValueStereo(UINT32 *ExposureValue);

BOOL SetExposureValueStereo(UINT32 ExposureValue);

BOOL GetStreamModeStereo(UINT *uStreamMode);

BOOL SetStreamModeStereo(UINT uStreamMode);

BOOL GetHDRModeStereo(UINT8 *uHDRMode);

BOOL SetHDRModeStereo(UINT8 uHDRMode);

#pragma endregion Stereo

#pragma region See3CAM_130
//See3CAM_130AF
BOOL GetSceneMode130 (UINT8 *uSceneMode);

BOOL SetSceneMode130 (UINT8 uSceneMode);

BOOL GetEffectsMode130 (UINT8 *uEffectsMode);

BOOL SetEffectsMode130 (UINT8 uEffectsMode);

BOOL GetDenoiseValue130 (UINT8 *uDenoiseValue);

BOOL SetDenoiseValue130 (UINT8 uDenoiseValue);

BOOL GetAFMode130(UINT8 *uAFMode);

BOOL SetAFMode130(UINT8 uAFMode);

BOOL GetAFRoiMode130(UINT8 *uRoiMode, UINT8 *uXPos, UINT8 *uYPos, UINT8 *uWindowSize);

BOOL SetAFRoiMode130(UINT8 uRoiMode, UINT8 uXPos, UINT8 uYPos, UINT8 uWindowSize);

BOOL GetExpRoiMode130(UINT8 *uExpRoiMode, UINT8 *uXPos, UINT8 *uYPos, UINT8 *uWindowSize);

BOOL SetExpRoiMode130(UINT8 uExpRoiMode, UINT8 uXPos, UINT8 uYPos, UINT8 uWindowSize);

BOOL GetBurstLength130(UINT8 *uBurstLength);

BOOL SetBurstLength130(UINT8 uBurstLength);

BOOL GetiHDRMode130(UINT8 *uHDRMode, UINT8 *uHDRValue);

BOOL SetiHDRMode130(UINT8 uHDRMode, UINT8 uHDRValue);

BOOL GetQFactor130(UINT8 *uQFactorValue);

BOOL SetQFactor130(UINT8 uQFactorValue);

BOOL RestoreDefault130();

BOOL GetFocusRectangleMode(UINT8 *uFocusRectangleMode);

BOOL SetFocusRectangleMode(UINT8 uFocusRectangleMode);

BOOL GetFlipMode130(UINT8 *uFlipMode, UINT8 *uFlipType);

BOOL SetFlipMode130(UINT8 uFlipMode, UINT8 uFlipType);

BOOL GetStreamMode130(UINT8 *uStreamMode);

BOOL SetStreamMode130(UINT8 uStreamMode);

BOOL GetFaceDetectionRect130(UINT8 *iState, UINT8 *uEmbedDataState, UINT8 *uOverlayRectState);

BOOL SetFaceDetectionRect130(UINT8 uState, UINT8 uEmbedDataState, UINT8 uOverlayRectState);

BOOL GetSmileDetectionState130(UINT8 *uState, UINT8 *uThresholdVal, UINT8 *uEmbedDataState, UINT8 *uSmileTrigger);

BOOL SetSmileDetectionState130(UINT8 uState, UINT8 uThresholdVal, UINT8 uEmbedDataState, UINT8 uSmileTrigger);

BOOL GetExposureCompensation130(UINT32 *iExposureValue);

BOOL SetExposureCompensation130(UINT32 iExposureValue);

BOOL GetFrameRateControlVal130(UINT8 *iFrameRate);

BOOL SetFrameRateControlVal130(UINT8 uFrameRate);

#pragma endregion See3CAM_130

#pragma region See3CAM_81
//See3CAM_81 
BOOL GetFocusMode_81 (UINT8 *uFocusMode);

UINT8 SetFocusMode_81 (UINT8 uFocusMode);

BOOL GetFocusPosition_81 (UINT16 *FocusPosition);

BOOL SetFocusPosition_81 (UINT16 FocusPosition);

BOOL GetFocusStatus_81 (UINT8 *FocusStatus);

BOOL GetStrobe_81(UINT8 *Value);

BOOL SetStrobe_81(UINT8 Value);

BOOL GetSpecialEffect_81(UINT8 *uValue);

BOOL SetSpecialEffect_81(UINT8 uValue);

BOOL GetFlipMirrorStatus_81(UINT8 *uValue);

BOOL SetFlipMirrorStatus_81(UINT8 uValue);

BOOL RestoreDefault_81();

BOOL GetManualWhiteBalanceGain81(UINT8 uManualWhiteBalGain,UINT16 *WhiteBalanceGain);

BOOL SetManualWhiteBalanceGain81(UINT8 uManualWhiteBalGain,UINT16 WhiteBalanceGain);

BOOL GetFocusWindowSize81(UINT8 *uAFMode, UINT8 *uWindowSize);

BOOL SetFocusWindowSize81(UINT8 uAFMode, UINT8  uWindowSize);

BOOL GetAFMode81(UINT8 *uXPos, UINT8 *uYPos);

BOOL SetAFMode81(UINT8 uXPos, UINT8 uYPos);

#pragma endregion See3CAM_81

#pragma region See3CAM_30
//See3CAm_30
BOOL GetSceneMode30(UINT8 *uSceneMode);

BOOL SetSceneMode30(UINT8 uSceneMode);

BOOL GetEffectsMode30 (UINT8 *uEffectsMode);

BOOL SetEffectsMode30 (UINT8 uEffectsMode);

BOOL GetDenoiseValue30 (UINT8 *uDenoiseValue);

BOOL SetDenoiseValue30 (UINT8 uDenoiseValue);

BOOL GetAFMode30(UINT8 *uAFMode);

BOOL SetAFMode30(UINT8 uAFMode);

BOOL GetQFactor30(UINT8 *uQFactorValue);

BOOL SetQFactor30(UINT8 uQFactorValue);

BOOL GetAFRoiMode30(UINT8 *uRoiMode, UINT8 *uXPos, UINT8 *uYPos, UINT8 *uWindowSize);

BOOL SetAFRoiMode30(UINT8 uRoiMode, UINT8 uXPos, UINT8 uYPos, UINT8 uWindowSize);

BOOL GetExpRoiMode30(UINT8 *uExpRoiMode, UINT8 *uXPos, UINT8 *uYPos, UINT8 *uWindowSize);

BOOL SetExpRoiMode30(UINT8 uExpRoiMode, UINT8 uXPos, UINT8 uYPos, UINT8 uWindowSize);

BOOL GetBurstLength30(UINT8 *uBurstLength);

BOOL SetBurstLength30(UINT8 uBurstLength);

BOOL GetAFMarker30(UINT8 *uMarkerState);

BOOL SetAFMarker30(UINT8 uMarkerState);

BOOL GetFaceDetectionRect30(UINT8 *uState, UINT8 *uEmbedDataState, UINT8 *uOverlayRectState);

BOOL SetFaceDetectionRect30(UINT8 uState, UINT8 uEmbedDataState, UINT8 uOverlayRectState);

BOOL GetSmileDetectionState30(UINT8 *uState, UINT8 *uThresholdVal, UINT8 *uEmbedDataState, UINT8 *uSmileTrigger);

BOOL SetSmileDetectionState30(UINT8 uState, UINT8 uThresholdVal, UINT8 uEmbedDataState, UINT8 uSmileTrigger);

BOOL GetExposureCompensation30(UINT32 *uExposureValue);

BOOL SetExposureCompensation30(UINT32 uExposureValue);

BOOL GetFrameRateControlVal30(UINT8 *uFrameRate);

BOOL SetFrameRateControlVal30(UINT8 uFrameRate);

BOOL RestoreDefault30();

BOOL SetOrientationMode30(UINT8 uFlipMode);

BOOL GetOrientationMode30(UINT8 *uFlipMode);

BOOL GetFlash30(UINT8 *uFlashValue);

BOOL SetFlash30(UINT8 uFlashValue);

#pragma endregion See3CAM_30

#pragma region See3CAM_CU135
//See3CAM_CU135
BOOL GetSceneModeCU135(UINT8 *uSceneMode);

BOOL SetSceneModeCU135(UINT8 uSceneMode);

BOOL GetSpecialEffectsCU135(UINT8 *uEffects);

BOOL SetSpecialEffectsCU135(UINT8 uEffects);

BOOL GetDenoiseValueCU135(UINT8 *uDenoiseValue);

BOOL SetDenoiseValueCU135(UINT8 uDenoiseValue);

BOOL GetExpRoiModeCU135(UINT8 *uExpRoiMode, UINT8 *uXPos, UINT8 *uYPos, UINT8 *uWindowSize);

BOOL SetExpRoiModeCU135(UINT8 uExpRoiMode, UINT8 uXPos, UINT8 uYPos, UINT8 uWindowSize);

BOOL GetBurstLengthCU135(UINT8 *uBurstLength);

BOOL SetBurstLengthCU135(UINT8 uBurstLength);

BOOL GetHDRModeCU135(UINT8 *uHDRMode, UINT8 *uHDRValue);

BOOL SetHDRModeCU135(UINT8 uHDRMode, UINT8 uHDRValue);

BOOL GetQFactorCU135(UINT8 *uQFactorVal);

BOOL SetQFactorCU135(UINT8 uQFactorVal);

BOOL GetStreamModeCU135(UINT8 *uStreamMode);

BOOL SetStreamModeCU135(UINT8 uStreamMode);

BOOL RestoreDefaultCU135();

BOOL GetFlipModeCU135(UINT8 *uFlipMode);

BOOL SetFlipModeCU135(UINT8 uFlipMode);

BOOL GetFaceDetectionRectCU135(UINT8 *uState, UINT8 *uStatusStructState, UINT8 *uOverlayRectState);

BOOL SetFaceDetectionRectCU135(UINT8 uState, UINT8 uStatusStructState, UINT8 uOverlayRectState);

BOOL GetSmileDetectionStateCU135(UINT8 *uState, UINT8 *uThresholdVal, UINT8 *uEmbedDataState,UINT8 *uSmileTriggerState);

BOOL SetSmileDetectionStateCU135(UINT8 iState, UINT8 iThresholdVal, UINT8 iEmbedDataState,UINT8 uSmileTriggerState);

BOOL GetExposureCompensationCU135(UINT32 *iExposureComp);

BOOL SetExposureCompensationCU135(UINT32 iExposureComp);

BOOL GetFrameRateValueCU135(UINT8 *uFrameRateVal);

BOOL SetFrameRateValueCU135(UINT8 uFrameRateVal);

#pragma endregion See3CAM_CU135

#pragma region See3CAM_CU20
//See3CAM_CU20
BOOL GetSensorModeCU20(UINT8 *uSensorMode);

BOOL SetSensorModeCU20(UINT8 uSensorMode);

BOOL GetCAMModeCU20(UINT8 *uCAMMode);

BOOL SetCAMModeCU20(UINT8 uCAMMode);

BOOL GetSpecialEffectsCU20(UINT8 *ueffects);

BOOL SetSpecialEffectsCU20(UINT8 ueffects);

BOOL GetExpRoiModeCU20(UINT8 *uRoiMode, UINT8 *uXPos, UINT8 *uYPos, UINT8 *uWindowSize);

BOOL SetExpRoiModeCU20(UINT8 uRoiMode, UINT8 uXPos, UINT8 uYPos, UINT8 uWindowSize);

BOOL GetFlipModeCU20(UINT8 *uFlipMode);

BOOL SetFlipModeCU20(UINT8 uFlipMode);

BOOL GetStrobeCU20(UINT8 *uStrobeValue);

BOOL SetStrobeCU20(UINT8 uStrobeValue);

BOOL GetColorKillValueCU20(UINT8 *uColorkillVal);

BOOL SetColorKillValueCU20(UINT8 uColorkillVal);

BOOL RestoreDefaultCU20();

BOOL GetBurstLengthCU20(UINT8 *uBurstLength);

BOOL SetBurstLengthCU20(UINT8 uBurstLength);

BOOL GetAntiFlickerModeCU20(UINT8 *uflicker);

BOOL SetAntiFlickerModeCU20(UINT8 uflucker);

BOOL GetDenoiseValueCU20(UINT8 *uDenoiseValue);

BOOL SetDenoiseValueCU20(UINT8 uDenoiseValue);

BOOL GetLSCModeCU20(UINT8 *uLSCMode);

BOOL SetLSCModeCU20(UINT8 uLSCMode);

BOOL GetExposureCompensationCU20(UINT8 *uExposureComp);

BOOL SetExposureCompensationCU20(UINT8 uExposureComp);

#pragma endregion See3CAM_CU20