from django.shortcuts import render
from django.core.paginator import Paginator, PageNotAnInteger, EmptyPage, InvalidPage
from django.http import HttpResponse
import MySQLdb

# Create your views here.


def index(request):
    file = open('/home/liubowen/.ros/log/latest/rosout.log')
    log_list = []
    for line in file.readlines():
        log_list.append(line)
    paginator = Paginator(log_list,15)

    if request.method == "GET":
        # 获取 url 后面的 page 参数的值, 首页不显示 page 参数, 默认值是 1
        page = request.GET.get('page')
        try:
            logs = paginator.page(page)
        # todo: 注意捕获异常
        except PageNotAnInteger:
            # 如果请求的页数不是整数, 返回第一页。
            logs = paginator.page(1)
        except InvalidPage:
            # 如果请求的页数不存在, 重定向页面
            return HttpResponse('找不到页面的内容')
        except EmptyPage:
            # 如果请求的页数不在合法的页数范围内，返回结果的最后一页。
            logs = paginator.page(paginator.num_pages)

    return render(request, 'index.html', {'logs':logs})