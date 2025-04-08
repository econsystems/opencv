import cv2
import numpy as np
import os
from datetime import datetime

import sys

class Conversion:
    '''
    This class provides methods to convert frames from device output format to rgb for rendering and saving images.
    '''
    # Flag values to denote type of y16 camera.
    OTHER_Y16CAMERAS = 0
    SEE3CAM_20CUG = 1
    SEE3CAM_CU40 = 2
    SEE3CAM_27CUG = 3
    SEE3CAM_CU83 = 4

    V4L2_PIX_FMT_Y16 = "Y16 "
    V4L2_PIX_FMT_Y12 = "Y12 "
    V4L2_PIX_FMT_Y8  = "Y8  "
    V4L2_PIX_FMT_Y10 = "Y10 "
    format_type = 0

    y16CameraFlag = -1  # flag which denotes type of y16 camera.
    y8_frame = None
    IRRGBCameraFlag27CUG = "UYVY"
    UYVYCameraFlagCU83 = "UYVY"

    @classmethod
    def init_conversion(cls, current_format, device_name):
        '''
        Method Name: init_conversion
        Description: This method enables y16CameraFlag based on the name of the camera, since for each camera, the
                    conversion method is different.
        :param current_format: current output format of the device
        :type current_format: str
        :param device_name: Name of the selected device
        :type device_name:  str
        '''
        cls.format_type, width, height, fps = current_format
        if cls.format_type == cls.V4L2_PIX_FMT_Y16:
            if device_name.find("See3CAM_20CUG") > -1  or device_name.find("See3CAM_CU135MH") > -1 or device_name.find("See3CAM_CU135M_H03R1") > -1:
                cls.y16CameraFlag = cls.SEE3CAM_20CUG
            elif device_name.find("See3CAM_CU40") > -1:
                cls.y16CameraFlag = cls.SEE3CAM_CU40
            elif device_name.find("See3CAM_CU83") > -1:
                cls.y16CameraFlag = cls.SEE3CAM_CU83
            elif device_name.find("See3CAM_CU83_H03R1") > -1:
                cls.y16CameraFlag = cls.SEE3CAM_CU83
            else:
                cls.y16CameraFlag = cls.OTHER_Y16CAMERAS

        elif cls.format_type == cls.IRRGBCameraFlag27CUG:
            if device_name.find("See3CAM_27CUG") > -1:
                cls.IRRGBCameraFlag27CUG = cls.SEE3CAM_27CUG
            if device_name.find("See3CAM_CU83") > -1:
                cls.y16CameraFlag = cls.SEE3CAM_CU83

        elif cls.format_type == cls.V4L2_PIX_FMT_Y8:
            if device_name.find("See3CAM_CU83") > -1:
                cls.y16CameraFlag = cls.SEE3CAM_CU83
                
        cls.y8_frame = np.zeros(shape=(height, width), dtype=np.uint8)

    @classmethod
    def convert_frame(cls, frame, frame_format):
        '''
        Method Name: convert_frame
        Description: This method calls the conversion function based on the frame foramt
        :param frame: frame which needs to be converted
        :type frame: Mat
        :param frame_format: The format of the frame
        :type frame_format: str
        :return: the converted frame
        :rtype: Mat
        '''

        if cls.format_type == "UYVY":
            return cv2.cvtColor(frame, cv2.COLOR_YUV2BGR_UYVY)
        if cls.format_type == "YUY2":
            return cv2.cvtColor(frame, cv2.COLOR_YUV2BGR_YUY2)
        if cls.format_type == "Y8  ":
            return cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        convert_func = {
            "Y12 ": cls.convert_y12_to_y8,
            "Y16 ": cls.convert_y16_to_rgb,
            "Y10 ": cls.convert_y16_to_rgb,
        }

        func = convert_func.get(frame_format, lambda: "Invalid Selection")
        return func(frame)

    @classmethod
    def convert_y12_to_y8(cls, frame):
        '''
        Method Name: convert_y12_to_y8
        Description: This method converts the y12 frame to y8 frame
        :param frame: frame which needs to be converted
        :type frame: Mat
        :return: the converted frame
        :rtype: Mat
        '''

        raw_bytes = frame.tobytes()#converting two dimensional mat data to byte array
        row = frame.shape[0]
        column = frame.shape[1]
        filtered_bytes = np.frombuffer(raw_bytes, dtype=np.uint8)
        filtered_bytes = np.reshape(filtered_bytes, (-1, 3))
        filtered_bytes = np.delete(filtered_bytes,2,1)
        filtered_bytes= np.reshape(filtered_bytes,-1)
        m=0
        for i in range(0, row):
            cls.y8_frame[i,]=filtered_bytes[m:m+column]
            m+=column
        return cls.y8_frame  # Converting back to two dimensional Mat

    @classmethod
    def convert_y16_to_rgb(cls, frame):
        '''
        Method Name: convert_y16_to_rgb
        Description: This method converts y16 rgb or y8 for rendering and saving image.
        :param frame: frame which needs to be converted
        :type frame: Mat
        :return: the converted frame
        :rtype: Mat
        '''
        if cls.y16CameraFlag == cls.SEE3CAM_20CUG:
            return cv2.convertScaleAbs(frame, 0.2490234375)
        elif cls.y16CameraFlag == cls.SEE3CAM_CU40:
            return cls.convert_RGIR_to_RGGB(frame)
        elif cls.y16CameraFlag == cls.OTHER_Y16CAMERAS:
            return cv2.convertScaleAbs(frame, 0.06226)
        return cv2.convertScaleAbs(frame, 0.2490234375)
    @staticmethod
    def convert_y12_for_still(frame):
        '''
        Method Name: convert_y12_for_still
        Description: This method converts the y12 frame to y16 with padding in order to save as raw image.
        :param frame: frame which needs to be converted
        :type frame: Mat
        :return: the converted frame
        :rtype: bytearray
        '''

        raw_bytes = frame.tobytes()
        row, column = frame.shape
        y12_still_buffer = np.zeros(row * column * 2, dtype=np.uint8)
        m = 0
        stride = column * 2
        for j in range(0, row):
            for i in range(0, stride, 4):
                y12_still_buffer[(stride * j) + i + 1] = ((0XF0 & raw_bytes[m]) >> 4)
                bPixel1 = (raw_bytes[m] & 0X0F)
                bPixel2 = (raw_bytes[m + 2] & 0X0F)
                bPixel1 = (bPixel1 << 4)
                y12_still_buffer[(stride * j) + i] = bPixel1 + bPixel2
                y12_still_buffer[(stride * j) + i + 3] = ((0XF0 & raw_bytes[m + 1]) >> 4)
                bPixel1 = (raw_bytes[m + 1] & 0X0F)
                bPixel2 = (raw_bytes[m + 2] & 0XF0)
                bPixel1 = (bPixel1 << 4)
                bPixel2 = (bPixel2 >> 4)
                y12_still_buffer[(stride * j) + i + 2] = bPixel1 + bPixel2
                m += 3
        return y12_still_buffer

    @staticmethod
    def convert_RGIR_to_RGGB(frame):
        '''
        Method Name: convert_RGIR_to_RGGB
        Description: This method converts RGIR to RGGB frame using nearby neighbour interpolation method
                     and seperates the IR frame
        :param frame: frame which needs to be converted
        :type frame: Mat
        :return: the converted frame
        :rtype: rgb frame and IR frame
        '''

        row, column = frame.shape
        bayer_RGIR = cv2.convertScaleAbs(frame, 0.249023)
        bayer_RGGB = bayer_RGIR.clone()

        ir_frame = np.zeros(int((column * row) / 4), np.uint8).reshape(int(row / 2), int(column / 2))

        for i in range(0, row, 2):
            for j in range(0, column, 2):
                bayer_RGGB[i + 1, j] = bayer_RGIR[i, j + 1]
                ir_frame[int(i / 2), int(j / 2)] = bayer_RGIR[i + 1, j]

        rgb_frame = cv2.cvtColor(bayer_RGGB, cv2.COLOR_BayerRG2BGR)
        return rgb_frame, ir_frame
    

    @staticmethod
    def Check_RGB_Frame(frame):
        raw_bytes = frame.tobytes()
        if(raw_bytes[7] == 0):
            return frame
    
    @staticmethod
    def SeparatingRGBIRBuffers(frame , frame_format):
        RGBIRBuff = np.frombuffer(frame.tobytes(), dtype=np.uint8)
        rows , cols = frame.shape
        if (rows == 2160 and cols == 4440):
            size = rows * cols * 2

            Buffcnt = 0
            RGBBufSize = 0
            IRBufSize = 0

            RGBframe = np.zeros((2160*3840*2), dtype=np.uint8)
            IRframe = np.zeros((1080*1920), dtype=np.uint8)
            IRBuffLength = size - RGBframe.size
            IRBuff = np.zeros(IRBuffLength, dtype=np.uint8)
            while size > 0:
                lsb = RGBIRBuff[Buffcnt] & 0x03
                if lsb == 0:   
                    RGBframe[RGBBufSize:RGBBufSize +7679] = RGBIRBuff[Buffcnt:Buffcnt+7679]
                    Buffcnt += 7680
                    RGBBufSize += 7680
                    size -= 7680
                elif lsb == 0x03:
                    IRBuff[IRBufSize:(IRBufSize+2399)] = RGBIRBuff[Buffcnt:(Buffcnt+2399)]
                    IRBufSize += 2400
                    Buffcnt += 2400
                    size -= 2400
                else:
                    return None , None
            
            IRframe[0:IRframe.size] = IRBuff[np.mod(np.arange(1, IRBuffLength + 1), 5) != 0]
            IRframe = IRframe.reshape(1080,1920)
            RGBframe = RGBframe.reshape(2160, 3840, 2)
            return RGBframe , IRframe

        size = rows * cols * 2
        
        Buffcnt = 0
        RGBBufSize = 0
        IRBufSize = 0

        RGBframe = np.zeros((1080*1920*2), dtype=np.uint8)
        IRframe = np.zeros((1080*1920), dtype=np.uint8)
        IRBuffLength = size - RGBframe.size
        IRBuff = np.zeros(IRBuffLength, dtype=np.uint8)
        while size > 0:
            lsb = RGBIRBuff[Buffcnt] & 0x03
            if lsb == 0:  
                RGBframe[RGBBufSize:RGBBufSize +3839] = RGBIRBuff[Buffcnt:Buffcnt+3839]
                Buffcnt += 3840
                RGBBufSize += 3840
                size -= 3840
            elif lsb == 0x03:
                IRBuff[IRBufSize:(IRBufSize+2399)] = RGBIRBuff[Buffcnt:(Buffcnt+2399)]
                IRBufSize += 2400
                Buffcnt += 2400
                size -= 2400
            else:
                return None , None
            
        IRframe[0:IRframe.size] = IRBuff[np.mod(np.arange(1, IRBuffLength + 1), 5) != 0]
        IRframe = IRframe.reshape(1080, 1920)
        RGBframe = RGBframe.reshape(1080, 1920, 2)
        return RGBframe , IRframe
  
    @staticmethod
    def ConvertRaw10toRaw8(frame ,frame_format):

        '''
        Method Name: ConvertRaw10toRaw8
        Description: This method converts RAW10 to RAW8 frames using nearby neighbour interpolation method
                     and seperates the IR frame
        :param frame: frame which needs to be converted
        :type frame: Mat
        :return: the converted frame
        :rtype:  IR frame
        '''

        IRBuff = np.frombuffer(frame.tobytes(), dtype=np.uint8)
        Buffcnt = 0
        bufsize_IR = 0
        rows , cols = frame.shape
        size = rows * cols * 2
        if rows == 1350 and cols == 3840:           
            IRframe = np.zeros((2160, 3840), dtype=np.uint8)
        else:
            IRframe = np.zeros((1080, 1920), dtype=np.uint8)
        while size > 0:
            IRframe.flat[bufsize_IR:bufsize_IR +4] = IRBuff[Buffcnt:Buffcnt+4]
            bufsize_IR += 4
            Buffcnt += 5
            size -= 5
        return IRframe
    
