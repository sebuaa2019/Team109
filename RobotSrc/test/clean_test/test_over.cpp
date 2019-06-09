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


bool G[200][200];
bool over(int x, int y){
    return x < 0 || y < 0 || x >= 200 || y >= 200 || G[x][y];
}

int main(int argc, char** argv){
    ros::init(argc, argv, "test_over");

	printf("[test_over] 200 * 200 map:\n");

	printf("[test_over] (0, 0): %d\n", over(0, 0));
	printf("[test_over] (0, 200): %d\n", over(0, 200));
	printf("[test_over] (200, 0): %d\n", over(200, 0));
	printf("[test_over] (200, 200): %d\n", over(200, 200));
	printf("[test_over] (0, 199): %d\n", over(0, 199));
	printf("[test_over] (199, 0): %d\n", over(199, 0));
	printf("[test_over] (199, 199): %d\n", over(199, 199));
	printf("[test_over] (-1, 199): %d\n", over(-1, 199));
	printf("[test_over] (199, -1): %d\n", over(199, -1));
	printf("[test_over] (-1, -1): %d\n", over(-1, -1));
	printf("[test_over] (100, 100): %d\n", over(100, 100));
	

    return 0;
}
