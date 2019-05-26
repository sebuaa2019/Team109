#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <iostream>
using namespace std;

/*
int 
valid(const cv::Mat& index, int x, int y){
	return index[x][y] ;
&& index[x-1][y] \
		&& index[x][y-1] && index[x][y+1] && index[x+1][y-1] \
		&& index[x+1][y] && index[x+1][y+1];
}*/


void
rect(cv::Mat &index, cv::Rect& box){
	
	int row = index.rows;
	int col = index.cols;
	
	int lx, ly, bx, by;
	
	lx = row; ly = col;
	bx = by = 0;
	
int cnt=0;
	for(int x=1; x<row-1; x++){
		for(int y=1; y<col-1; y++){
			if (index.at<int>(x, y)==255){
				if(cnt<100)
				cout<<(index.at<int>(x,y))<<" ";
cnt++;
				if (x < lx) lx = x;
				if (x > bx) bx = x;
				if (y < ly) ly = y;
				if (y > by) by = y;
			}
		}
	}
	box.x = lx; box.y = ly;
	box.width = by - ly;
	box.height = bx - lx;
	cout<<endl;
}

void
extractTemp(cv::Mat& dst, cv::Mat& src)
{
	cv::Mat mask;
	cv::Rect box;
	
	cv::inRange(src, cv::Scalar(0, 90, 90), cv::Scalar(150, 220, 220), mask);
	
	cout<<mask.rows<<" "<<mask.cols<<endl;
	cout<<"mask "<<(mask.at<int>(1, 1))<<" end"<<endl;
	//cout<<mask<<endl;
	
cout<<mask(cv::Rect(1, 1, 50, 50))<<endl;
	rect(mask, box);
	
	cout<<box<<endl;
	box &= cv::Rect(0, 0, src.cols, src.rows);
	cout<<box<<endl;
	
	cout<<src.rows<<" "<<src.cols<<endl;

	dst = src(box);
	
	//cv::imshow("ROI", box);
	//cv::waitKey(0);
}

int 
main(int argc, char **argv)
{
	if(argc != 3){
		printf("usage: ./extract path_src_img path_dst_img\n");
		return -1;
	}
	
	cv::Mat src, dst;
	src = cv::imread(argv[1]);
	if(src.data==NULL){
		printf("no img found %s\n", argv[1]);
		return -1;
	}
	//cv::imshow("src", src);
	//cv::waitkey(0);

	extractTemp(dst, src);
	cv::imwrite(argv[2], dst);
	
	//cv::imshow("roi", dst);
	//cv::waitKey(0);
	return 0;
}

