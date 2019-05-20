# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.shortcuts import render
from PIL import Image
import os, glob

import multiprocessing
import os
import time

def init(name):
    os.system('roslauch clean_module my_clean.launch')
def clean(name):
    os.system('rosrun clean_module clean')

def change_map():
    im = Image.open('./static/img/map.pgm')
    im = im.crop((400,400,600,600))
    im.save('./static/img/map.jpg')


def index(request):
    return render(request, 'index.html')

def clean(request):
    if request.method == 'POST':
        change_map()
        
        #user auth should be done
        p = multiprocessing.Process(target=init, args=('1',))
        p.start()
        time.sleep(1)
        p = multiprocessing.Process(target=clean, args=('2',))
        p.start()
        p.join()
        return render(request,'clean.html')
    else:
        return render(request,'clean.html')
