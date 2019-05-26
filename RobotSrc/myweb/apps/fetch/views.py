from django.shortcuts import render
import os
import re

def grab(request):
    pic = [] 
    for i in range(1, 4):
        pic.append( [i, 'img/pic%d.jpg'%i] )
    res= ''
    if 'shop' in request.POST:
        os.system('roslaunch wpb_home_apps shopping.launch')
        return render(request, 'grab.html', {'pic':pic, 'res':res})

    if request.method == 'POST':
        sel = request.POST.get('pic')
        if type(sel) is str and  re.match('pic[1-3]',sel) :
            os.system('roslaunch grab_109 grab_obj.launch')
        else:
            res = 'please choose a label'
    return render(request, 'grab.html', {'pic':pic, 'res':res})
