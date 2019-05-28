from django.shortcuts import render
import os
import re

path_dic = { 
        'pic1':'default.jpg',
        'pic2':'label10.jpg',
        'pic3':'default.jpg'
        }

def init(sel):
    path = '/home/robot/catkin_ws/src/team_109/grab_109/src/label/'+path_dic[sel]
    os.system('roslaunch grab_109 grab_obj.launch templpath:=%s'%path)


def grab(request):
    pic = [] 
    for i in range(1, 4):
        pic.append( [i, 'img/pic%d.png'%i] )
    res= ''

    if request.method == 'POST':
        if 'shop' in request.POST:
            os.system('roslaunch wpb_home_apps shopping.launch')
            return render(request, 'grab.html', {'pic':pic, 'res':res})
        sel = request.POST.get('pic')
        if type(sel) is str and  re.match('pic[1-3]',sel) :
            init(sel)
        else:
            res = 'please choose a label'
    return render(request, 'grab.html', {'pic':pic, 'res':res})
