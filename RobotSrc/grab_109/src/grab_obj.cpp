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


//using namespace std;
using namespace cv;

// 抓取参数调节（单位：米）
static float grab_y_offset = -0.05f;          //抓取前，对准物品，机器人的横向位移偏移量
static float grab_lift_offset = 0.25f;       //手臂抬起高度的补偿偏移量
static float grab_forward_offset = 0.05f;    //手臂抬起后，机器人向前抓取物品移动的位移偏移量
static float grab_gripper_value = 0.040;    //抓取物品时，手爪闭合后的手指间距


// 默认标签
static std::string default_label = "/home/robot/catkin_ws/src/team_109/grab_109/src/label/default.jpg";

#define STEP_WAIT           0
//改变检测策略，先检测标签，进而检测平面
#define STEP_FIND_OBJ       1
#define STEP_FIND_PLANE     2
#define STEP_PLANE_DIST     3
#define STEP_OBJ_DIST       4
#define STEP_HAND_UP        5
#define STEP_FORWARD        6
#define STEP_GRAB           7
#define STEP_OBJ_UP         8
#define STEP_BACKWARD       9
#define STEP_DONE           10
static int nStep = STEP_WAIT;

static std::string rgb_topic;
static std::string pc_topic;

static ros::Publisher rgb_pub;
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
//static stBoxMarker obj_to_track;
static bool find_obj = false; // 标记是否找到物体
static Mat templ; // 保存模板图像
static Mat objects; // 存储匹配的区域
static Rect matchObj; 
//static vector<Rect>::const_iterator obj_iter; // 迭代所有的标签
static int match_method = TM_CCOEFF_NORMED; // 匹配方法
//static int match_method = TM_CCORR_NORMED;

static float thresholdVal = 0.90; //设定匹配阈值


static cv::Point tl; // 矩形框左上角
static cv::Point br; // 矩形框右下角

static int nTimeDelayCounter = 0;
static int nFrameCount = 0;
static float fLastPlaneHeight = 0.5;
static float fPlaneHeight = 0;
static int nPlaneHeightCounter = 0;

static float fPlaneDist = 0;
static float fTargetPlaneDist = 0.7;    //与桌子之间的目标距离
static float fTargetGrabX = 0.8;        //抓取时目标物品的x坐标
static float fTargetGrabY = 0.0;        //抓取时目标物品的y坐标

static std::vector<stBoxMarker> vObj;        
static stBoxMarker boxLastObject;
static int nObjDetectCounter = 0;

static int nFrameNum = 0;

void 
ProcImageCB(const sensor_msgs::ImageConstPtr& msg)
{
	//ROS_INFO("[callbackRGB] nStep = %d", nStep);
	if(nStep != STEP_FIND_OBJ && nStep != STEP_FIND_PLANE) return;
	
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
	
	if(nFrameNum == 10){
	//保存图像
		std::ostringstream stringStream;
		stringStream << "/home/robot/catkin_ws/src/team_109/grab_109/src/pic/pic_" << nFrameNum;
		std::string frame_id = stringStream.str();
		imwrite(frame_id+".jpg", cv_ptr->image);
	}
	nFrameNum ++;

	//保存图像
	//std::ostringstream stringStream;
	//stringStream << "/home/robot/catkin_ws/src/team_109/grab_109/src/pic/pic_" << nFrameNum++;
	//std::string frame_id = stringStream.str();
	//imwrite(frame_id+".jpg", cv_ptr->image);
	//ROS_INFO("(w, h)=(%d, %d)", frame_gray.cols, frame_gray.rows);
	
	/*
	// 脂肪图均衡化
	cv::Mat src, dst, yuv;
	cv::Mat tsrc, tdst, tyuv;

	std::vector<cv::Mat> channels, tchannels;
	
	cv::cvtColor(cv_ptr->image, yuv, COLOR_BGR2YUV);
	cv::cvtColor(templ, tyuv, COLOR_BGR2YUV);
	
	cv::split(yuv, channels);
	
	cv::equalizeHist(channels[0], channels[0]); // 均衡化
	cv::merge(channels, dst); // 合并

	cv::split(tyuv, tchannels);
	cv::equalizeHist(tchannels[0], tchannels[0]);
	cv::merge(tchannels, tdst);

	*/

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

		//ROS_INFO("[callbackRGB] find object (x,y,width,height)=(%d,%d,%d,%d) maxVal=%.2f", tl.x,tl.y,templ.cols,templ.rows,val);



		
	}else{
		find_obj = false;
		ROS_INFO("[ProcImageCB FIND_OBJ] no object");
	}
	
	rgb_pub.publish(cv_ptr->toImageMsg());
	
	
}


