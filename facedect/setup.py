# -*- encoding: utf-8 -*-
"""
@File    : facedetect.py
@Time    : 2020/9/15 20:10
@Author  : hurui
@Email   : 18476856458@163.com
@Software: PyCharm
"""
from setuptools import setup
import sys,os
path = sys.executable
folder = path[0: path.rfind(os.sep)]
setup(
        name='facedetection',
        version='1.0',
        author='hurui',
        author_email='frack.hu@gmail.com',
        url='https://e.coding.net/timelessness/facedetect-sdk/facedetect-sdk.git',
        py_modules=['facedetection.facedetect'], 
        data_files=[(folder, ['facedetection/facedetec.dll',
                              'facedetection/opencv_world412.dll']
                     )
                    ]
        )