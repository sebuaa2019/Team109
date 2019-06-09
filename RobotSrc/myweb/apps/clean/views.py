# -*- coding: utf-8 -*-

from django.shortcuts import render
from django.http import JsonResponse
from PIL import Image

import multiprocessing
import os
import time
import re
from logManage.models import logs
from logManage.views import update_log 



def init(mode, level):
    os.system('roslaunch clean_module my_clean.launch mode:=%d level:=%d'%(mode,level))


def change_map():
    im = Image.open('./static/img/map.pgm')
    im = im.crop((400, 400, 600, 600))
    im.save('./static/img/map.jpg')


def clean(request):
    update_log()
    info = [0,0,0,0]
    try:
        info_str = logs.objects.order_by("-time").filter(info__contains='cleaninfo')[0].info
        pat = re.match('.*cleaninfo ([0-9]+) ([0-9]+) ([0-9]+)',info_str)
        if pat:
            info[0] = int(pat.group(1)) * 0.2
            info[1] = int(pat.group(2)) * 0.5
            info_tmp = int(pat.group(3))
            info[2] =  info_tmp // 60000
            info[3] =  info_tmp % 60000 // 1000
    except:
        pass

    if request.method == 'POST':
        level = request.POST.get('level') 
        if type(level) is str and re.match('[1-5]', level) :
           #os.system("echo %s"%level)
           level = int(level)-1
        else:
           res = "repeat time should be 1, 2, 3, 4 or 5" 
           #os.system("echo %s"%level)
           return render(request, 'clean.html', {'res':res, 'info':info})
        

        mode = 0
        if 'zig' in request.POST:
            mode = 1
        init(mode, level)


        return render(request, 'clean.html', {'res':'', 'info':info})
    else:
        return render(request, 'clean.html', {'res':'', 'info':info})
    
"""
        p = multiprocessing.Process(target=init)
        p.start()
        p = multiprocessing.Process(target=robot_clean, args=(mode,level,))
        p.start()
        p.join()
"""