void 
ProcCloudCB(const sensor_msgs::PointCloud2 &input)
{

	ROS_INFO("[callbackPointCloud] nStep = %d", nStep);

	sensor_msgs::PointCloud2 pc_footprint;
    tf_listener->waitForTransform("/base_footprint", input.header.frame_id, input.header.stamp, ros::Duration(5.0));  //return value always  false!
    pcl_ros::transformPointCloud("/base_footprint", input, pc_footprint, *tf_listener);

    //source cloud
    pcl::PointCloud<pcl::PointXYZRGB> cloud_src;
    pcl::fromROSMsg(pc_footprint , cloud_src);
    //ROS_INFO("cloud_src size = %d",cloud_src.size()); 
    //pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_source_ptr;
	////////////////////////////////////
	
//ROS_INFO("(input.width, input.height)=(%d,%d)",input.width, input.height);
	//ROS_INFO("[callbackPointCloud] (x1,y1,x2,y2)=(%d,%d,%d,%d)", tl.x,tl.y,br.x,br.y);

	//数据处理
	if(nStep == STEP_FIND_PLANE || nStep==STEP_FIND_OBJ){
		
		if(find_obj){ // 在RGB图中寻找物体
			ROS_INFO("[callbackPointCloud] find object");

			nObjDetectCounter ++; // 累积检测次数
			
			//RemoveBoxes();
			
			float plane_height = 0.0;
			// 确定平面的高度，假设物体放在平面上
			for(int k=tl.x; k<= br.x; k++){
				if(isnan((cloud_src.points[br.y * input.width + k]).z)) continue;
				
				plane_height += (cloud_src.points[br.y * input.width + k]).z;
			}
			plane_height /= br.x - tl.x;
			fPlaneHeight = plane_height;
			
			ROS_INFO("[callbackPointCloud] planeHeight=%.2f", fPlaneHeight);
			
			int xhalf = (int)((br.x - tl.x)/4);
			int yhalf = (int)((br.y - tl.y)/4);
			
			bool firstP = true;
			for(int y = tl.y + yhalf; y<br.y - yhalf; y++){
				for(int x=tl.x + xhalf; x<br.x - xhalf; x++){
					int index_pc = y * input.width + x;
			// 确定标签的空间位置
					pcl::PointXYZRGB p = cloud_src.points[index_pc];
					//pcl::PointXYZRGB pbr = cloud_src.points[index_br];
					if(isnan(p.z)) continue;
					if(firstP){
						firstP = false;
						boxMarker.xMin = boxMarker.xMax = p.x;
						boxMarker.yMin = boxMarker.yMax = p.y;
						boxMarker.zMin = boxMarker.zMax = p.z;
					}
			
					if(p.x < boxMarker.xMin) { boxMarker.xMin = p.x;}
					if(p.x > boxMarker.xMax) { boxMarker.xMax = p.x;}
					if(p.y < boxMarker.yMin) { boxMarker.yMin = p.y;}
					if(p.y > boxMarker.yMax) { boxMarker.yMax = p.y;}
					if(p.z < boxMarker.zMin) { boxMarker.zMin = p.z;}
					if(p.z > boxMarker.zMax) { boxMarker.zMax = p.z;}
				}
			}
			
			//ROS_INFO("[callbackPointCloud] boxMarker=(%.2f, %.2f, %.2f, %.2f, %.2f, %.2f)", \
boxMarker.xMin, boxMarker.xMax, boxMarker.yMin, boxMarker.yMax, boxMarker.zMin, boxMarker.zMax);

			
			if(boxMarker.xMin < 1.5 && boxMarker.yMin > -0.5 && boxMarker.yMax < 0.5)   //物品所处的空间限定
            {
				RemoveBoxes();
                DrawBox(boxMarker.xMin, boxMarker.xMax, boxMarker.yMin, boxMarker.yMax, boxMarker.zMin, boxMarker.zMax, 0, 1, 0);
				DrawText("object x",0.08, boxMarker.xMax,(boxMarker.yMin+boxMarker.yMax)/2,boxMarker.zMax + 0.04, 1,0,1);
			}
			
			boxLastObject = boxMarker;
			
		}else{
			nObjDetectCounter= 0;
			ROS_INFO("[ProcCloudCB FIND_OBJ] no object");
		}
		
	}
	
	
	
	
	/////////////////////////////////////////////////////////////
	/*二、有限状态机 *************************************************************************************************/
    //1、统计识别次数，确认平面
	if(nStep == STEP_FIND_PLANE){
		mani_ctrl_msg.position[0] =0;
		mani_ctrl_msg.position[1] = 0.16;
		VelCmd(0, 0, 0);
		ctrl_msg.data = "pose_diff reset";
		ctrl_pub.publish(ctrl_msg);
		//if(fabs(fPlaneHeight - fLastPlaneHeight) < 0.05){
		if(fPlaneHeight >= 0.5  && fPlaneHeight<=0.7){		
			nPlaneHeightCounter ++;
		}else{
			nPlaneHeightCounter = 0;
		}
		ROS_WARN("[FIND_PLANE] z= %.2f  counter= %d" ,fPlaneHeight, nPlaneHeightCounter);
        
		if(nPlaneHeightCounter > 5)
        {
            nPlaneHeightCounter = 0;
            nTimeDelayCounter = 0;
            nStep = STEP_PLANE_DIST;
            //fMoveTargetX = boxPlane.xMin - 0.7;
            //fMoveTargetY = 0;
        }
        result_msg.data = "find plane";
        result_pub.publish(result_msg);
		
		
	}
	
	//2、前后运动：控制到平面的距离
	if(nStep == STEP_PLANE_DIST && nPlaneDistMode == 1){
		float fMinDist = 100;
		for(int i=0; i<cloud_src.points.size(); i++){
			pcl::PointXYZRGB p = cloud_src.points[i];

			if(
                p.y > -0.2 && p.y < 0.2 && !isnan(p.z) &&
                p.z > fPlaneHeight-0.15 && p.z < fPlaneHeight+0.15 && 
                p.x < fMinDist)
            {
                fMinDist = p.x;
            }
        }
		fPlaneDist = fMinDist;
		ROS_WARN("[PLANE_DIST] dist= %.2f", fPlaneDist);
		float diff = fPlaneDist - fTargetPlaneDist;
		if(fabs(diff) < 0.02){
			//nObjDetectCounter = 0;
			nStep = STEP_FIND_OBJ;
			VelCmd(0, 0, 0);
		}else{
			if(diff>0) 
{
//ROS_INFO("距离太远");
VelCmd(0.05, 0, 0); //距离还太远，前进
}			
else{
//ROS_INFO("距离太近");
 VelCmd(-0.05, 0, 0); //距离太近，后退
		
}
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
		//mani_ctrl_msg.position[0] = (boxLastObject.zMax + boxLastObject.zMax)/2 + grab_lift_offset;
            mani_ctrl_msg.position[1] = 0.16;
            mani_ctrl_pub.publish(mani_ctrl_msg);
            ROS_WARN("[MANI_CTRL] lift= %.2f  gripper= %.2f " ,mani_ctrl_msg.position[0], mani_ctrl_msg.position[1]);
            result_msg.data = "hand up";
            result_pub.publish(result_msg);
        }
		nTimeDelayCounter ++;
		VelCmd(0,0,0);
		if(nTimeDelayCounter > 30){
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
        if(nTimeDelayCounter > 15)
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
	if(nTimeDelayCounter==0){
        	result_msg.data = "done";
        	result_pub.publish(result_msg);
	}

	mani_ctrl_msg.position[1] = 1.0;
	mani_ctrl_pub.publish(mani_ctrl_msg);

	nTimeDelayCounter ++;
	if(nTimeDelayCounter > 10){
		nTimeDelayCounter = 0;
		//nStep = STEP_FIND_PLANE;
		
		mani_ctrl_msg.position[0] = 0.0;
		mani_ctrl_pub.publish(mani_ctrl_msg);

		
	}

    }
}

