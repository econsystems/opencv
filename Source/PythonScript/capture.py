import threading
import os
import cv2
import numpy as np
import display
from conversion import Conversion
from datetime import datetime
from time import sleep

class Capture:
    '''
    This class provides methods to save image in .jpg and .raw format as required.
    '''
    capture_flag = False
    convert_to_RAW_Selected = False
    convert_to_RGB_Selected = False
    capture_thread = threading.Thread()
    StillCapturingImage = False
    RGBFrameCaptured = False
    IRFrameCaptured = False
    RGBIRFrameCaptured = False


    @staticmethod
    def capture_image(frame, frame_format, cap):
        '''
        Method Name: capture_image
        Description: This method create the thread to save the frame.
        :param frame: The frame which needs to be saved
        :param frame_format: The format of the frame, for ex: YUYV, MJPG, etc
        :return: True.
        '''
        if not (np.sum(frame) == None):
            if Conversion.IRRGBCameraFlag27CUG != Conversion.SEE3CAM_27CUG:                  
                Capture.capture_flag = False
            # elif Conversion.y16CameraFlag != Conversion.SEE3CAM_CU83:
            #     Capture.capture_flag = False
            else:
                if Conversion.IRRGBCameraFlag27CUG == Conversion.SEE3CAM_27CUG:

                    raw_bytes = frame.tobytes()
                    if Capture.RGBFrameCaptured == False and raw_bytes[7] == 0x00:
                        print("inside rgbframe")
                        Capture.RGBFrameCaptured = True 
                    elif Capture.IRFrameCaptured == False and raw_bytes[7] == 0x01:
                        print("inside irframe")
                        Capture.IRFrameCaptured = True  
                    elif Capture.IRFrameCaptured == True and Capture.RGBFrameCaptured == True:
                        print("inside both")
                        print("\n Image Saved")
                        Capture.capture_flag = False
                        Capture.RGBFrameCaptured = False
                        Capture.IRFrameCaptured = False
                        Capture.convert_to_RAW_Selected = False
                        Capture.StillCapturingImage = False
                        
                        return True                  
                    else:
                        return True

            if Capture.convert_to_RAW_Selected:
                # print("\n Raw Frame")
                if not (frame_format == "UYVY") | (frame_format == "YUY2") | (frame_format == "Y8  ") | (frame_format == "Y16 "):
                    display.Display:stop_display()
                    cap.set(cv2.CAP_PROP_CONVERT_RGB, 1)
                    display.Display:resume_display()

            if not Capture.capture_thread.is_alive():
                Capture.capture_thread = threading.Thread(target=Capture.convert_image, args=(frame, frame_format,),
                                                            daemon=True)
                Capture.capture_thread.start()
            #Capture.convert_image(frame, frame_format)
            return True
            

    @staticmethod
    def convert_image(frame, frame_format):
        '''
        Method Name: convert_image
        Description: This method is Callback method, converts frame to required format to be saved as jpg or raw file.
        :param frame: The frame which needs to be converted
        :param frame_format: The format of the frame. for ex: YUYV, MJPG, etc
        '''
        # print(Capture.convert_to_RAW_Selected)
        if frame_format == "Y8  ":
           if Conversion.y16CameraFlag == Conversion.SEE3CAM_CU83:
               if Capture.convert_to_RAW_Selected:     
                    Capture.save_image(frame, frame_format, frame.shape[0], frame.shape[1], image_format='.raw')
                    Capture.convert_to_RAW_Selected = False
               else:
                   Capture.save_image(frame, frame_format, frame.shape[0], frame.shape[1], image_format='.jpg')
           else:
               image = frame.data
               Capture.save_image(image, frame_format, frame.shape[0], frame.shape[1], image_format='.raw')
               
        elif Capture.convert_to_RAW_Selected:

            if frame_format == Conversion.V4L2_PIX_FMT_Y16 and Conversion.y16CameraFlag == Conversion.SEE3CAM_CU83:
                        rows,cols = frame.shape
                        if rows == 2160 and cols == 4440:
                            Capture.RGBIRFrameCaptured = True 
                            rgbframe , irframe = Conversion.SeparatingRGBIRBuffers(frame,frame_format)
                            new_frame = cv2.cvtColor(rgbframe, cv2.COLOR_YUV2BGR_UYVY)           
                            Capture.save_image(new_frame, frame_format, new_frame.shape[0], new_frame.shape[1], image_format='.raw')
                            Capture.save_image(irframe, frame_format, irframe.shape[0], irframe.shape[1], image_format='.raw')
                            print("\n Image Saved")
                            Capture.StillCapturingImage = False
                            Capture.convert_to_RAW_Selected = False
                            Capture.RGBIRFrameCaptured = False
                        else:
                            IRframe = Conversion.ConvertRaw10toRaw8(frame,frame_format)
                            Capture.save_image(IRframe, frame_format, IRframe.shape[0], IRframe.shape[1], image_format='.raw')
                            Capture.convert_to_RAW_Selected = False
            else:
                        image = frame
                        Capture.save_image(image, frame_format, frame.shape[0], frame.shape[1], image_format='.raw')
                        if Conversion.y16CameraFlag == Conversion.SEE3CAM_CU83:
                            Capture.convert_to_RAW_Selected = False
            
        elif frame_format == Conversion.V4L2_PIX_FMT_Y16:
            if Conversion.y16CameraFlag == Conversion.SEE3CAM_CU40:
                IR_image, image = Conversion.convert_y16_to_rgb(frame)
                Capture.save_image(image, frame_format, frame.shape[0], frame.shape[1], image_format='.jpg')
                Capture.save_image(IR_image, frame_format, frame.shape[0], frame.shape[1], image_format='.jpg', ir="IR")

            elif Conversion.y16CameraFlag == Conversion.SEE3CAM_CU83:
                rows,cols = frame.shape
                if rows == 2160 and cols == 4440:
                        Capture.RGBIRFrameCaptured = True 
                        rgbframe , irframe = Conversion.SeparatingRGBIRBuffers(frame,frame_format)
                        new_frame = cv2.cvtColor(rgbframe, cv2.COLOR_YUV2BGR_UYVY)           
                        Capture.save_image(new_frame, frame_format, new_frame.shape[0], new_frame.shape[1], image_format='.jpg')
                        Capture.save_image(irframe, frame_format, irframe.shape[0], irframe.shape[1], image_format='.jpg')
                        print("\n Image Saved")                        
                        Capture.StillCapturingImage = False
                        Capture.RGBIRFrameCaptured = False
                else:
                    IRframe = Conversion.ConvertRaw10toRaw8(frame,frame_format)
                    Capture.save_image(IRframe, frame_format, IRframe.shape[0], IRframe.shape[1], image_format='.jpg')
            else:
                image = Conversion.convert_y16_to_rgb(frame)
                Capture.save_image(image, frame_format, frame.shape[0], frame.shape[1], image_format='.jpg')

        elif frame_format == Conversion.V4L2_PIX_FMT_Y12:
            image = Conversion.convert_y12_for_still(frame)
            Capture.save_image(image, frame_format, frame.shape[0], frame.shape[1], image_format='.raw')
        else:
            if frame_format == "UYVY":
                image = cv2.cvtColor(frame, cv2.COLOR_YUV2BGR_UYVY, 3)
            elif frame_format == "YUY2":
                image = cv2.cvtColor(frame, cv2.COLOR_YUV2BGR_YUY2, 3)
            else:
                image = frame
            Capture.save_image(image, frame_format, frame.shape[0], frame.shape[1], image_format='.jpg')

    @staticmethod
    def save_image(image, frame_format, width, height, image_format, ir=''):
        '''
        Method Name: save_image
        Description: This method saves the image in the current working directory.
        :param image: The converted image which needs to be saved.
        :param frame_format: The format of the frame. for ex: YUYV, MJPG, etc
        :param width: width of the image
        :param height: height of the image
        :param image_format: format in which image is saved. for ex: .raw, .jpg
        :param ir: if ir image is saved, "IR" is added to file name.
        :return: True - when image is saved successfully.
                or False - when image is not saved.
        '''
        time = datetime.now().strftime("%d%m%Y_%H%M%S%f")[:-3]
        if frame_format == Conversion.V4L2_PIX_FMT_Y12 or frame_format == Conversion.V4L2_PIX_FMT_Y16:
            frame_format = frame_format[:-1]  # slicing the string to remove space("Y16 ") in y12 and y16.

        file_name = f"{os.getcwd()}/OpenCVCam_{ir}image_{frame_format}_{height}x{width}_{time}{image_format}"

        if image_format == '.raw':  # since raw file write is not supported by imwrite, we are using file write
            try:
                fp = open(file_name, 'wb+')
                fp.write(image)
                fp.close()
            except IOError:
                print("File operation error.Image is not saved!")
                return False
        else:
            cv2.imwrite(file_name, image)

        if Conversion.IRRGBCameraFlag27CUG != Conversion.SEE3CAM_27CUG and Capture.RGBIRFrameCaptured == False:
            Capture.StillCapturingImage = False                    
            print("\n Image Saved")

          
        return True
