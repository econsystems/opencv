import errno
import os
import sys
from select import select
from time import sleep

if sys.platform == "linux":
    from pyudev import Context

if sys.platform == "win32":
    import ctypes


class HIDControl:
    '''
    This class provides method to access hid controls
    '''

    BUFFER_LENGTH = 65
    READ_FIRMWARE_VERSION = 0x40

    def __init__(self):
        '''
        This init method is called when the object of this class is created.
        '''

        if sys.platform == "linux":
            self.hid_device_path = ''
            self.hid_handle = None
        if sys.platform == "win32":
            self.eCAM_dll = ctypes.cdll.LoadLibrary("eCAMFwSw.dll")  # for windows, eCAMFwSW dll is loaded.


    def init_hid(self, vid, pid, device_path):
        '''
        Method Name: This method init the hid handle
        :param vid: The vendor id of the device
        :type vid: str
        :param pid: product id of the selected device
        :type pid: str
        :param device_path: device path at which the device is connected to the system
        :type device_path: str
        :return: True or False
        :rtype: bool
        '''


        if sys.platform == "linux":
            if self.get_hid_device_path(vid, pid):
                self.hid_handle = self.open_hid_handle()
                return True
            return False
        elif sys.platform == "win32":
            if not self.eCAM_dll:
                return False

            return self.eCAM_dll.InitExtensionUnit(device_path)

    def open_hid_handle(self):
        '''
        Method Name: open_hid_handle
        Description: This method open the hid handle of the device
        :return: True or False
        :rtype: bool
        '''

        return os.open(self.hid_device_path, os.O_RDWR, os.O_NONBLOCK)

    def get_hid_device_path(self, vid, pid):
        '''
        Method Name: get_hid_device_path
        Description: This method get the device path of device corresponding to the vid and pid
        :param vid: The vendor id of the device
        :type vid: str
        :param pid: product id of the selected device
        :type pid: str
        :return: True or False
        :rtype: bool
        '''

        device_count = 0
        for device in Context().list_devices(subsystem='hidraw'):
            usb_device = device.find_parent('usb', 'usb_device')
            if usb_device:
                vendor_id = usb_device.get('ID_VENDOR_ID')
                product_id = usb_device.get('ID_MODEL_ID')
                if str(vendor_id) == vid and str(product_id) == pid:  # need to add [and device_count == device_number]
                    self.hid_device_path = device.device_node
                    return True
            device_count += 1
        return False

    def hid_write(self, input_buffer):
        '''
        Method Name: hid_write
        Description: This method writes the input buffer on to the hid handle
        :param input_buffer: the data which needs to be written
        :type input_buffer: bytearray
        :return: True or False
        :rtype: bool
        '''

        if not self.hid_handle:
            return False
        if input_buffer is not None:
            retry_count = 0
            bytes_written = 0
            while retry_count < 3:
                try:
                    bytes_written = os.write(self.hid_handle, input_buffer)
                    retry_count += 1
                except IOError as e:
                    if e.errno == errno.EPIPE:
                        sleep(0.1)
                else:
                    break
            if bytes_written != len(input_buffer):
                raise IOError(errno.EIO, 'Written %d bytes out of expected %d' % (bytes_written, len(input_buffer)))
        return True

    def hid_read(self):
        '''
        Method Name: hid_read
        Description: This method reads data from the hid handle of the device
        :return: data which is read from hid handle
        :rtype: bytearray
        '''

        if not self.hid_handle:
            print("Invalid HID Handle")
            return False
        output_buffer = None
        timeout = 2000.0
        rlist, wlist, xlist = select([self.hid_handle], [], [self.hid_handle], timeout)

        if xlist:
            if xlist == [self.hid_handle]:
                raise IOError(errno.EIO, 'exception on file descriptor %d' % self.hid_handle)

        if rlist:
            if rlist == [self.hid_handle]:
                output_buffer = os.read(self.hid_handle, self.BUFFER_LENGTH)
                if output_buffer is None:
                    return b''
        return output_buffer

    def read_firmware_version(self):
        '''
        Method Name: read_firmware_version
        Description: This method read the firmware version of the device
        :return: True or False
        :rtype: bool
        '''

        if sys.platform == "linux":
            if not self.hid_handle:
                return False
            input_buffer = bytearray([0] * self.BUFFER_LENGTH)
            input_buffer[0] = 0x00
            input_buffer[1] = self.READ_FIRMWARE_VERSION
            self.hid_write(input_buffer)
            output_buffer = self.hid_read()
            if output_buffer[0] != self.READ_FIRMWARE_VERSION:
                return False
            SDK_VER = (output_buffer[3] << 8) + output_buffer[4]
            SVN_VER = (output_buffer[5] << 8) + output_buffer[6]
            pMajorVersion = output_buffer[1]
            pMinorVersion1 = output_buffer[2]
            print(f"\n\tFirmware Version is {pMajorVersion}.{pMinorVersion1}.{SDK_VER}.{SVN_VER}")
            return True
        elif sys.platform == "win32":
            if not self.eCAM_dll:
                return False
            pMajorVersion = ctypes.c_uint8()
            pMinorVersion1 = ctypes.c_uint8()
            SDK_VER = ctypes.c_uint16()
            SVN_VER = ctypes.c_uint16()

            res = self.eCAM_dll.ReadFirmwareVersion(ctypes.byref(pMajorVersion), ctypes.byref(pMinorVersion1),
                                                    ctypes.byref(SDK_VER), ctypes.byref(SVN_VER))
            if not res:
                return False
            else:
                print(f"\n\tFirmware Version is {pMajorVersion.value}.{pMinorVersion1.value}."
                      f"{SDK_VER.value}.{SVN_VER.value}")
            return True

    def deinit_hid(self):
        '''
        Method Name: deinit_hid
        Description: This method closes the hid handle of the device
        :return: True or False
        :rtype: bool
        '''
        if sys.platform == "linux":
            if self.hid_handle is not None:
                return os.close(self.hid_handle)
        elif sys.platform == "win32":
            if not self.eCAM_dll:
                return False
            return self.eCAM_dll.DeinitExtensionUnit()
