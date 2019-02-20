#include <matlab_engine.h>
#include "Timer.h"
#include "2D_arrays.h"
#include "2D.h"
#include "1D.h"

// - - - - - - - - - - - - - - - - 
auto main() -> int
{
	//cv::Mat x = cv::imread("lena_8.png",CV_LOAD_IMAGE_GRAYSCALE);
	cv::Mat x = cv::imread("lena_512.png", CV_LOAD_IMAGE_GRAYSCALE);

	//float* imp_4_input16x16_tile6x6_dynamic_var = two_D::imp_4_input16x16_tile6x6_dynamic();
	float* z = two_D::imp_4_general_tile6x6(x);
	
#ifdef MATLAB
	// Compare result against golden reference
	Matlab::Matlab matlab;
	//matlab.pass_2D_into_matlab(imp_4_input16x16_tile6x6_dynamic_var, 16, 16, "cpp");
	matlab.pass_2D_into_matlab(z, x.rows, x.cols, "cpp");

	//matlab.command("box_blur_16x16");
	matlab.command("box_blur");
#endif

	getchar();
	delete[] z;
	return 0;
}