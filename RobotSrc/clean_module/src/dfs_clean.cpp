#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <tf/tf.h>
#include <tf/transform_listener.h>
#include <std_msgs/String.h>

int mode; // 0:dfs, 1:zigzag
int level;
void args_init(int argc, char** argv){
    mode = 0, level = 0;
    printf("%d\n", argc);
    if (argc >= 2){
        mode = argv[1][0] - '0';
    }
    if (argc >= 3){
        level = argv[2][0] - '0';
    }
    printf("init args:[mode:%d][level:%d]\n", mode, level);
}

// 0, 1, 2, 3
const float pi = 3.14;
int mv_x[] = {1, 0, -1, 0};
int mv_y[] = {0, 1, 0, -1};
int rev[] = {2, 3, 0, 1};
int ori_x, ori_y;
tf::Quaternion q[4]; 
bool stop = false;
void clean_init(){
    q[0].setRPY(0, 0, 0);
    q[1].setRPY(0, 0, 0.5*pi);
    q[2].setRPY(0, 0, pi);
    q[3].setRPY(0, 0, 1.5*pi);
    ori_x = 0, ori_y = 0;
}

// 导航目标结构体
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;
bool move(int x, int y, MoveBaseClient &ac, tf::Quaternion &q, float du = 6.0){
    ros::Rate r(1);         //while函数的循环周期,这里为1Hz
    while(ros::ok())        //程序主循环
    {
        ros::spinOnce();        //短时间挂起,让回调函数得以调用
        r.sleep(); 
        if (!stop) break;
    }

    printf("try to move to (%d,%d)\n", x, y);
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
    ac.waitForResult(ros::Duration(du));
    if (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED){
        printf("move to (%d,%d) succeeded\n", x, y);
        return 1;
    }
    else {
        ac.cancelAllGoals();
        printf("move to (%d,%d) failed\n", x, y);
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
        _dir = (dir + off) % 4;
        _x = x + mv_x[_dir], _y = y + mv_y[_dir];
        
        if (!over(_x, _y)){

            setVis(_x, _y);
            if (move(_x, _y, ac, q[_dir])){
                for (int it = 0; it < level; it ++){// clean over
                    move(x, y, ac, q[rev[dir]]);
                    move(_x, _y, ac, q[_dir]);                
                }


                if (mode == 1 && dir != _dir){ // zigzag: turn point
                    _dir ++;
                }

                dfs(_x, _y, ac, _dir);
            }

 
        }
    }

    printf("try to go back...\n");
    move(x, y, ac, q[rev[dir]]);
}



void voiceCB(const std_msgs::String::ConstPtr &msg){
    printf("[test_voice]: %s\n", msg->data.c_str());
    int ret = 0;
    ret = msg->data.find("Stop"); 

    if ( ret >= 0){
        printf("[test_voice]: get Stop: %d\n", ret);
        stop = true;
        return;
    }

    ret = msg->data.find("Go");
    if ( ret >= 0){
        printf("[test_voice]: get Go: %d\n", ret);
        stop = false;
        return;
    }
}

int main(int argc, char** argv){
    ros::init(argc, argv, "dfs_clean");
    args_init(argc, argv);

    ros::NodeHandle n;
    ros::Subscriber sub_sr = n.subscribe("/xfyun/iat", 10, voiceCB);

    // 调用和主管监控导航功能的服务
    MoveBaseClient ac("move_base", true);

    // 休眠5s, 确认导航服务已经开启
    while (!ac.waitForServer(ros::Duration(5.0))){
        ROS_INFO("Waiting for the move_base action server to come up");
    }

    clean_init();

    setVis(ori_x, ori_y);
    if (move(ori_x, ori_y, ac, q[0], 10.0)){
        ROS_INFO("init the position");
        dfs(ori_x, ori_y, ac, 0);
    }
    else{
        ROS_INFO("position initial failed");
    }

    return 0;
}