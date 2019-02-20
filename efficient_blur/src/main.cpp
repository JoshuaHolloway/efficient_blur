#include <matlab_engine.h>
#include "Timer.h"
#include "2D_arrays.h"
#include "2D.h"
#include "1D.h"

// - - - - - - - - - - - - - - - - 
auto main() -> int
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
	//two_D::imp_1();
	//two_D::imp_2();
	//two_D::imp_3();
	//two_D::imp_4();
	//two_D::imp_4_input8x8_tile4x4();
	//two_D::imp_4_input8x8_tile6x6();
	
	//float* imp_4_input8x8_tile6x6_dynamic_var = two_D::imp_4_input8x8_tile6x6_dynamic();
	float* imp_4_input16x16_tile6x6_dynamic_var = two_D::imp_4_input16x16_tile6x6_dynamic();
	
#ifdef MATLAB
	// Compare result against golden reference
	Matlab::Matlab matlab;
	matlab.pass_2D_into_matlab(imp_4_input16x16_tile6x6_dynamic_var, 16, 16, "cpp");
	matlab.command("box_blur_16x16");
#endif

	delete[] imp_4_input16x16_tile6x6_dynamic_var;
	return 0;
}