# -*- encoding: utf-8 -*-
"""
@File    : facedetect.py
@Time    : 2020/9/15 20:10
@Author  : hurui
@Email   : 18476856458@163.com
@Software: PyCharm
"""
import platform
from ctypes import *
facelist = []

def _callback(para, imagepath):
    obj = para.__getitem__(0)
    facedict = {'x': obj.x,
                'y': obj.y,
                'w': obj.w,
                'h': obj.h,
                'confidence': obj.confidence,
                'angle': obj.angle,
                'base64origin': str(obj.base64origin, encoding='utf-8'),
                'base64crop': str(obj.base64crop, encoding='utf-8')
                }
    facelist.append(facedict)
    #faceinfo = [obj.x, obj.y, obj.w, obj.h]
    #tu_face = CropImage4File(imagepath, faceinfo)
    #print(facedict)

class faceframe(Structure):
    _fields_ = [
        ('x', c_int),
        ('y', c_int),
        ('w', c_int),
        ('h', c_int),
        ('confidence', c_uint),
        ('angle', c_uint),
        ('base64origin', c_char_p),
        ('base64crop', c_char_p)
    ]

CALLBACK = CFUNCTYPE(None, POINTER(faceframe), c_char_p)

class facedetectdll:
    def __init__(self):
        self.libface = None
        if platform.system() == "Windows":
            self.libface = cdll.LoadLibrary("facedetec.dll")
        elif platform.system() == "Linux":
            self.libface = cdll.LoadLibrary("./so/cppdll.so")
        else:
            print(f"unknown platform.")
        return
    def facedetect(self, callback, imgpath):
        if(self.libface !=None):
            self.libface.facedetect.argtypes = [CALLBACK, c_char_p]
            self.libface.facedetect.restype = c_int
            imagepath = bytes(imgpath, "utf-8")
            #ret_str = self.libface.facedetect(imagepath).decode("utf8")
        return self.libface.facedetect(callback, imagepath)

def getfacedetect(imagepath):
    faces = facedetectdll()
    callBackFunc = CALLBACK(_callback)
    ret_str = faces.facedetect(callBackFunc, imagepath)
    if(ret_str ==0):
        return facelist


'''
if __name__ == '__main__':
    faces = getfacedetect("D:\\test\\testScrip\\smartcom\\faced\\keliamoniz1.jpg")
    print(faces)
'''