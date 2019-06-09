#include <ros/ros.h>
#include <pcl_ros/point_cloud.h>
#include <pcl/point_types.h>
#include <boost/foreach.hpp>
#include <pcl/io/pcd_io.h>
/*#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/surface/convex_hull.h>
#include <pcl/segmentation/extract_polygonal_prism_data.h> 
#include <pcl/visualization/cloud_viewer.h>
#include <pcl/segmentation/extract_clusters.h>*/

#include <image_geometry/pinhole_camera_model.h>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <sensor_msgs/Image.h>
#include <pcl_ros/transforms.h>
#include <sensor_msgs/PointCloud2.h>
#include <pcl_conversions/pcl_conversions.h>
#include <geometry_msgs/PointStamped.h>
#include <tf/transform_listener.h>
#include <visualization_msgs/Marker.h>
#include <sensor_msgs/JointState.h>
#include <std_msgs/String.h>
#include <sound_play/SoundRequest.h>
#include <geometry_msgs/Pose2D.h>
#include "highgui.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <vector>

static std::string save_path = "/home/robot/catkin_ws/src/team_109/grab_109/src/pic/test_detect.jpg";

void 
testProcImageCB(const sensor_msgs::ImageConstPtr& msg)
{
	//ROS_INFO("[callbackRGB] nStep = %d", nStep);
	//if(nStep != STEP_FIND_OBJ && nStep != STEP_FIND_PLANE) return;
	
	//将传感器图片转为Mat
	cv_bridge::CvImagePtr cv_ptr;
	try{
		cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
	}catch(cv_bridge::Exception& e){
		ROS_ERROR("cv_bridge exception: %s", e.what());
		return;
	}
	//cv_ptr->image.convertTo(frame_gray);
	//cvtColor(cv_ptr->image, frame_gray, CV_BGR2GRAY);
	
	ROS_INFO("[test detect] match template");
	
	matchTemplate(cv_ptr->image, templ, objects, match_method);
	
	normalize(objects, objects, 0, 1, NORM_MINMAX, -1, Mat() );
	double minVal, maxVal, val;
	Point minLoc, maxLoc, matchLoc;
	
	minMaxLoc(objects, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	if( match_method  == TM_SQDIFF || match_method == TM_SQDIFF_NORMED ){
		matchLoc = minLoc; 
		val = minVal;
	}
	else{ 
		matchLoc = maxLoc; 
		val = maxVal;
	}
	
	tl.x = matchLoc.x; tl.y = matchLoc.y;
	br.x = tl.x + templ.cols; br.y = tl.y + templ.rows;
	
	//obj_tp_track.x = matchLoc.x; obj_tp_track.y = matchLoc.y;
	//obj_tp_track.width = templ.cols; obj_tp_track.height = templ.rows;
	
	if(val > thresholdVal){ // 检测到标签

		find_obj = true;
		
		rectangle(cv_ptr->image,
		tl,
		br,
		CV_RGB(255, 0 , 255),
		10);
		
		imwrite(save_path, cv_ptr->image);
		
		ROS_INFO("[test detect] find object (x,y,width,height)=(%d,%d,%d,%d) maxVal=%.2f", tl.x,tl.y,templ.cols,templ.rows,val);
		
	}else{
		find_obj = false;
		ROS_INFO("[test detect] no object");
	}
}

int 
main(int argc, char** argv)
{
	ros::init(argc, argv, "test_detect");
	ROS_INFO("[test detect]");
	tf_listener = new tf::TransformListener();
	
	ros::NodeHandle nh_param("~");
	//nh_param.param<std::string>("rgb_topic", rgb_topic, "/kinect2/qhd/image_color");
	//nh_param.param<std::string>("topic", pc_topic, "/kinect2/qhd/points");
	
	rgb_topic = "/kinect2/qhd/image_color";
	//pc_topic = "/kinect2/qhd/points";

	std::string templpath = "";
	//ros::param::get("templpath", templpath);
	nh_param.param<std::string>("templpath", templpath, "");

	ROS_INFO("[test detect] templpath: %s\n", templpath.c_str());
	
	templ = imread(templpath);
	if(templ.data == NULL){
		//ROS_INFO("(w, h)=(%d, %d)", templ.cols, templ.rows);
		ROS_ERROR("[test detect] the file path %s does not exist", templpath.c_str());
		return -1;
	}
	
	
	ros::NodeHandle nh;
	
	ros::Subscriber rgb_sub = nh.subscribe(rgb_topic, 1, testProcImageCB);
	
	ros::Rate r(30);
	while(ros::ok()){
		ros::spinOnece();
		r.sleep();
	}
	
	return 0;
}

	
	

