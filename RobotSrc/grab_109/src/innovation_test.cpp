#include <ros/ros.h>
#include <std_msgs/String.h>
#include "grab_109/innovation_script.h"

static CInnovationScript inno_script;

void KeywordCB(const std_msgs::String::ConstPtr & msg)
{
    //ROS_WARN("[inno_script_KeywordCB] - %s",msg->data.c_str());
    string strListen = msg->data;
    inno_script.strListen = strListen;
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "inno_script");
    ROS_INFO("[main] inno_script");
    inno_script.Init();
    inno_script.Queue();
    inno_script.ShowActs();

    ros::NodeHandle n;
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, KeywordCB);
    ros::Rate r(10);
    ros::spinOnce();
    while(ros::ok())
    {
        inno_script.Main();
        ros::spinOnce();
        r.sleep();
    }

    return 0;
}
