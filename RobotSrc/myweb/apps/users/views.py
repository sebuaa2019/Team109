from django.shortcuts import render
from django.contrib.auth import authenticate, login, logout
from .forms import UserRegisterForm, ModifyUserInfoForm
from .models import UserInfo
from django.contrib.auth.hashers import make_password
from django.http import HttpResponseRedirect, HttpResponse
from django.urls import reverse
import json
# Create your views here.


def user_login(request):
    if request.method == "POST":
        user_name = request.POST.get('username', '')
        pass_word = request.POST.get('password', '')
        user = authenticate(username=user_name, password=pass_word)
        if user is not None:
            login(request, user)
            return render(request, 'introduction.html')
        else:
            return render(request, "login.html", {'msg': '用户名或者密码错误！'})
    elif request.method == "GET":
        return render(request, "login.html", {})


def user_logout(request):
    if request.method == "GET":
        logout(request)
        return HttpResponseRedirect(reverse("home"))


def user_info(request):
    if request.method == "GET":
        return render(request, "profile.html", {})
    elif request.method == "POST":
        user_info_form = ModifyUserInfoForm(request.POST, instance=request.user)
        user_info_form.save()
        return HttpResponse(
            '{"status":"success"}',
            content_type='application/json')


def user_register(request):
    if request.method == "POST":
        register_form = UserRegisterForm(request.POST)
        if register_form.is_valid():
            user_name = request.POST.get("username", "")
            if UserInfo.objects.filter(username=user_name):
                return render(request, "register.html", {"register_form": register_form, "msg": "用户已存在"})
            pass_word = request.POST.get("password", "")
            user_info = UserInfo()
            user_info.username = user_name
            user_info.email = request.POST.get("email", "")
            user_info.is_active = True
            user_info.password = make_password(pass_word)
            user_info.save()
            return render(request, "login.html", )
        else:
            return render(request, "register.html", {"register_form": register_form})
    elif request.method == "GET":
        register_form = UserRegisterForm()
        return render(request, "register.html", {'register_form': register_form})