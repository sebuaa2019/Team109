"""myweb URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/2.0/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
import os
from django.contrib import admin
from django.urls import path,include
from django.conf.urls import url 
from django.views.generic import TemplateView
import movebase.views as api
from logManage.views import index
from fetch.views import grab
from clean.views import clean, clean_info
from users.views import user_login, user_logout, user_register


urlpatterns = [
    path('admin/', admin.site.urls),
    path('', TemplateView.as_view(template_name='introduction.html'), name='index'),
    path('home/',TemplateView.as_view(template_name='introduction.html'),name = 'home'),
    path('movebase/', TemplateView.as_view(template_name='movebase.html'), name = 'movebase'),
    path('api/move', api.move, name="api_move"),
    path('Log/',index),
    path('grab/',grab),
    path('clean/',clean),
    path('clean_info/',clean_info, name='clean_info'),
    path('login/',user_login, name='login'),
    path('logout/', user_logout, name="logout"),
    path('register/', user_register, name='register'),
    path('profile/', TemplateView.as_view(template_name='profile.html'), name='profile'),
]
