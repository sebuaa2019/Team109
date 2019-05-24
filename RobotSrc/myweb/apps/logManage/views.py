from django.shortcuts import render
from django.core.paginator import Paginator, PageNotAnInteger, EmptyPage, InvalidPage
from django.http import HttpResponse
import re
import datetime
from logManage.models import logs

# Create your views here.


def index(request):
    file = open("C:/Users/72927/Desktop/rosout.log")
    list = []
    log_list = [[] for i in range(3)]
    for line in file.readlines():
        list.append(line)

    pattern = r'(\d+\.\d+) ([A-Z]+)? (.*)'

    for x in list:
        obj = re.match(pattern, x)
        # print(obj.group(1))
        timestr = obj.group(1)
        timeStamp = float(timestr)
        timeArray = datetime.datetime.utcfromtimestamp(timeStamp)
        stdTime = timeArray.strftime("%Y-%m-%d:%H:%M:%S")
        # print(stdTime)
        log_list[0].append(stdTime)
        if obj.group(2) is None:
            log_list[1].append("BASE")
        else:
            log_list[1].append(obj.group(2))
        log_list[2].append(obj.group(3))

        Time = stdTime
        if obj.group(2) is None:
            Type = "BASE"
        else :
            Type = obj.group(2)
        Info = obj.group(3)
        logs.objects.get_or_create(time=Time,type=Type,info=Info)

    log_list = logs.objects.all().reverse()
    paginator = Paginator(log_list,15)

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

    return render(request, 'index.html', {'logs':Logs})