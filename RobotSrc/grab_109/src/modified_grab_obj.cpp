#include <ros/ros.h>
#include <pcl_ros/point_cloud.h>
#include <pcl/point_types.h>
#include <boost/foreach.hpp>
#include <pcl/io/pcd_io.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/surface/convex_hull.h>
#include <pcl/segmentation/extract_polygonal_prism_data.h> 
#include <pcl/visualization/cloud_viewer.h>
#include <pcl/segmentation/extract_clusters.h>
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


// 抓取参数调节（单位：米）
static float grab_y_offset = 0.0f;          //抓取前，对准物品，机器人的横向位移偏移量
static float grab_lift_offset = 0.0f;       //手臂抬起高度的补偿偏移量
static float grab_forward_offset = 0.0f;    //手臂抬起后，机器人向前抓取物品移动的位移偏移量
static float grab_gripper_value = 0.032;    //抓取物品时，手爪闭合后的手指间距

#define STEP_WAIT           0
//取消了平面检测
#define STEP_FIND_PLANE     1
#define STEP_PLANE_DIST     2
#define STEP_FIND_OBJ       3
#define STEP_OBJ_DIST       4
#define STEP_HAND_UP        5
#define STEP_FORWARD        6
#define STEP_GRAB           7
#define STEP_OBJ_UP         8
#define STEP_BACKWARD       9
#define STEP_DONE           10
static int nStep = STEP_WAIT;


static std::string pc_topic;
static ros::Publisher pc_pub;
static ros::Publisher marker_pub;
static ros::Publisher vel_pub;
static ros::Publisher mani_ctrl_pub;
static sensor_msgs::JointState mani_ctrl_msg;
static ros::Publisher result_pub;
static tf::TransformListener *tf_listener; 

void DrawBox(float inMinX, float inMaxX, float inMinY, float inMaxY, float inMinZ, float inMaxZ, float inR, float inG, float inB);
void DrawText(std::string inText, float inScale, float inX, float inY, float inZ, float inR, float inG, float inB);
void DrawPath(float inX, float inY, float inZ);
void RemoveBoxes();
void VelCmd(float inVx , float inVy, float inTz);


static visualization_msgs::Marker line_box;
static visualization_msgs::Marker line_follow;
static visualization_msgs::Marker text_marker;

typedef pcl::PointCloud<pcl::PointXYZRGB> PointCloud;
//ros::Publisher segmented_plane;
//ros::Publisher segmented_objects;
ros::Publisher masking;
ros::Publisher color;
static std_msgs::String result_msg;

static ros::Publisher ctrl_pub;
static std_msgs::String ctrl_msg;
static geometry_msgs::Pose2D pose_diff;

static float fObjGrabX = 0;
static float fObjGrabY = 0;
static float fObjGrabZ = 0;
static float fMoveTargetX = 0;
static float fMoveTargetY = 0;
static bool nPlaneDistMode = 1;
cv::Mat rgb_image;

typedef struct stBoxMarker
{
    float xMax;
    float xMin;
    float yMax;
    float yMin;
    float zMax;
    float zMin;
}stBoxMarker;

static stBoxMarker boxMarker;
static stBoxMarker boxPlane;
static stBoxMarker obj_to_track;

static int nTimeDelayCounter = 0;
static int nFrameCount = 0;
static float fLastPlaneHeight = 0.5;
static float fPlaneHeight = 0;
static int nPlaneHeightCounter = 0;

static float fPlaneDist = 0;
static float fTargetPlaneDist = 0.6;    //与桌子之间的目标距离
static float fTargetGrabX = 0.8;        //抓取时目标物品的x坐标
static float fTargetGrabY = 0.0;        //抓取时目标物品的y坐标

static std::vector<stBoxMarker> vObj;        
static stBoxMarker boxLastObject;
static int nObjDetectCounter = 0;

