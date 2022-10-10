import cv2
from input import get_integer


class Device:
    '''
    This Devices class provides method to enumerate devices connected to the system.
    '''

    @staticmethod
    def list_devices(cap):
        '''
        Method name: list_devices
        description: This method enumerates all the video devices connected and allows to select the device.
        :param cap: cv2.Videocapture object
        :return: device name, vendor id, product id, device path - if all the child functions are executes successfully
                or None - if any of the child function is failed.
        '''
        if cap.isOpened():
            cap.release()
            cv2.destroyAllWindows()
        ret, device_count = cap.getDevices()
        if not ret:
            print("cap.getDevices Failed")
            return None
        print(f"Total Number of devices:{device_count}")
        print("\t0.Exit")
        for i in range(0, device_count):
            ret, device_name, vid, pid, device_path = cap.getDeviceInfo(i)
            if not ret:
                print("cap.getDeviceInfo Failed")
                return None
            print(f"\t{i + 1}.{device_name}")
        choice = get_integer("Enter 0 for Exit/Select Any device:", 0, device_count)
        if choice == 0:
            return None
        ret, device_name, vid, pid, device_path = cap.getDeviceInfo(choice - 1)
        if not ret:
            print("cap.getDeviceInfo Failed")
            return None
        cap.open(choice - 1)
        if not cap.isOpened():
            print("cap.open Failed")
            return None
        iFormatType ="".join([chr((int(cap.get(cv2.CAP_PROP_FOURCC)) >> 8 * i) & 0xFF) for i in range(4)])
        if ((iFormatType == "UYVY") | (iFormatType == "YUY2") | (iFormatType == "Y16 ")):
            if not cap.set(cv2.CAP_PROP_CONVERT_RGB, 0):
                print("Setting RGB flag as False")
        return device_name, vid, pid, device_path
