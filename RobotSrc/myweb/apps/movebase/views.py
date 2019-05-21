from django.shortcuts import render
from django.http import HttpResponse, JsonResponse, HttpResponseNotAllowed
import json
import os

# Create your views here.

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
        os.system('rosrun my_base_package_109 turnleft')
    elif type == 'Forwards':
        os.system('rosrun my_base_package_109 forwards')
    elif type == 'Turn Right':
        os.system('rosrun my_base_package_109 turnright')
    elif type == 'Move Left':
        os.system('rosrun my_base_package_109 moveleft')
    elif type == 'Backward':
        os.system('rosrun my_base_package_109 backwards')
    elif type == 'Move Right':
        os.system('rosrun my_base_package_109 moveright')
    elif type == 'Begin':
        os.system('roslaunch wpb_home_bringup minimal.launch')
    elif type == 'Stop':
        pass
    elif type == 'Begin Mapping':
        os.system('roslaunch my_map_package my_gmapping.launch')
    elif type == 'Save Your Map':
        os.system('rosrun map_server map_saver -f map')
        move_map()

    return JsonResponse({"status": "ok"})