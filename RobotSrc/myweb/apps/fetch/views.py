from django.shortcuts import render
import os

def grab(request):
    res= ''
    if request.method == 'POST':
        sel = request.POST.get('pic')
        if sel != None:
            os.system("echo %s"%sel)
            os.system('rosrun %s.jpg'%sel)
        else:
            res = 'please choose a label'
    pic = [] 
    for i in range(1, 4):
        pic.append( [i, 'img/pic%d.jpg'%i] )
    return render(request, 'grab.html', {'pic':pic, 'res':res})