void 
ProcCloudCB(const sensor_msgs::PointCloud2 &input)
{
	sensor_msgs::PointCloud2 pc_footprint;
    tf_listener->waitForTransform("/base_footprint", input.header.frame_id, input.header.stamp, ros::Duration(5.0));  //return value always  false!
    pcl_ros::transformPointCloud("/base_footprint", input, pc_footprint, *tf_listener);

    //source cloud
    pcl::PointCloud<pcl::PointXYZRGB> cloud_src;
    pcl::fromROSMsg(pc_footprint , cloud_src);
    //ROS_INFO("cloud_src size = %d",cloud_src.size()); 
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_source_ptr;
	////////////////////////////////////
	//新构建一个点云，只取一部分
	PointCloud new_point_cloud;
	
	cv::Rect objRect(0, 0, 0, 0);
	//数据处理
	//需要分割平面，防止物体过于靠近平面内部
	if(nStep == STEP_FIND_PLANE || nStep==STEP_FIND_OBJ){
		
		//callbackRGB(); //?
		PointCloud::Ptr plane(new PointCloud);
		PointCloud::Ptr convexHull(new PointCloud);
		
		pcl::ModelCoefficients::Ptr coefficients(new pcl::ModelCoefficients);
        pcl::SACSegmentation<pcl::PointXYZRGB> segmentation;
        segmentation.setInputCloud(cloud_source_ptr);
        segmentation.setModelType(pcl::SACMODEL_PLANE);
        segmentation.setMethodType(pcl::SAC_RANSAC);
        segmentation.setDistanceThreshold(0.005);
        segmentation.setOptimizeCoefficients(true);
        pcl::PointIndices::Ptr planeIndices(new pcl::PointIndices);
        segmentation.segment(*planeIndices, *coefficients);
        //ROS_INFO_STREAM("Num_Of_Planes = " << planeIndices->indices.size());
        pcl::ExtractIndices<pcl::PointXYZRGB> extract;
		
		
		RemoveBoxes();
        pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_f (new pcl::PointCloud<pcl::PointXYZRGB>);
        int i = 0, nr_points = (int) cloud_source_ptr->points.size ();
        // While 30% of the original cloud is still there
        while (cloud_source_ptr->points.size () > 0.03 * nr_points)
        {
            // Segment the largest planar component from the remaining cloud
            segmentation.setInputCloud (cloud_source_ptr);
            segmentation.segment (*planeIndices, *coefficients);
            if (planeIndices->indices.size () == 0)
            {
                ROS_WARN("Could not estimate a planar model for the given dataset.");
                break;
            }

            // Extract the planeIndices
            extract.setInputCloud (cloud_source_ptr);
            extract.setIndices (planeIndices);
            extract.setNegative (false);
            extract.filter (*plane);
            float plane_height = 0;
            for(int k=0; k<plane->width * plane->height ;k ++ )
            {
                plane_height += plane->points[k].z;
            }
            plane_height /= plane->width * plane->height;
            //ROS_WARN("%d - plane: %d points. height =%.2f" ,i, plane->width * plane->height,plane_height);
            fPlaneHeight = plane_height;
           
            bool bFirstPoint = true;
            for (int j = 0; j < plane->points.size(); j++) 
            {
                pcl::PointXYZRGB p = plane->points[j];
                if(bFirstPoint == true)
                {
                    boxPlane.xMax = boxPlane.xMin = p.x;
                    boxPlane.yMax = boxPlane.yMin = p.y;
                    boxPlane.zMax = boxPlane.zMin = p.z;
                    bFirstPoint = false;
                }

                if(p.x < boxPlane.xMin) { boxPlane.xMin = p.x;}
                if(p.x > boxPlane.xMax) { boxPlane.xMax = p.x;}
                if(p.y < boxPlane.yMin) { boxPlane.yMin = p.y;}
                if(p.y > boxPlane.yMax) { boxPlane.yMax = p.y;}
                if(p.z < boxPlane.zMin) { boxPlane.zMin = p.z;}
                if(p.z > boxPlane.zMax) { boxPlane.zMax = p.z;}
            } 
            //////////////////////////////////
            //测试：将平面用框框框起来
            DrawBox(boxPlane.xMin, boxPlane.xMax, boxPlane.yMin, boxPlane.yMax, boxPlane.zMin, boxPlane.zMax, 1, 0, 1);
            //ROS_WARN("[FIND_PLANE] x= (%.2f , %.2f) y=(%.2f , %.2f) z=(%.2f , %.2f)" ,boxPlane.xMin,boxPlane.xMax,boxPlane.yMin,boxPlane.yMax,boxPlane.zMin,boxPlane.zMax);
            ///////////////////////////////////
            if(plane_height > 0.5 && plane_height < 0.85) 
            break;

            // Create the filtering object
            extract.setNegative (true);
            extract.filter (*cloud_f);
            cloud_source_ptr.swap (cloud_f);
            i++;
        }
		
		if(planeIndices->indices.size() == 0)
			ROS_WARN("could not find a plane");
		else
			ProcessRGB(objRect); //检测桌面上的标签
	}
	
	
	
	
	/////////////////////////////////////////////////////////////
	/*二、有限状态机 *************************************************************************************************/
    //1、左右运动：统计识别次数，确认平面
	if(nStep == STEP_FIND_PLANE){
		mani_ctrl_msg.position[0] =0;
		mani_ctrl_msg.position[1] = 0.16;
		VelCmd(0, 0, 0);
		ctrl_msg.data = "pose_diff reset";
		ctrl_pub.publish(ctrl_msg);
		if(fabs(fPlaneHeight - fLastPlaneHeight) < 0.05){
			nPlaneHeightCounter ++;
		}else{
			nPlaneHeightCounter = 0;
		}
		ROS_WARN("[FIND_PLANE] z= %.2f xm= %.2f y=(%.2f , %.2f) counter= %d" ,fPlaneHeight,boxPlane.xMin,boxPlane.yMin,boxPlane.yMax,nPlaneHeightCounter);
        if(nPlaneHeightCounter > 10)
        {
            nPlaneHeightCounter = 0;
            nTimeDelayCounter = 0;
            nStep = STEP_PLANE_DIST;
            fMoveTargetX = boxPlane.xMin - 0.7;
            fMoveTargetY = 0;
        }
        result_msg.data = "find plane";
        result_pub.publish(result_msg);
	}
	
	//2、前后运动：控制到平面的距离
	if(nStep == STEP_PLANE_DIST && nPlaneDistMode == 1){
		float fMinDist = 100;
		for(int i=0; i<cloud_source_ptr->points.size(); i++){
			pcl::PointXYZRGB p = cloud_source_ptr->points[i];
			if(
                p.y > -0.2 && p.y < 0.2 && 
                p.z > fPlaneHeight-0.05 && p.z < fPlaneHeight+0.05 && 
                p.x < fMinDist)
            {
                fMinDist = p.x;
            }
        }
		fPlaneDist = fMinDist;
		ROS_WARN("[PLANE_DIST] dist= %.2f", fPlaneDist);
		float diff = fPlaneDist - fTargetPlaneDist;
		if(fabs(diff) < 0.02){
			nObjDetectCounter = 0;
			nStep = STEP_FIND_OBJ;
			VelCmd(0, 0, 0);
		}else{
			if(diff>0) VelCmd(0.1, 0, 0); //距离还太远，前进
			else VelCmd(-0.1, 0, 0); //距离太近，后退
		}
		result_msg.data = "plane dist";
		result_pub.publish(result_msg);
	}
	//3、左右移动：检测物品，挑选出准备抓取的目标物品
	if(nStep == STEP_FIND_OBJ){
		VelCmd(0, 0, 0);
		ctrl_msg.data = "pose_diff reset";
		ctrl_pub.publish(ctrl_msg);
		if(nObjDetectCounter > 3){
			nObjDetectCounter = 0;
			//目标物品的坐标
			fObjGrabX = boxLastObject.xMin;
			fObjGrabY = (boxLastObject.yMin+boxLastObject.yMax)/2;
			fObjGrabZ = boxLastObject.zMax;
			
			//判断物品和桌子边缘的距离，如果放的太靠里，放弃抓取
			if(fabs(fObjGrabX - fTargetPlaneDist) > 0.4){
				ROS_WARN("[OBJ_TO_GRAB] object is hard to reach !!!");
				nStep = STEP_DONE;
			}else{
				fMoveTargetX = 0.0f;
				fMoveTargetY = fObjGrabY - fTargetGrabY + grab_y_offset;
				nStep = STEP_OBJ_DIST;
			}
		}
		result_msg.data = "find objects";
		result_pub.publish(result_msg);
	}
	
	//5、抬起手臂
	if(nStep == STEP_HAND_UP){
		if(nTimeDelayCounter == 0){
			mani_ctrl_msg.position[0] = fPlaneHeight + grab_lift_offset;
            mani_ctrl_msg.position[1] = 0.16;
            mani_ctrl_pub.publish(mani_ctrl_msg);
            ROS_WARN("[MANI_CTRL] lift= %.2f  gripper= %.2f " ,mani_ctrl_msg.position[0], mani_ctrl_msg.position[1]);
            result_msg.data = "hand up";
            result_pub.publish(result_msg);
        }
		nTimeDelayCounter ++;
		VelCmd(0,0,0);
		if(nTimeDelayCounter > 50){
			fMoveTargetX = fObjGrabX - 0.55 + grab_forward_offset;
			fMoveTargetY = 0;
			ROS_WARN("[STEP_FORWARD] x = %.2f y=%.2f", fMoveTargetX, fMoveTargetY);
			nTimeDelayCounter = 0;
			nStep = STEP_FORWARD;
		}
	}
	
	//7、抓取物品
	if(nStep == STEP_GRAB){
		if(nTimeDelayCounter == 0){
			result_msg.data = "grab";
            result_pub.publish(result_msg);
        }
        mani_ctrl_msg.position[1] = grab_gripper_value;      //抓取物品手爪闭合宽度
        mani_ctrl_pub.publish(mani_ctrl_msg);
        //ROS_WARN("[MANI_CTRL] lift= %.2f  gripper= %.2f " ,mani_ctrl_msg.position[0], mani_ctrl_msg.position[1]);

        nTimeDelayCounter++;
        VelCmd(0,0,0);
        if(nTimeDelayCounter > 30)
        {
            nTimeDelayCounter = 0;
            nStep = STEP_OBJ_UP;
        }
    }
		
	//8、拿起物品
	if(nStep == STEP_OBJ_UP){
		if(nTimeDelayCounter == 0){
			mani_ctrl_msg.position[0] += 0.03;
			mani_ctrl_pub.publish(mani_ctrl_msg);
			result_msg.data = "object up";
			result_pub.publish(result_msg);
		}
		nTimeDelayCounter ++;
		VelCmd(0,0,0);
		
		if(nTimeDelayCounter > 10)
        {
            fMoveTargetX = -(fTargetGrabX-0.4);
            fMoveTargetY = 0;
            //ROS_WARN("[STEP_BACKWARD] x= %.2f y= %.2f " ,fMoveTargetX, fMoveTargetY);

            nTimeDelayCounter = 0;
            nStep = STEP_BACKWARD;
        }
	}
	//10、抓取任务完毕
    if(nStep == STEP_DONE)
    {
        result_msg.data = "done";
        result_pub.publish(result_msg);
    }
}

