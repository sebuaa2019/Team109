#!/usr/bin/env python
'''control ROS Node'''
# license removed for brevity
import rospy
from geometry_msgs.msg import Twist
import sys, select, termios, tty
import socket
 
HOST = '127.0.0.1'
PORT = 65432


def talker():
    '''control Publisher'''
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((HOST, PORT))
    s.listen(5)
    conn, addr = s.accept()

    print 'Connected by', addr
    rospy.init_node('control_pub', anonymous=True)
    vel_pub = rospy.Publisher("/cmd_vel", Twist, queue_size=10)
    c = ''
    now_x = 0
    now_y = 0
    now_z = 0
    while not rospy.is_shutdown():
        data = conn.recv(1024)
        if not data:
            c = ' '
        else:
            c = data[0]

        if c == 'w':
            now_x = 0.1
        if c == 'a':
            now_y = 0.1
        if c == 's':
            now_x = -0.1
        if c == 'd':
            now_y = -0.1
        if c == 'l':
            now_z = 0.1
        if c == 'r':
            now_z = -0.1
        if c == 't':
            now_x = 0
            now_y = 0
            now_z = 0

        vel_cmd = Twist()
        vel_cmd.linear.x = now_x
        vel_cmd.linear.y = now_y
        vel_cmd.linear.z = 0
        vel_cmd.angular.x = 0
        vel_cmd.angular.y = 0
        vel_cmd.angular.z = now_z

        vel_pub.publish(vel_cmd)


if __name__ == '__main__':
    try:
        talker()
    except rospy.ROSInterruptException:
        pass
