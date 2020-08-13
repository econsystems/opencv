import sys

import display
import format
import hid
import uvc
from capture import Capture
from device import Device
from input import get_integer

import cv2


def intro():
    print(" E-con's OpenCV Python Application ".center(100, "*"))
    print('OpenCV Python App Version = 1.0.1'.center(100, " "))
    if sys.platform == "linux":
        print("Running in Linux Platform")
    elif sys.platform == "win32":
        print("Running in Windows Platform")


class MainClass:
    '''
    Main Class: The initiator class. The program starts from here.
    '''

    def __init__(self):
        '''
        this init method initialise all the class objects and starts the main menu.
        '''

        self.vid = None
        self.pid = None
        self.device_path = None
        self.device_name = None
        self.Is_HID_Opened = False
        self.cap = cv2.VideoCapture()
        self.display2 = display.Display()
        self.uvc_obj = uvc.UVCControl(self.cap)
        self.hid = hid.HIDControl()
        self.main_menu_init()
        self.main_menu()

    def main_menu_init(self):
        '''
        Method name: main_menu_init.
        description: This Method calls the device enumeration Method and initialize the hid for selected device
        :return: True - if all the all the statements are executed successfully.
                 or main_menu_exit is called.
        '''

        self.hid.deinit_hid()
        self.display2.stop_display()
        self.display2.Kill_Display_thread()
        device_info = Device.list_devices(self.cap)
        if device_info is None:
            self.main_menu_exit()
        self.device_name, self.vid, self.pid, self.device_path = device_info
        self.format = format.Format(self.cap, self.device_name)
        self.display2.start_display(self.cap, self.format.get_current_format(), self.device_name)
        self.uvc_obj.get_supported_controls()
        if not self.hid.init_hid(self.vid, self.pid, self.device_path):
            print("\tHid initialisation Failed")
            self.Is_HID_Opened = False
        else:
            self.Is_HID_Opened = True
        return True

    def main_menu(self):
        '''
        Method name: main_menu
        description: this Method displays the main menu and allows the user to select various Methodality.
        '''

        main_menu_opt = {
            0: self.main_menu_exit,
            1: self.main_menu_init,
            2: self.uvc_obj.change_uvc_control,
            3: self.format.change_format,
            4: self.still_capture,
            5: self.hid_control_menu,
        }
        while True:
            print("\n\t0.EXIT\n\t1.BACK \n\t2.UVC CONTROLS\n\t3.FORMAT SETTINGS\n\t4.CAPTURE STILL"
                  "IMAGE")
            if self.Is_HID_Opened:      # Should disable the HID Controls option for non e-con devices.
                print("\t5.HID PROPERTIES")
                choice = get_integer("Enter Your Option:", min(main_menu_opt, key=int), max(main_menu_opt, key=int))
            else:
                choice = get_integer("Enter Your Option:", min(main_menu_opt, key=int), max(main_menu_opt, key=int)-1)
            func = main_menu_opt.get(choice, lambda: "Invalid Selection")
            # main_menu_opt.get will return the Method name corresponding to the choice value.
            # for ex: if choice is 1, it will return self.main_menu_init
            if not func():
                self.main_menu_exit()

    def hid_control_menu(self):
        '''
        method name: hid_control_menu
        description: this method shows the hid controls menu and helps the user to read firmware version of the device
        :return: True -  if user wants to go back to main menu or read firmware version is completed.
                False -  if user wants to exit the program.
        '''

        while True:
            print("\n\t0.EXIT\n\t1.BACK\n\t2.Read Firmware Version")
            choice = get_integer("Enter Your Option:", 0, 2)
            if choice == 0:
                return False
            elif choice == 1:
                return True
            else:
                if not self.hid.read_firmware_version():
                    print("Read Firmware Version Failed!!")
                return True

    def still_capture(self):
        '''
        Method name: still_capture
        description: This method enables the flag which is used to capture image.
        :return: True.
        '''
        
        if self.format.IsRawSaveSupport():
            print("\n\t1.BACK\n\t2.RAW FORMAT\n\t3.RGB FORMAT")
            choice = get_integer("Enter Your Option:", 1, 3)
            if choice == 1:
                return True
            if choice == 2:
                print("\tSaving Raw image to the current working directory...")
                if sys.platform == "win32":
                    self.display2.stop_display()
                self.cap.set(cv2.CAP_PROP_CONVERT_RGB, False)
                Capture.capture_flag = True
                Capture.convert_RGB_Selected = True
                if sys.platform == "win32":
                    self.display2.resume_display()
            if choice == 3:
                print("\tSaving image to the current working directory...")
                Capture.capture_flag = True

        else:
            print("\tSaving image to the current working directory...")
            Capture.capture_flag = True
        return True

    def main_menu_exit(self):
        '''
        Method name: main_menu_exit
        description: this method is called before program exists. This method
        releases cap, stop display and de_init hid
        '''

        self.display2.stop_display()
        self.display2.Kill_Display_thread()
        # When everything done, release the capture
        self.cap.release()
        cv2.destroyAllWindows()
        self.hid.deinit_hid()
        exit(0)


# main
if __name__ == '__main__':
    intro()
    main = MainClass()