void 
PoseDiffCallback(const geometry_msgs::Pose2D::ConstPtr& msg)
{
    pose_diff.x = msg->x;
    pose_diff.y = msg->y;
    pose_diff.theta = msg->theta;
}



void 
DrawBox(float inMinX, float inMaxX, float inMinY, float inMaxY, float inMinZ, float inMaxZ, float inR, float inG, float inB)
{
    line_box.header.frame_id = "base_footprint";
    line_box.ns = "line_box";
    line_box.action = visualization_msgs::Marker::ADD;
    line_box.id = 0;
    line_box.type = visualization_msgs::Marker::LINE_LIST;
    line_box.scale.x = 0.005;
    line_box.color.r = inR;
    line_box.color.g = inG;
    line_box.color.b = inB;
    line_box.color.a = 1.0;

    geometry_msgs::Point p;
    p.z = inMinZ;
    p.x = inMinX; p.y = inMinY; line_box.points.push_back(p);
    p.x = inMinX; p.y = inMaxY; line_box.points.push_back(p);

    p.x = inMinX; p.y = inMaxY; line_box.points.push_back(p);
    p.x = inMaxX; p.y = inMaxY; line_box.points.push_back(p);

    p.x = inMaxX; p.y = inMaxY; line_box.points.push_back(p);
    p.x = inMaxX; p.y = inMinY; line_box.points.push_back(p);

    p.x = inMaxX; p.y = inMinY; line_box.points.push_back(p);
    p.x = inMinX; p.y = inMinY; line_box.points.push_back(p);

    p.z = inMaxZ;
    p.x = inMinX; p.y = inMinY; line_box.points.push_back(p);
    p.x = inMinX; p.y = inMaxY; line_box.points.push_back(p);

    p.x = inMinX; p.y = inMaxY; line_box.points.push_back(p);
    p.x = inMaxX; p.y = inMaxY; line_box.points.push_back(p);

    p.x = inMaxX; p.y = inMaxY; line_box.points.push_back(p);
    p.x = inMaxX; p.y = inMinY; line_box.points.push_back(p);

    p.x = inMaxX; p.y = inMinY; line_box.points.push_back(p);
    p.x = inMinX; p.y = inMinY; line_box.points.push_back(p);

    p.x = inMinX; p.y = inMinY; p.z = inMinZ; line_box.points.push_back(p);
    p.x = inMinX; p.y = inMinY; p.z = inMaxZ; line_box.points.push_back(p);

    p.x = inMinX; p.y = inMaxY; p.z = inMinZ; line_box.points.push_back(p);
    p.x = inMinX; p.y = inMaxY; p.z = inMaxZ; line_box.points.push_back(p);

    p.x = inMaxX; p.y = inMaxY; p.z = inMinZ; line_box.points.push_back(p);
    p.x = inMaxX; p.y = inMaxY; p.z = inMaxZ; line_box.points.push_back(p);

    p.x = inMaxX; p.y = inMinY; p.z = inMinZ; line_box.points.push_back(p);
    p.x = inMaxX; p.y = inMinY; p.z = inMaxZ; line_box.points.push_back(p);
    marker_pub.publish(line_box);
}

