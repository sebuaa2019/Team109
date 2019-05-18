#include <ros/ros.h>
#include <std_msgs/String.h>

void voiceCB(const std_msgs::String::ConstPtr &msg){
    ROS_INFO("[test_voice]: %s\n", msg->data.c_str());
}

int main(int argc, char** argv){
    ros::init(argc, argv, "test_voice");
    ros::NodeHandle n;
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, voiceCB);

    ros::spin();

    return 0;
}
