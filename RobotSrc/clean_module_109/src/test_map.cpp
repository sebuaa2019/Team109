#include <ros/ros.h>
#include <costmap_2d/costmap_2d.h>
#include <costmap_2d/costmap_2d_ros.h>
#include <tf2_ros/transform_listener.h>

int n, m;
costmap_2d::Costmap2D* map2d;

void func(tf2_ros::Buffer& tf){
    costmap_2d::Costmap2DROS* map2dROS = new costmap_2d::Costmap2DROS("global_costmap", tf);
    map2d = map2dROS->getCostmap();

    n = map2d->getSizeInCellsX();
    m = map2d->getSizeInCellsY();


}

int main(int argc, char** argv){
    ros::init(argc, argv, "test_map");

    tf2_ros::Buffer tf(ros::Duration(10));
    func(tf);
    
    printf("the map is %dx%d:\n", n, m);
    for (int i = 0; i < n; i ++){
        for (int j = 0; j < m; j ++){
            printf("%d ", map2d->getCost(i, j));
        }
        printf("\n");
    }

    return 0;
}