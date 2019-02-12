#include "2D.h"
#include "1D.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
// - - - - - - - - - - - - - - - - 
int main()
{
	cv::Mat mat(2,2,CV_32FC1);
	cv::imshow("mat", mat);

	//one_D::one_D();
	two_D::two_D();
	return 0;
}