void 
PoseDiffCallback(const geometry_msgs::Pose2D::ConstPtr& msg)
{
    //ROS_INFO("[pose info] (%.2f, %.2f)", msg->x, msg->y);

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

	if(vx >= 0.08) vx = 0.08;
	if(vx <= -0.08) vx = -0.08;
	if(vy >= 0.08) vy = 0.08;
	if(vx <= -0.08) vy = -0.08;

	//vx = min(vx, 0.08);
	//vy = min(vy, 0.08);

	VelCmd(vx,vy,0);
	ROS_INFO("[MOVE] T(%.2f %.2f)  od(%.2f , %.2f) v(%.2f,%.2f)" ,fMoveTargetX, fMoveTargetY, pose_diff.x ,pose_diff.y,vx,vy);
	
	//ROS_INFO("[main] nStep = %d", nStep);

	if(fabs(vx) <= 0.01 && fabs(vy) <= 0.01)
	{
		VelCmd(0,0,0);
		ctrl_msg.data = ctl_msg;
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
	ros::init(argc, argv, "grab_obj");
	ROS_INFO("my_grab_object");
	tf_listener = new tf::TransformListener();
	
	ros::NodeHandle nh_param("~");
	//nh_param.param<std::string>("rgb_topic", rgb_topic, "/kinect2/qhd/image_color");
	//nh_param.param<std::string>("topic", pc_topic, "/kinect2/qhd/points");
	
	rgb_topic = "/kinect2/qhd/image_color";
	pc_topic = "/kinect2/qhd/points";

	std::string templpath = "";
	//ros::param::get("templpath", templpath);
	nh_param.param<std::string>("templpath", templpath, "");

	printf("[grab_obj =====] templpath:%s\n", templpath.c_str());

	//if(argc == 2){
		//templpath = argv[1];
					
	
	//}
	//nh_param.param<std::string>("label_109", templpath, default_label);

	//std::string templpath = "/home/robot/catkin_ws/src/team_109/grab_109/src/label/pic_10.jpg";//label_qhd_color.jpg";
	templ = imread(templpath);
	if(templ.data == NULL){
		//ROS_INFO("(w, h)=(%d, %d)", templ.cols, templ.rows);
		ROS_ERROR("the file path %s does not exist", templpath.c_str());
		return -1;
	}
	
	
	ros::NodeHandle nh;
	
	ros::Subscriber rgb_sub = nh.subscribe(rgb_topic, 1, ProcImageCB);
	ros::Subscriber pc_sub = nh.subscribe(pc_topic, 1, ProcCloudCB);
	
	rgb_pub = nh.advertise<sensor_msgs::Image>("/object/image", 1);
	pc_pub = nh.advertise<sensor_msgs::PointCloud2>("obj_pointcloud",1);
    marker_pub = nh.advertise<visualization_msgs::Marker>("obj_marker", 2);
	
//result_pub = nh.n.advertise<std_msgs::String>("/wpb_home/grab_result", 30);
	

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

/*
	nh_param.getParam("grab/grab_y_offset", grab_y_offset);
    nh_param.getParam("grab/grab_lift_offset", grab_lift_offset);
    nh_param.getParam("grab/grab_forward_offset", grab_forward_offset);
*/
    //nh_param.getParam("grab/grab_gripper_value", grab_gripper_value);


    bool bActive = true;
    nh_param.param<bool>("start", bActive, true);
	
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
		
		ros::spinOnce();
		r.sleep();
		
	}
	
	delete tf_listener;
	return 0;
	
}
