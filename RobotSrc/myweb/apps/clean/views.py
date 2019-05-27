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



def init():
    try:
        os.system('rosnode kill rviz')
    except:
        print('kill rviz failed')
    os.system('roslaunch clean_module my_clean.launch')


def robot_clean(mode, level):
    #time.sleep(0.5)
    os.system('rosrun clean_module dfs_clean %d %d'%(mode,level))


def change_map():
    im = Image.open('./static/img/map.pgm')
    im = im.crop((400, 400, 600, 600))
    im.save('./static/img/map.jpg')

def clean_info(request):
    info = [0,0,0]
    try:
        info_str = logs.objects.order_by("-time").filter(info__contains='cleaninfo')[0].info
        pat = re.match('.*cleaninfo ([0-9]+) ([0-9]+) ([0-9]+)',info_str)
        if pat:
            info[0] = pat.group(1)
            info[1] = pat.group(2)
            info[2] = pat.group(3)
    except:
        pass
    data = {'info0':info[0],'info1':info[1],'info2':info[2]}
    #return JsonResponse(json.dumps(data), content_type='application/json')
    print(data)
    return JsonResponse(data)


def clean(request):
    change_map()
    update_log()
    info = [0,0,0]
    try:
        info_str = logs.objects.order_by("-time").filter(info__contains='cleaninfo')[0].info
        pat = re.match('.*cleaninfo ([0-9]+) ([0-9]+) ([0-9]+)',info_str)
        if pat:
            info[0] = pat.group(1)
            info[1] = pat.group(2)
            info[2] = pat.group(3)
    except:
        pass

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

        p = multiprocessing.Process(target=robot_clean, args=(mode,level,))
        p.start()
        p.join()

        return render(request, 'clean.html', {'res':'', 'info':info})
    else:
        return render(request, 'clean.html', {'res':'', 'info':info})
    
