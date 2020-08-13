import threading
import os
import cv2
import display
from conversion import Conversion
from datetime import datetime


class Capture:
    '''
    This class provides methods to save image in .jpg and .raw format as required.
    '''

    capture_flag = False
    convert_RGB_Selected = False
    capture_thread = threading.Thread()

    @staticmethod
    def capture_image(frame, frame_format, cap):
        '''
        Method Name: capture_image
        Description: This method create the thread to save the frame.
        :param frame: The frame which needs to be saved
        :param frame_format: The format of the frame, for ex: YUYV, MJPG, etc
        :return: True.
        '''

        Capture.capture_flag = False
        if Capture.convert_RGB_Selected:
            display.Display:stop_display()
            cap.set(cv2.CAP_PROP_CONVERT_RGB, True)
            display.Display:resume_display()
            
        if not Capture.capture_thread.is_alive():
            Capture.capture_thread = threading.Thread(target=Capture.convert_image, args=(frame, frame_format,),
                                                      daemon=True)
            Capture.capture_thread.start()
        return True

    @staticmethod
    def convert_image(frame, frame_format):
        '''
        Method Name: convert_image
        Description: This method is Callback method, converts frame to required format to be saved as jpg or raw file.
        :param frame: The frame which needs to be converted
        :param frame_format: The format of the frame. for ex: YUYV, MJPG, etc
        '''
        if Capture.convert_RGB_Selected:
            Capture.convert_RGB_Selected = False
            image = frame
            Capture.save_image(image, frame_format, frame.shape[0], frame.shape[1], image_format='.raw')

        elif frame_format == Conversion.V4L2_PIX_FMT_Y16:
            if Conversion.y16CameraFlag == Conversion.SEE3CAM_CU40:
                IR_image, image = Conversion.convert_y16_to_rgb(frame)
                Capture.save_image(image, frame_format, frame.shape[0], frame.shape[1], image_format='.jpg')
                Capture.save_image(IR_image, frame_format, frame.shape[0], frame.shape[1], image_format='.jpg', ir="IR")
            else:
                image = Conversion.convert_y16_to_rgb(frame)
                Capture.save_image(image, frame_format, frame.shape[0], frame.shape[1], image_format='.jpg')

        elif frame_format == Conversion.V4L2_PIX_FMT_Y12:
            image = Conversion.convert_y12_for_still(frame)
            Capture.save_image(image, frame_format, frame.shape[0], frame.shape[1], image_format='.raw')
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

        time = datetime.now().strftime("%d_%m_%Y_%H_%M_%S")
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
        return True
