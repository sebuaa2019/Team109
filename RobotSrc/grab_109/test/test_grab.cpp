#include <ros/ros.h>
#include <std_msgs/String.h>

static ros::Publisher behaviors_pub;
static std_msgs::String behavior_msg;

void GrabResultCB(const std_msgs::String::ConstPtr &msg)
{
	ROS_WARN("[GrabResultCB] %s", msg->data.c_str());
}
	
int 
main(int argc, char **argv)
{
	ros::init(argc, argv, "test_grab");
	
	ros::NodeHandle n;
	
	behaviors_pub = n.advertise<std_msgs::String>("/wpb_home/behaviors", 3);
	ros::Subscriber res_sub = n.subscribe("/wpb_home/grab_result", 30, GrabResultCB);
	
	ROS_WARN("[main] grab_test");
	sleep(1);
	
	behavior_msg.data = "grab start";
	behaviors_pub.publish(behavior_msg);
	
	ros::Rate r(30);
	while(ros::ok()){
		ros::spinOnece();
		r.sleep();
	}
	
	return 0;
	
}