static int nTextNum = 2;
void 
DrawText(std::string inText, float inScale, float inX, float inY, float inZ, float inR, float inG, float inB)
{
    text_marker.header.frame_id = "base_footprint";
    text_marker.ns = "line_obj";
    text_marker.action = visualization_msgs::Marker::ADD;
    text_marker.id = nTextNum;
    nTextNum ++;
    text_marker.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
    text_marker.scale.z = inScale;
    text_marker.color.r = inR;
    text_marker.color.g = inG;
    text_marker.color.b = inB;
    text_marker.color.a = 1.0;

    text_marker.pose.position.x = inX;
    text_marker.pose.position.y = inY;
    text_marker.pose.position.z = inZ;
    
    text_marker.pose.orientation=tf::createQuaternionMsgFromYaw(1.0);

    text_marker.text = inText;

    marker_pub.publish(text_marker);
}

void 
RemoveBoxes()
{
    line_box.action = 3;
    line_box.points.clear();
    marker_pub.publish(line_box);
    line_follow.action = 3;
    line_follow.points.clear();
    marker_pub.publish(line_follow);
    text_marker.action = 3;
    marker_pub.publish(text_marker);
}

void 
VelCmd(float inVx , float inVy, float inTz)
{
    geometry_msgs::Twist vel_cmd;
    vel_cmd.linear.x = inVx;
    vel_cmd.linear.y = inVy;
    vel_cmd.angular.z = inTz;
    vel_pub.publish(vel_cmd);
}

