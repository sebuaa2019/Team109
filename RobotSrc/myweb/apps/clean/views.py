# -*- coding: utf-8 -*-

from django.shortcuts import render
from PIL import Image

import multiprocessing
import os
import time


def init(name):
    os.system('ls')
    #os.system('roslauch clean_module my_clean.launch')


def robot_clean(name):
    time.sleep(0.5)
    os.system('echo hello')
    #os.system('rosrun clean_module clean')


def change_map():
    im = Image.open('./static/img/map.pgm')
    im = im.crop((400, 400, 600, 600))
    im.save('./static/img/map.jpg')

def clean(request):
    if request.method == 'POST':
        change_map()
        p = multiprocessing.Process(target=init, args=('1',))
        p.start()
        p = multiprocessing.Process(target=robot_clean, args=('2',))
        p.start()
        p.join()

        return render(request, 'clean.html')
    else:
        return render(request, 'clean.html')

    
"""
        # user auth should be done
        p = multiprocessing.Process(target=init, args=('1',))
        p.start()
        time.sleep(1)
        p = multiprocessing.Process(target=clean, args=('2',))
        p.start()
        p.join()
"""
