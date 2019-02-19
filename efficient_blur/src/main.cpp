#include "Timer.h"
#include "2D_arrays.h"
#include "2D.h"
#include "1D.h"
// - - - - - - - - - - - - - - - - 
int main()
{
	Timer<> timer1;
	Timer<std::chrono::seconds> timer2;

	std::vector<int> v(5e6);
	std::sort(std::begin(v), std::end(v));
	timer1.end();


	cv::Mat x = cv::imread("lena_512.png",CV_LOAD_IMAGE_GRAYSCALE);
	//cv::imshow("mat", mat);
	//cv::waitKey(0);

	//int N = 2;
	//int** arr = getArray(N);
	//printArray(arr, N);
	
	//one_D::one_D();

	//two_D::two_D();
	//cv::Mat y = two_D::two_D_image(x);
	//two_D::imp_1();
	//two_D::imp_2();
	//two_D::imp_3();
	//two_D::imp_4();
	two_D::imp_5();
	return 0;
}