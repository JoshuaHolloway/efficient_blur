#include "2D_arrays.h"
#include "2D.h"
#include "1D.h"
// - - - - - - - - - - - - - - - - 
int main()
{
	cv::Mat x = cv::imread("lena_512.png",CV_LOAD_IMAGE_GRAYSCALE);
	//cv::imshow("mat", mat);
	//cv::waitKey(0);

	//int N = 2;
	//int** arr = getArray(N);
	//printArray(arr, N);
	
	//one_D::one_D();

	//two_D::two_D();
	//cv::Mat y = two_D::two_D_image(x);
	two_D::extreme_1();
	//two_D::extreme_2();
	return 0;
}