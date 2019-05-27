#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <tf/tf.h>
#include <tf/transform_listener.h>
#include <std_msgs/String.h>

int mode; // 0:dfs, 1:zigzag
int level;

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

bool move(int x, int y){
    ROS_INFO("[test_dfs] move to (%d,%d)\n", x, y);
	return 1;
}

bool G[4][4];
bool over(int x, int y){
    return x < 0 || y < 0 || x >= 4 || y >= 4 || G[x][y];
}
void setVis(int x, int y){
    G[x][y] = 1;
}

void dfs(int x, int y, int dir, bool turn_point){
    for (int off = 0, _dir, _x, _y; off < 4; off ++){
        _dir = (dir + off) % 4;
        _x = x + mv_x[_dir], _y = y + mv_y[_dir];
        
        if (!over(_x, _y)){

            setVis(_x, _y);
            if (move(_x, _y)){
                for (int it = 0; it < level; it ++){// clean over
                    move(x, y);
                    move(_x, _y);                
                }

                bool flag = (dir != _dir && !turn_point);
                if (mode == 1 && dir != _dir && !turn_point){ // zigzag: turn point
                    _dir ++;
                }

                dfs(_x, _y, _dir, flag);
            }

 
        }
    }

    ROS_INFO("[test_dfs] try to go back...\n");
    move(x, y);
}

int main(int argc, char** argv){
    ros::init(argc, argv, "test_dfs");
    clean_init();

	/*=====================================================*/

	ROS_INFO("[test_dfs] mode:0 level:0");
	mode = 0, level = 0;
	for (int i = 0; i < 4; i ++)
		for (int j = 0; j < 4; j ++)
			G[i][j] = false;
	
    setVis(ori_x, ori_y);
    move(ori_x, ori_y);
    dfs(ori_x, ori_y, 0, 0);

	/*=====================================================*/

	ROS_INFO("[test_dfs] mode:1 level:0");
	mode = 1, level = 0;
	for (int i = 0; i < 4; i ++)
		for (int j = 0; j < 4; j ++)
			G[i][j] = false;
	
    setVis(ori_x, ori_y);
    move(ori_x, ori_y);
    dfs(ori_x, ori_y, 0, 0);

	/*=====================================================*/

	ROS_INFO("[test_dfs] mode:1 level:3");
	mode = 1, level = 3;
	for (int i = 0; i < 4; i ++)
		for (int j = 0; j < 4; j ++)
			G[i][j] = false;
	
    setVis(ori_x, ori_y);
    move(ori_x, ori_y);
    dfs(ori_x, ori_y, 0, 0);
	
    return 0;
}
