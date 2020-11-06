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
                        ret, Stillframe = cap.read()
                        if not (np.sum(Stillframe) == None):
                            Capture.capture_image(Stillframe, cls.frame_format, cap)
                            continue

                    if cls.frame_format == Conversion.V4L2_PIX_FMT_Y12 or cls.frame_format == Conversion.V4L2_PIX_FMT_Y16:
                        if Conversion.y16CameraFlag == Conversion.SEE3CAM_CU40:
                            new_frame, IR_frame = Conversion.convert_frame(frame, cls.frame_format)
                            cv2.namedWindow('IR Frame', cv2.WINDOW_NORMAL)
                            cv2.resizeWindow('IR Frame', cls.frame_width, cls.frame_height)
                            cv2.imshow('IR Frame', IR_frame)
                        else:
                            new_frame = Conversion.convert_frame(frame, cls.frame_format)
                    elif cls.frame_format == "UYVY" or cls.frame_format == "YUY2":
                        new_frame = Conversion.convert_frame(frame, cls.frame_format)
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
        if sys.platform == "linux":
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
