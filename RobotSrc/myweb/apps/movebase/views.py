from django.shortcuts import render
from django.http import HttpResponse, JsonResponse, HttpResponseNotAllowed
import json
import os
import socket
import multiprocessing
import time

# Create your views here.

HOST = '127.0.0.1'  # 服务器的主机名或者 IP 地址
PORT = 65432        # 服务器使用的端口

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

# def write_file(content):
#     home_path = os.popen('echo $HOME').readlines()[0].strip()
#     path = home_path + '/ros_test/src/my_control/src/keys'
#     fw = open(path,'a')
#     fw.write(content)

def write_file(content):
    s.sendall(content.encode())


def save_map():
    os.system('rosrun map_server map_saver -f map')


def move_map():
    home_path = os.popen('echo $HOME').readlines()[0].strip()
    rootdir = home_path
    goaldir = rootdir + '/catkin_ws/src/team_109/clean_moudle/config'
    os.system('mv ' + rootdir + '/map.pgm' + ' ' + goaldir + '/map109.pgm')
    os.system('mv ' + rootdir + '/map.ymal' + ' ' + goaldir + '/map109.ymal')


def move(request):
    if request.method != "POST":
        return HttpResponseNotAllowed()
    json_data = json.loads(request.POST['json_data'])
    type = json_data['type']
    if type == 'Turn Left':
        write_file('l')
    elif type == 'Forwards':
        write_file('w')
    elif type == 'Turn Right':
        write_file('r')
    elif type == 'Move Left':
        write_file('a')
    elif type == 'Backwards':
        write_file('s')
    elif type == 'Move Right':
        write_file('d')
    elif type == 'Begin':
        pass
    elif type == 'Stop':
        write_file('t')
    elif type == 'Begin Mapping':
        os.system('roslaunch my_map_package my_gmapping.launch')
    elif type == 'Save Your Map':
        save_map()
        time.sleep(5)
        move_map()


    return JsonResponse({"status": "ok"})