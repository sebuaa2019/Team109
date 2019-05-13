#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <tf/tf.h>
#include <tf/transform_listener.h>

// 导航目标结构体
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

bool move(int x, int y, MoveBaseClient &ac, tf::Quaternion &q, float du = 6.0){
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

const float pi = 3.14;
bool G[200][200];
bool over(int x, int y){
    x += 100, y += 100;
    return x < 0 || y < 0 || x >= 200 || y >= 200 || G[x][y];
}
void setVis(int x, int y){
    x += 100, y += 100;
    G[x][y] = 1;
}

// 0, 1, 2, 3
int mv_x[] = {1, 0, -1, 0};
int mv_y[] = {0, 1, 0, -1};
int rev[] = {2, 3, 0, 1};
int ori_x, ori_y;
tf::Quaternion q[4]; 
void clean_init(){
    q[0].setRPY(0, 0, 0);
    q[1].setRPY(0, 0, 0.5*pi);
    q[2].setRPY(0, 0, pi);
    q[3].setRPY(0, 0, 1.5*pi);
    ori_x = 1, ori_y = 1;
}

void dfs(int x, int y, MoveBaseClient &ac, int dir){
    for (int off = 0, _dir, _x, _y; off < 4; off ++){
        _dir = (dir + off) % 4;
        _x = x + mv_x[_dir], _y = y + mv_y[_dir];
        
        if (!over(_x, _y)){

            setVis(_x, _y);
            if (move(_x, _y, ac, q[_dir])){
                dfs(_x, _y, ac, _dir);
            }
 
        }
    }

    printf("try to go back...\n");
    move(x, y, ac, q[rev[dir]]);
}

void zigzag_dfs(int x, int y, MoveBaseClient &ac, int dir){
    for (int off = 0, _dir, _x, _y; off < 4; off ++){
        _dir = (dir + off) % 4;
        _x = x + mv_x[_dir], _y = y + mv_y[_dir];
        
        if (!over(_x, _y)){

            setVis(_x, _y);
            if (move(_x, _y, ac, q[_dir])){
                if (dir != _dir){ // turn point
                    _dir ++;
                }

                zigzag_dfs(_x, _y, ac, _dir);
            }
 
        }
    }

    printf("try to go back...\n");
    move(x, y, ac, q[rev[dir]]);
}

int main(int argc, char** argv){
    ros::init(argc, argv, "dfs_clean");

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
        if (argc == 0){
            dfs(ori_x, ori_y, ac, 0);
        }
        else{
            zigzag_dfs(ori_x, ori_y, ac, 0);
        }
    }
    else{
        ROS_INFO("position initial failed");
    }

    return 0;
}