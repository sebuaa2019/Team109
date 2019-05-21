#!/usr/bin/env python
'''control ROS Node'''
# license removed for brevity
import rospy
from geometry_msgs.msg import Twist
import sys, select, termios, tty


def get_key_board():
    tty.setraw(sys.stdin.fileno())
    rlist, _, _ = select.select([sys.stdin], [], [], 0.1)
    if rlist:
        key = sys.stdin.read(1)
    else:
        key = ''
    termios.tcsetattr(sys.stdin, termios.TCSADRAIN, settings)
    return key


def set_key():
    linear_control = {'w':(1,0,0),'s':(-1,0,0),'a':(0,1,0),'d':(0,-1,0)}
    angular_control = {'j':(0,0,1),'l':(0,0,-1)}
    return linear_control,angular_control


def talker():
    '''control Publisher'''
    rospy.init_node('control_pub', anonymous=True)
    vel_pub = rospy.Publisher("/cmd_vel", Twist, queue_size=10)

    linear_control,angular_control = set_key()
    linear_speed_x = 0.1
    linear_speed_y = 0.1
    angular_speed = 1
    constant = 0
    now_control = (0,0,0)

    print "Control The Robot!"
    
    while not rospy.is_shutdown():
        ins = get_key_board()
        if ins in linear_control.keys():
            now_control = linear_control[ins]
            constant = 0

        elif ins in angular_control.keys():
            now_control = angular_control[ins]
            constant = 0

        elif ins == '1':
            if linear_speed_x < 0.5:
                linear_speed_x += 0.05
            if linear_speed_y < 0.5:
                linear_speed_y += 0.05

        elif ins == '2':
            if linear_speed_x > 0.1:
                linear_speed_x -= 0.1
            if linear_speed_y > 0.1:
                linear_speed_y -= 0.1

        elif ins == ' ':
            now_control = (0,0,0)
        
        else:
            constant += 1
            if constant > 5:
                now_control = (0,0,0)

        now_speed_x = linear_speed_x * now_control[0]
        now_speed_y = linear_speed_y * now_control[1]
        now_angular_speed = angular_speed * now_control[2]


        vel_cmd = Twist()
        vel_cmd.linear.x = now_speed_x
        vel_cmd.linear.y = now_speed_y
        vel_cmd.linear.z = 0
        vel_cmd.angular.x = 0
        vel_cmd.angular.y = 0
        vel_cmd.angular.z = now_angular_speed
        vel_pub.publish(vel_cmd)


    # rate = rospy.Rate(10) # 10hz
    # while not rospy.is_shutdown():
    #     hello_str = "hello world %s" % rospy.get_time()
    #     rospy.loginfo(hello_str)
    #     pub.publish(hello_str)
    #     rate.sleep()

if __name__ == '__main__':
    settings = termios.tcgetattr(sys.stdin)
    try:
        talker()
    except rospy.ROSInterruptException:
        pass
    termios.tcsetattr(sys.stdin, termios.TCSADRAIN, settings)
