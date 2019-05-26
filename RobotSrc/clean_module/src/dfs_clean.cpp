#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <tf/tf.h>
#include <tf/transform_listener.h>
#include <std_msgs/String.h>
#include <rosgraph_msgs/Log.h>
#include <ctime>
#include <cstdio>
#include <cstdlib>

#define db double

int mode; // 0:dfs, 1:zigzag
int level;
void args_init(int argc, char** argv){
    mode = 0, level = 0;
    ROS_INFO("[dfs_clean] argc: %d\n", argc);
    if (argc >= 2){
        mode = argv[1][0] - '0';
    }
    if (argc >= 3){
        level = argv[2][0] - '0';
    }
    ROS_INFO("[dfs_clean] init args:[mode:%d][level:%d]\n", mode, level);
}

// 0, 1, 2, 3
const float pi = 3.14;
int mv_x[] = {1, 0, -1, 0};
int mv_y[] = {0, 1, 0, -1};
int rev[] = {2, 3, 0, 1};
int zigzag_dir[] = {0, 2, 1, 3};
int ori_x, ori_y;
tf::Quaternion q[4]; 
bool stop, logErr;

// 统计信息
db S, L; int s_t; // 面积，距离，时间


void clean_init(){
	s_t = clock();
    q[0].setRPY(0, 0, 0);
    q[1].setRPY(0, 0, 0.5*pi);
    q[2].setRPY(0, 0, pi);
    q[3].setRPY(0, 0, 1.5*pi);
    ori_x = 0, ori_y = 0;
}

void update(){
    /*
	FILE *fp = fopen("clean.out", "w");
    printf("%lf %lf %d\n", S, L, (int)((clock() - s_t)) );
	fprintf(fp, "%lf %lf %d\n", S, L, (int)((clock() - s_t)) );
	*/
	ROS_INFO("cleaninfo %d %d %d\n", int(S), int(L), (int)((clock() - s_t)));
}

// 导航目标结构体
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;
bool move(int x, int y, MoveBaseClient &ac, tf::Quaternion &q, float du = 10.0){
    update();
    ros::Rate r(1);         //while函数的循环周期,这里为1Hz
    while(ros::ok() && stop)        //程序主循环
    {
        ros::spinOnce();        //短时间挂起,让回调函数得以调用
        r.sleep(); 
    }

    ROS_INFO("[dfs_clean] try to move to (%d,%d)\n", x, y);
    move_base_msgs::MoveBaseGoal newWayPoint;
    //q.setRPY(0,0,0);
    
    newWayPoint.target_pose.header.frame_id = "map";
    newWayPoint.target_pose.pose.position.x = x / 5.0 + 0.0;
    newWayPoint.target_pose.pose.position.y = y / 5.0 + 0.0;
    newWayPoint.target_pose.pose.orientation.x = q.x();
    newWayPoint.target_pose.pose.orientation.y = q.y();
    newWayPoint.target_pose.pose.orientation.z = q.z();
    newWayPoint.target_pose.pose.orientation.w = q.w();
    
    ac.sendGoal(newWayPoint);

    int time = 0;
    logErr = 0;
    while(ros::ok() && time < int(du)){
        ros::spinOnce();        //短时间挂起,让回调函数得以调用
        r.sleep();
        time ++;
		if (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) break;
		if (logErr) break;
    }

    if (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED){
        ROS_INFO("[dfs_clean] move to (%d,%d) succeeded\n", x, y);
		L += 1 / 5.0;
        return 1;
    }
    else {
        ac.cancelAllGoals();
        ROS_INFO("[dfs_clean] move to (%d,%d) failed\n", x, y);
		logErr = 0;
		return 0;
    }
}

bool G[200][200];
bool over(int x, int y){
    x += 100, y += 100;
    return x < 0 || y < 0 || x >= 200 || y >= 200 || G[x][y];
}
void setVis(int x, int y){
    x += 100, y += 100;
    G[x][y] = 1;
}

void dfs(int x, int y, MoveBaseClient &ac, int dir){
    for (int off = 0, _dir, _x, _y; off < 4; off ++){
        _dir = mode ? zigzag_dir[off] : (dir + off) % 4;
        _x = x + mv_x[_dir], _y = y + mv_y[_dir];
        
        if (!over(_x, _y)){

            setVis(_x, _y);
            if (move(_x, _y, ac, q[_dir])){
                for (int it = 0; it < level; it ++){// clean over
                    move(x, y, ac, q[rev[dir]]);
                    move(_x, _y, ac, q[_dir]);                
                }

				S += (1 / 5.0) * (1 / 5.0);

                dfs(_x, _y, ac, _dir);
            }

        }
    }

    ROS_INFO("[dfs_clean] try to go back...\n");
    move(x, y, ac, q[rev[dir]]);
}



void voiceCB(const std_msgs::String::ConstPtr &msg){
    ROS_INFO("[dfs_clean][test_voice]: %s\n", msg->data.c_str());
    int ret = 0;
    ret = msg->data.find("Stop") + msg->data.find("stop"); 

    if ( ret >= 0){
        ROS_INFO("[dfs_clean][test_voice]: get Stop: %d\n", ret);
        stop = true;
        return;
    }

    ret = msg->data.find("Go") + msg->data.find("go");
    if ( ret >= 0){
        ROS_INFO("[dfs_clean][test_voice]: get Go: %d\n", ret);
        stop = false;
        return;
    }
}

void rosoutCB(const rosgraph_msgs::Log::ConstPtr& msg){
	if (msg->level == rosgraph_msgs::Log::ERROR){
        
    	logErr = 1;
	}
	printf("[dfs_clean] FIND LOG ERR\n");
}

int main(int argc, char** argv){
    ros::init(argc, argv, "dfs_clean");
    args_init(argc, argv);

    ros::NodeHandle n;
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, voiceCB);
    ros::Subscriber rosout_sb = n.subscribe("/rosout", 0, rosoutCB);


    // 调用和主管监控导航功能的服务
    MoveBaseClient ac("move_base", true);

    // 休眠5s, 确认导航服务已经开启
    while (!ac.waitForServer(ros::Duration(5.0))){
        ROS_INFO("[dfs_clean] Waiting for the move_base action server to come up");
    }

    clean_init();

    setVis(ori_x, ori_y);
    if (move(ori_x, ori_y, ac, q[0], 1000.0)){
        ROS_INFO("[dfs_clean] init the position");
        dfs(ori_x, ori_y, ac, 0);
    }
    else{
        ROS_INFO("[dfs_clean] position initial failed");
    }

    return 0;
}