void 
BehaviorCB(const std_msgs::String::ConstPtr &msg)
{
    int nFindIndex = 0;
    nFindIndex = msg->data.find("grab start");
    if( nFindIndex >= 0 )
    {
        VelCmd(0,0,0);
        nStep = STEP_FIND_PLANE;
        ROS_WARN("[grab_start] ");
    }

    nFindIndex = msg->data.find("grab stop");
    if( nFindIndex >= 0 )
    {
        ROS_WARN("[grab_stop] ");
        nStep = STEP_WAIT;
        geometry_msgs::Twist vel_cmd;
        vel_cmd.linear.x = 0;
        vel_cmd.linear.y = 0;
        vel_cmd.linear.z = 0;
        vel_cmd.angular.x = 0;
        vel_cmd.angular.y = 0;
        vel_cmd.angular.z = 0;
        vel_pub.publish(vel_cmd);
    }

}

void 
move(std::string rlt_msg, std::string ctl_msg)
{
	float vx,vy;
	vx = (fMoveTargetX - pose_diff.x)/2;
	vy = (fMoveTargetY - pose_diff.y)/2;

	VelCmd(vx,vy,0);
	//ROS_INFO("[MOVE] T(%.2f %.2f)  od(%.2f , %.2f) v(%.2f,%.2f)" ,fMoveTargetX, fMoveTargetY, pose_diff.x ,pose_diff.y,vx,vy);

	if(fabs(vx) < 0.01 && fabs(vy) < 0.01)
	{
		VelCmd(0,0,0);
		ctrl_msg.data = ctr_msg;
		ctrl_pub.publish(ctrl_msg);
		if(nStep == STEP_PLANE_DIST && nPlaneDistMode == 2)
			nStep = STEP_FIND_OBJ;
		else if(nStep == STEP_OBJ_DIST)
			nStep = STEP_HAND_UP;
		else if(nStep == STEP_FORWARD)
			nStep = STEP_GRAB;
		else if(nStep == STEP_BACKWARD)
			nStep = STEP_DONE;
		else 
			ROS_ERROR("[MOVE] state of step is exception");
		
	}

	result_msg.data = rlt_msg;
	result_pub.publish(result_msg);
	
}


