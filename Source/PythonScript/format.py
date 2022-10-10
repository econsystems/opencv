import cv2
from input import get_integer
import display
from time import sleep

class Format:
    def __init__(self, cap, device_name):
        '''
        This init method is called whenever the object for this class is created.
        :param cap: object of VideoCapture class in cv2 module
        :type cap: cv2.VideoCapture
        :param device_name: Name of the selected device
        :type device_name: str
        '''

        self.cap = cap
        self.display1 = display.Display()
        self.device_name = device_name

    @staticmethod
    def decode_fourcc(cc):
        '''
        Method Name: decode_fourcc
        Description: This method converts the fourcc value of format to string.
        :param cc: v4l2_fourcc value of format
        :type cc: int
        :return: format string
        :rtype: str
        '''

        return "".join([chr((int(cc) >> 8 * i) & 0xFF) for i in range(4)])
    def IsRawSaveSupport(self):
        '''
        Method Name: IsRawSaveSupport
        Description: This method Used to get the current format is will support the Raw save or not
        :return: True - If all the child function executed successfully
                or False -  If any of the child function failed
        :rtype: bool
        '''

        format_type = self.decode_fourcc(self.cap.get(cv2.CAP_PROP_FOURCC))
        if ((format_type == "Y8  ") | (format_type == "UYVY") | (format_type == "Y16 ") | (format_type == "YUY2") | (format_type == "YUYV") ):
            return True
        else:
            return False

    def get_current_format(self):
        '''
        Method Name: get_current_format
        Description: This method get the current format of the device
        :return: format type, width, height ,fps
        :rtype: Tuple
        '''
        format_type = self.decode_fourcc(self.cap.get(cv2.CAP_PROP_FOURCC))
        width = int(self.cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        height = int(self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        fps = int(self.cap.get(cv2.CAP_PROP_FPS))
        return format_type, width, height, fps

    def change_format(self):
        '''
        Method Name: change_format
        Description: This method changes the current format of the device to the format requested by the user
        :return: True - If all the child function executed successfully
                or False -  If any of the child function failed
        :rtype: bool
        '''
        format_type, width, height, fps = self.get_current_format()
        print(f"\tCurrent Format={format_type},Width={width},Height={height},FPS={fps}")
        ret, total_formats = self.cap.getFormats()
        if not ret:
            return False
        print("\t0.EXIT\n\t1.BACK")
        for cnt in range(total_formats):
            ret, format_type, width, height, fps = self.cap.getFormatType(cnt)
            if not ret:
                return False
            print(f"\t{cnt + 2}.Format={format_type},Width={width},Height={height},FPS={fps}")
        choice = get_integer("Select the Format:", 0, total_formats + 1)
        if choice == 0:
            return False
        elif choice == 1:
            return True
        else:
            self.display1.stop_display()
            if not self.cap.setFormatType(choice - 2):
                return False
            format_type, width, height, fps = self.get_current_format()
            if ((format_type == "UYVY") | (format_type == "YUY2") | (format_type == "Y12") | (format_type == "Y16 ")):
                if not self.cap.set(cv2.CAP_PROP_CONVERT_RGB, 0):
                    print("Failed to set CAP_PROP_CONVERT_RGB to False")
                else:
                    sleep(0.5)
            else:
                self.cap.set(cv2.CAP_PROP_CONVERT_RGB, 1)
            print(f"\tCurrent Format={format_type},Width={width},Height={height},FPS={fps}")
            self.display1.start_display(self.cap, self.get_current_format(),self.device_name)
            return True
