from django.shortcuts import render
from django.http import HttpResponse, JsonResponse, HttpResponseNotAllowed
import json
import os

# Create your views here.


def write_file(content):
    home_path = os.popen('echo $HOME').readlines()[0].strip()
    path = home_path + '/ros_test/src/my_control/src/keys'
    fw = open(path,'a')
    fw.write(content)


def move_map():
    home_path = os.popen('echo $HOME').readlines()[0].strip()
    rootdir = home_path
    goaldir = home_path + '/catkin_ws/src/wpb_home/wpb_home_tutorials/maps'
    os.system('mv ' + rootdir + '/map.pgm' + ' ' + goaldir + '/map.pgm')
    os.system('mv ' + rootdir + '/map.ymal' + ' ' + goaldir + '/map.ymal')


def move(request):
    if request.method != "POST":
        return HttpResponseNotAllowed()
    json_data = json.loads(request.POST['json_data'])
    type = json_data['type']
    if type == 'Turn Left':
        write_file('l\n')
    elif type == 'Forwards':
        write_file('w\n')
    elif type == 'Turn Right':
        write_file('r\n')
    elif type == 'Move Left':
        write_file('a\n')
    elif type == 'Backward':
        write_file('s\n')
    elif type == 'Move Right':
        write_file('d\n')
    elif type == 'Begin':
        os.system('roslaunch wpb_home_bringup minimal.launch')
    elif type == 'Stop':
        write_file('t\n')
    elif type == 'Begin Mapping':
        os.system('roslaunch my_map_package my_gmapping.launch')
    elif type == 'Save Your Map':
        os.system('rosrun map_server map_saver -f map')
        move_map()

    return JsonResponse({"status": "ok"})