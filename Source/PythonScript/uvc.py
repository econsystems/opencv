from enum import Enum

import display
import cv2
from input import *#get_integer, get_yes_or_no


class UVCControl:
    '''
    This class provides methods to change UVC controls of the device
    '''

    uvc_controls = {
        cv2.CAP_PROP_BRIGHTNESS: "Brightness",
        cv2.CAP_PROP_CONTRAST: "Contrast",
        cv2.CAP_PROP_SATURATION: "Saturation",
        cv2.CAP_PROP_HUE: "Hue",
        cv2.CAP_PROP_GAIN: "Gain",
        cv2.CAP_PROP_EXPOSURE: "Exposure",
        cv2.CAP_PROP_WHITE_BALANCE_BLUE_U: "White Balance",
        cv2.CAP_PROP_SHARPNESS: "Sharpness",
        cv2.CAP_PROP_GAMMA: "Gamma",
        cv2.CAP_PROP_ZOOM: "Zoom",
        cv2.CAP_PROP_FOCUS: "Focus",
        cv2.CAP_PROP_BACKLIGHT: "BackLight",
        cv2.CAP_PROP_PAN: "Pan",
        cv2.CAP_PROP_TILT: "Tilt",
    }
    minimum = -1
    maximum = -1
    stepping_delta = -1
    supported_mode = -1
    current_value = -1
    current_mode = -1
    default_value = -1

    modes = Enum('MODES', 'AUTO MANUAL AUTO_AND_MANUAL')

    def __init__(self, cap):
        '''
        this init method is called when the object of the class is created.
        :param cap: object of VideoCapture class in opencv
        :type cap: cv2.VideoCapture
        '''
        self.cap = cap
        self.display1 = display.Display()
        self.available_controls = {}

    def get_supported_controls(self):
        '''
        Method Name: get_supported_controls
        Description: This method get the UVC controls supported by the device
        :return: True
        '''
        self.available_controls.clear()
        self.available_controls = self.uvc_controls.copy()

        for i in self.uvc_controls:
            ret, minimum, maximum, stepping_delta, supported_mode, current_value, current_mode, default_value = \
                self.cap.get(i, self.minimum, self.maximum, self.stepping_delta, self.supported_mode,
                             self.current_value, self.current_mode, self.default_value)
            if not ret:
                self.available_controls.pop(i)

        return True

    def change_uvc_control(self):
        '''
        Method Name: change_uvc_control
        Description: This method allows the user to select any of the supported UVC control and change tha value.
        :return: True - if all the child functions are executes successfully
                or False - if any of the child function is failed.
        '''
        set_value = 0
        while True:
            print("\n\t0 Exit\n\t1 Back")
            for cnt, key in enumerate(self.available_controls):
                print("\t{} {}".format(cnt + 2, self.available_controls[key]))
            choice = get_integer("Select Any Control:", 0, len(self.available_controls) + 1)
            if choice == 0:
                return False
            elif choice == 1:
                return True
            else:
                control_id, control_name = next(
                    (key, value) for i, (key, value) in enumerate(self.available_controls.items()) if i == choice - 2)
                while True:
                    ret, minimum, maximum, stepping_delta, supported_mode, current_value, current_mode, default_value = \
                    self.cap.get(control_id, self.minimum, self.maximum, self.stepping_delta, self.supported_mode,
                    self.current_value, self.current_mode, self.default_value)
                    if not ret:
                        print("Get control Values failed!!")
                        return False
                    print(f"Camera {control_name} Control Values:")
                    print(f"\tMinimum Value: {minimum}\n\tMaximum Value: {maximum}\n\tDefault Value: {default_value}\n"
                          f"\tStep Value: {stepping_delta}\n\tCurrent Value: {current_value}\n"
                          f"\tSupported Mode: {self.modes(supported_mode).name}\n"
                          f"\tCurrent Mode: {self.modes(current_mode).name}")
                    if supported_mode != self.modes.AUTO_AND_MANUAL.value:
                        print(f"Only {self.modes(current_mode).name} {control_name} Control is supported by the camera,"
                              f"User can't set the mode!")
                        set_mode = current_mode
                    else:
                        set_mode = get_integer("\n\t1 AUTO\n\t2 MANUAL\nSelect Any mode:", self.modes.AUTO.value,
                                               self.modes.AUTO_AND_MANUAL.value, "Enter a valid mode")
                    if set_mode == self.modes.MANUAL.value:
                        set_value = get_integer("Enter a Valid value:", minimum, maximum,
                                                "Certain condition not met,Please",
                                                (lambda val: val % stepping_delta == 0))
                    if self.cap.set(control_id, set_value, set_mode):
                        print(f"{self.modes(set_mode).name} {control_name} is set successfully")
                    else:
                        print("Set Control Failed!")
                        break
                    print(f"Continue again to set {control_name}?")
                    if get_yes_or_no():
                        continue
                    else:
                        break
