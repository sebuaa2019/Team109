from django.shortcuts import render

def grab(request):
    pic = [] 
    for i in range(1,4):
        pic.append( [i, 'img/pic%d.jpg'%i] )
    return render(request, 'grab.html', {'pic':pic})
