import threading
import sys
import cv2
import numpy as np
from conversion import Conversion
from capture import Capture
from time import sleep

class Display:
    '''
    This Class provides method for displaying the frame from the device.
    '''
    stop_thread = False
    kill_thread = False
    display_thread = threading.Thread()
    SCREEN_WIDTH = 1920
    SCREEN_HEIGHT = 1280
    frame_width = 0
    frame_height = 0
    frame_format = ''
    
    @staticmethod
    def start_display(cap, current_format, device_name):
        '''
        Method Name: start_display
        Description: This method starts the display thread.
        :param cap: object of VideoCapture class in cv2 module
        :param current_format: The current output image format of the device.
        :param device_name: name of the device
        '''
        Display.stop_thread = False
        Display.kill_thread = False

        Conversion.init_conversion(current_format, device_name)
        Display.resize_frame(current_format)
        if not Display.display_thread.is_alive():
            Display.display_thread = threading.Thread(target=Display.display_frame, args=(cap,), name = "Display-Thread", daemon=False)
            Display.display_thread.start()

    @classmethod
    def display_frame(cls, cap):
        '''
        Method Name: display_frame
        Description: This method is a seperate thread which reads the frame from the device and displays it.
        :param cap: object of VideoCapture class in cv2 module
        '''
        while True:
            while not Display.stop_thread:
                ret, frame = cap.read()
                if not (np.sum(frame) == None):
                    if Capture.capture_flag:
                        Stillframe = frame
                        Capture.capture_image(Stillframe, cls.frame_format, cap)
                    if cls.frame_format == Conversion.V4L2_PIX_FMT_Y12 or cls.frame_format == Conversion.V4L2_PIX_FMT_Y16 or cls.frame_format == Conversion.V4L2_PIX_FMT_Y10:
                        if Conversion.y16CameraFlag == Conversion.SEE3CAM_CU40:
                            new_frame, IR_frame = Conversion.convert_frame(frame, cls.frame_format)
                            cv2.namedWindow('Frame', cv2.WINDOW_NORMAL)
                            cv2.resizeWindow('Frame', cls.frame_width, cls.frame_height)
                            cv2.imshow('Frame', IR_frame)

                        elif Conversion.y16CameraFlag == Conversion.SEE3CAM_CU83:
                            rows,cols = frame.shape
                            if (rows == 2160 and cols == 4440) or (rows == 1080 and cols == 3120):
                                RGBframe, IRframe = Conversion.SeparatingRGBIRBuffers(frame, cls.frame_format)
                                if not (np.sum(RGBframe) == None):
                                    new_frame = cv2.cvtColor(RGBframe, cv2.COLOR_YUV2BGR_UYVY)
                                    cv2.namedWindow('RGB Frame', cv2.WINDOW_NORMAL)
                                    if rows == 2160:  # Adjust window size for 4440 x 2160 resolution
                                        cv2.resizeWindow('RGB Frame', 3840, 2160)
                                    elif rows == 1080:  # Adjust window size for 3120 x 1080 resolution
                                        cv2.resizeWindow('RGB Frame', 1920, 1080)
                                    cv2.imshow('RGB Frame', new_frame)

                                if not (np.sum(IRframe) == None):
                                    cv2.namedWindow('IR Frame', cv2.WINDOW_NORMAL)
                                    cv2.resizeWindow('IR Frame', 1920, 1080)
                                    cv2.imshow('IR Frame', IRframe)

                            elif rows == 1350 and cols == 3840:

                                IRframe = Conversion.ConvertRaw10toRaw8(frame,cls.frame_format)
                                cv2.namedWindow('Frame', cv2.WINDOW_NORMAL)
                                cv2.resizeWindow('Frame', 3840, 2160)
                                cv2.imshow('Frame', IRframe)
                            
                            elif rows == 675 and cols == 1920:

                                IRframe = Conversion.ConvertRaw10toRaw8(frame,cls.frame_format)
                                cv2.namedWindow('Frame', cv2.WINDOW_NORMAL)
                                cv2.resizeWindow('Frame', 1920, 1080)
                                cv2.imshow('Frame', IRframe)
                        else:
                            new_frame = Conversion.convert_frame(frame, cls.frame_format)
                            cv2.namedWindow('Frame', cv2.WINDOW_NORMAL)
                            cv2.resizeWindow('Frame', cls.frame_width, cls.frame_height)
                            cv2.imshow('Frame', new_frame)
                    elif cls.frame_format == "UYVY" or cls.frame_format == "YUY2":

                        if Conversion.y16CameraFlag == Conversion.SEE3CAM_CU83:
                            if not (np.sum(frame) == None):
                                new_frame = Conversion.convert_frame(frame, cls.frame_format)
                                cv2.namedWindow('RGB Frame', cv2.WINDOW_NORMAL)
                                cv2.resizeWindow('RGB Frame', cls.frame_width, cls.frame_height)       
                                cv2.imshow('RGB Frame', new_frame)

                        elif Conversion.IRRGBCameraFlag27CUG == Conversion.SEE3CAM_27CUG:
                            RGB_frame = Conversion.Check_RGB_Frame(frame)
                            if not (np.sum(RGB_frame) == None):
                                new_frame = Conversion.convert_frame(RGB_frame, cls.frame_format)
                                cv2.namedWindow('RGB Frame', cv2.WINDOW_NORMAL)
                                cv2.resizeWindow('RGB Frame', cls.frame_width, cls.frame_height)       
                                cv2.imshow('RGB Frame', new_frame)
                                continue
                            if not (np.sum(frame) == None):                                  
                                IRFrame = Conversion.convert_frame(frame, cls.frame_format)
                                cv2.namedWindow('Frame', cv2.WINDOW_NORMAL)
                                cv2.resizeWindow('Frame', cls.frame_width, cls.frame_height)          
                                cv2.imshow('Frame', IRFrame)
                            else:
                                new_frame = Conversion.convert_frame(frame, cls.frame_format)
                                cv2.namedWindow('Frame', cv2.WINDOW_NORMAL)
                                cv2.resizeWindow('Frame', cls.frame_width, cls.frame_height)
                                cv2.imshow('Frame', new_frame)
                        else:
                            new_frame = Conversion.convert_frame(frame, cls.frame_format)
                            cv2.namedWindow('Frame', cv2.WINDOW_NORMAL)
                            cv2.resizeWindow('Frame', cls.frame_width, cls.frame_height)
                            cv2.imshow('Frame', new_frame)
                    else:
                        new_frame = frame
                        cv2.namedWindow('Frame', cv2.WINDOW_NORMAL)
                        cv2.resizeWindow('Frame', cls.frame_width, cls.frame_height)
                        cv2.imshow('Frame', new_frame)
                            
                    cv2.waitKey(1)
                else:
                    print("Null Frame Received...")
                        
            if Display.kill_thread:
                cv2.destroyAllWindows()
                break
            if sys.platform == "linux":
                break
            if sys.platform == "win32":
                break

    @staticmethod
    def stop_display():
        '''
        Method Name: stop_display
        Description: This method stops the display thread and closes all windows.
        '''

        if not Display.stop_thread:
            if Display.display_thread.is_alive():
                Display.stop_thread = True
                sleep(0.3)
                if sys.platform == "linux":
                    Display.display_thread.join()
                elif sys.platform == "win32":
                    Display.display_thread.join()


        if sys.platform == "linux":
            cv2.destroyAllWindows()
        if sys.platform == "win32":
            cv2.destroyAllWindows()


    @staticmethod
    def resume_display():
        '''
        Method Name: resume_display
        Description: This method resume the display thread and closes all windows.
        '''
        if Display.stop_thread:              
                Display.stop_thread = False

    @classmethod
    def resize_frame(cls, current_format):
        '''
        Method Name: resize_frame
        Description: this method resizes the frame displayed according to the screen resolution
        :param current_format: The current output image format of the device.
        '''
        format_type, width, height, fps = current_format
        cls.frame_format = format_type
        cls.frame_width = width
        cls.frame_height = height
        if width > cls.SCREEN_WIDTH - 100:
            cls.frame_width = cls.SCREEN_WIDTH - 100
            cls.frame_height = int((cls.frame_width * height) / width)
        if cls.frame_height > cls.SCREEN_HEIGHT - 100:
            cls.frame_height = cls.SCREEN_HEIGHT - 100
            cls.frame_width = int((cls.frame_height * width) / height)

    @staticmethod
    def Kill_Display_thread():
        '''
        Method Name: Kill_Display_thread
        Description: This method Kills the display thread and closes all windows.
        '''
        if Display.display_thread.is_alive():
            Display.kill_thread = True
