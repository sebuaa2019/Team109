from django.shortcuts import render
from django.core.paginator import Paginator, PageNotAnInteger, EmptyPage, InvalidPage
from django.http import HttpResponse
import os
import re
import datetime
from .models import logs

# Create your views here.


def update_log():
    try:
        file = open("/home/robot/.ros/log/latest/rosout.log")
    except:
        file = open("./rosout.log")
    li = []
    log_list = []
    for line in file.readlines():
        li.append(line)

    pattern = r'(\d+\.\d+) ([A-Z]+)? (.*)'

    i = -1
    for x in li:
        obj = re.match(pattern, x)
        if obj :
            timestr = obj.group(1)
            timeStamp = float(timestr)
            timeArray = datetime.datetime.utcfromtimestamp(timeStamp)
            stdTime = timeArray.strftime("%Y-%m-%d %H:%M:%S")

            Time = stdTime
            Type = "BASE"
            if not obj.group(2) is None:
                Type = obj.group(2)
            Info = obj.group(3)
            log_list.append([Time, Type, Info])
            i = i + 1
        elif i >= 0 :
            log_list[i][2] += x

    for x in log_list:
        Time = x[0]
        Type = x[1]
        Info = x[2]
        logs.objects.get_or_create(time=Time,type=Type,info=Info)
    

def index(request):
    update_log()

    log_list = logs.objects.all().order_by("-time")
    paginator = Paginator(log_list, 10)

    if request.method == "GET":
        # 获取 url 后面的 page 参数的值, 首页不显示 page 参数, 默认值是 1
        page = request.GET.get('page')
        try:
            Logs = paginator.page(page)
        # todo: 注意捕获异常
        except PageNotAnInteger:
            # 如果请求的页数不是整数, 返回第一页。
            Logs = paginator.page(1)
        except InvalidPage:
            # 如果请求的页数不存在, 重定向页面
            return HttpResponse('找不到页面的内容')
        except EmptyPage:
            # 如果请求的页数不在合法的页数范围内，返回结果的最后一页。
            Logs = paginator.page(paginator.num_pages)

    return render(request, 'mylog.html', {'logs':Logs})
