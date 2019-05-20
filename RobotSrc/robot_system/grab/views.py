# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.shortcuts import render

def grab(request):
    return render(request, 'grab.html')

# Create your views here.
