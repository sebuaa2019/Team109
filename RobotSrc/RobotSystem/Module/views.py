import os
from django.shortcuts import render
from django.http import HttpResponse

def clean(request):
    if request.method == 'POST':
        #user auth should be done
        os.system("ls")
        return HttpResponse('cleaned')
    else:
        return render(request,'clean.html')
# Create your views here.
