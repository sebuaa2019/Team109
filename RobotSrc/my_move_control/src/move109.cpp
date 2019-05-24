#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <fstream>
#include <iostream>
#include <string>
#include <queue>

std::queue<char> q;

void get_file(){
    std::string s;
    std::ifstream in("keys",std::ios::in);
    while(in >> s){
        q.push(s[0]);
    }
    std::ofstream out("keys",std::ios::out);
}


int main(int argc, char** argv){
    ros::init(argc, argv, "move109");
    ros::NodeHandle n;
    ros::Publisher vel_pub = n.advertise<geometry_msgs::Twist>("/cmd_vel", 10);

    float linear_speed_x = 0.1;
    float linear_speed_y = 0.1;
    float angular_speed = 1;
    int constant = 0;
    int x = 0,y = 0,z = 0;
    char c;

    while(ros::ok){
        if(q.empty()){
            get_file();
            c = ' ';
        }
        else{
            c = q.front();
            q.pop();
        }
        switch(c){
            case 'w':{
                x = 1;y = 0;z = 0;
                constant = 0;
                break;
            }
            case 'a':{
                x = 0;y = 1;z = 0;
                constant = 0;
                break;
            }
            case 's':{
                x = -1;y = 0;z = 0;
                constant = 0;
                break;
            }
            case 'd':{
                x = 0;y = -1;z = 0;
                constant = 0;
                break;
            }
            case 'l':{
                x = 0;y = 0;z = 1;
                constant = 0;
                break;
            }
            case 'r':{
                x = 0;y = 0;z = -1;
                constant = 0;
                break;
            }
            case 't':{
                x = 0;y = 0;z = 0;
                break;
            }
            case ' ':{
                constant += 1;
                break;
            }
        }
        float now_speed_x = linear_speed_x * x;
        float now_speed_y = linear_speed_y * y;
        float now_angular_speed = angular_speed * z;
        geometry_msgs::Twist vel_cmd;
        vel_cmd.linear.x = now_speed_x;
        vel_cmd.linear.y = now_speed_y;
        vel_cmd.linear.z = 0;
        vel_cmd.angular.x = 0;
        vel_cmd.angular.y = 0;
        vel_cmd.angular.z = now_angular_speed;
        vel_pub.publish(vel_cmd);

        ros::spinOnce();
    }
    return 0;
}
