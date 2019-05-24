from django.shortcuts import render
import os
import re

def grab(request):
    res= ''
    if request.method == 'POST':
        sel = request.POST.get('pic')
        if type(sel) is str and  re.match('pic[1-3]',sel) :
            #os.system('echo %s'%sel)
            os.system('roslaunch grab_109 grab_obj.launch')
        else:
            res = 'please choose a label'
    pic = [] 
    for i in range(1, 4):
        pic.append( [i, 'img/pic%d.jpg'%i] )
    return render(request, 'grab.html', {'pic':pic, 'res':res})
