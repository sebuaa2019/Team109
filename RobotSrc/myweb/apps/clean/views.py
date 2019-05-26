# -*- coding: utf-8 -*-

from django.shortcuts import render
from PIL import Image

import multiprocessing
import os
import time
import re


def init():
    os.system('roslaunch clean_module my_clean.launch')


def robot_clean(mode, level):
    #time.sleep(0.5)
    os.system('rosrun clean_module dfs_clean %d %d'%(mode,level))


def change_map():
    im = Image.open('./static/img/map.pgm')
    im = im.crop((400, 400, 600, 600))
    im.save('./static/img/map.jpg')

def clean(request):
    info = [1,1,1]
    if request.method == 'POST':
        level = request.POST.get('level') 
        if type(level) is str and re.match('[1-5]', level) :
           #os.system("echo %s"%level)
           level = int(level)
        else:
           res = "repeat time should be 1, 2, 3, 4 or 5" 
           #os.system("echo %s"%level)
           return render(request, 'clean.html', {'res':res, 'info':info})
        
        p = multiprocessing.Process(target=init)
        p.start()

        mode = 0
        if 'zig' in request.POST:
            mode = 1
        change_map()

        p = multiprocessing.Process(target=robot_clean, args=(mode,level,))
        p.start()
        p.join()

        return render(request, 'clean.html', {'res':'', 'info':info})
    else:
        return render(request, 'clean.html', {'res':'', 'info':info})
    
