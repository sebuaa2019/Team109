from django.shortcuts import render
import os
import re

path_dic = { 
        'pic1':'/home/catkin_ws/src/team_109/grab_109/label/label1.jpg',
        'pic2':'/home/catkin_ws/src/team_109/grab_109/label/label10.jpg',
        'pic3':'/home/catkin_ws/src/team_109/grab_109/label/label1.jpg'
        }

def init():
    os.system('roslaunch grab_109 grab_obj.launch')


def robot_grab(sel):
    #time.sleep(0.5)
    os.system('rosrun grab_109 grab_obj _templpath:=%s'%(path_dic[sel]))

def grab(request):
    pic = [] 
    for i in range(1, 4):
        pic.append( [i, 'img/pic%d.jpg'%i] )
    res= ''


    if request.method == 'POST':
        try:
            os.system('rosnode kill rviz')
        except:
            print('kill rviz failed')

        if 'shop' in request.POST:
            os.system('roslaunch wpb_home_apps shopping.launch')
            return render(request, 'grab.html', {'pic':pic, 'res':res})
            sel = request.POST.get('pic')
        if type(sel) is str and  re.match('pic[1-3]',sel) :
            p = multiprocessing.Process(target=init)
            p.start()
            p = multiprocessing.Process(target=robot_grab, args=(sel))
            p.start()
            p.join()
        else:
            res = 'please choose a label'
    return render(request, 'grab.html', {'pic':pic, 'res':res})
