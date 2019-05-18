#include <ros/ros.h>
#include <geometry_msgs/Pose2D.h>

static geometry_msgs::Pose2D pose_diff;

void PoseDiffCallback(const geometry_msgs::Pose2D::ConstPtr& msg)
{
    pose_diff.x = msg->x;
    pose_diff.y = msg->y;
    pose_diff.theta = msg->theta;
    ROS_INFO("[test_pose] pose_diff: %f %f\n", pose_diff.x, pose_diff.y);
}

int main(int argc, char** argv){
    ros::init(argc, argv, "test_pose");
    ros::NodeHandle nh;

    ros::Subscriber pose_diff_sub = nh.subscribe("/wpb_home/pose_diff", 1, PoseDiffCallback);

    ros::spin();

    return 0;
}