int 
main(int argc, char** argv)
{
	ros::init(argc, argv, "modified_grab_obj");
	ROS_INFO("my_grab_object");
	tf_listener = new tf::TransformListener();
	
	ros::NodeHandle nh_param("~");
	nh_param.param<std::string>("topic", pc_topic, "/kinect2/sd/points");
	
	ros::NodeHandle nh;
	ros::Subscriber pc_sub = nh.subscribe(pc_topic, 1, ProcCloudCB);
	
	pc_pub = nh.advertise<sensor_msgs::PointCloud2>("obj_pointcloud",1);
    marker_pub = nh.advertise<visualization_msgs::Marker>("obj_marker", 10);
	
	vel_pub = nh.advertise<geometry_msgs::Twist>("/cmd_vel", 30);
    mani_ctrl_pub = nh.advertise<sensor_msgs::JointState>("/wpb_home/mani_ctrl", 30);
    result_pub = nh.advertise<std_msgs::String>("/wpb_home/grab_result", 30);

    ros::Subscriber sub_sr = nh.subscribe("/wpb_home/behaviors", 30, BehaviorCB);
    ctrl_pub = nh.advertise<std_msgs::String>("/wpb_home/ctrl", 30);
    ros::Subscriber pose_diff_sub = nh.subscribe("/wpb_home/pose_diff", 1, PoseDiffCallback);

	mani_ctrl_msg.name.resize(2);
    mani_ctrl_msg.position.resize(2);
    mani_ctrl_msg.velocity.resize(2);
    mani_ctrl_msg.name[0] = "lift";
    mani_ctrl_msg.name[1] = "gripper";
    mani_ctrl_msg.position[0] = 0;
    mani_ctrl_msg.velocity[0] = 0.5;     //升降速度(单位:米/秒)
    mani_ctrl_msg.position[1] = 0.16;
    mani_ctrl_msg.velocity[1] = 5;       //手爪开合角速度(单位:度/秒)

	nh_param.getParam("grab/grab_y_offset", grab_y_offset);
    nh_param.getParam("grab/grab_lift_offset", grab_lift_offset);
    nh_param.getParam("grab/grab_forward_offset", grab_forward_offset);
    nh_param.getParam("grab/grab_gripper_value", grab_gripper_value);

    bool bActive = false;
    nh_param.param<bool>("start", bActive, false);
	
	if(bActive == true)
    {
        VelCmd(0,0,0);
        nStep = STEP_FIND_PLANE;
    }

    ros::Rate r(30);
	while(nh.ok()){
		//2、前后运动控制到平面的距离
		if(nStep == STEP_PLANE_DIST && nPlaneDistMode == 2){
			move("plane dist", "pose_diff reset");
		}
		
		
		//4、左右平移对准目标物品
		if(nStep == STEP_OBJ_DIST){
			move("object x", "pose_diff reset");
        }
		
		//6、前进靠近物品
		if(nStep == STEP_FORWARD){
			move("forward", "pose_diff reset");
        }
		
		//9、带着物品后退 
		if(nStep == STEP_BACKWARD){
			move("backward", "pose_diff reset");
		}
		
		ros::spinOnec();
		r.sleep();
		
	}
	
	delete tf_listener;
	return 0;
	
}
