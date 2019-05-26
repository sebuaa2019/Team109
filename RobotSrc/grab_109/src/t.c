#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

using namespace cv;

int 
main()
{
	cv::Mat src, dst, yuv;
	std::vector<cv::Mat> channels;
	
	cv::cvtColor(src, yuv, COLOR_BGR2YCrYCb);
	//把一个3通道图像转为三个但通道图像
	cv::split(yuv, channels);
	equalizeHist(channels[0], channels[0]); // 均衡化

	cv::merge(channels, dst); // 合并
	
	return 0;
}

