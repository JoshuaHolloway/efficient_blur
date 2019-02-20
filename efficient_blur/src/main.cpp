#include <matlab_engine.h>
#include "Timer.h"
#include "2D_arrays.h"
#include "2D.h"
#include "1D.h"
#include "imp_4.h"
#include "imp_1.h"
// - - - - - - - - - - - - - - - - 
auto main() -> int
{
	cv::Mat x = cv::imread("lena_512.png", CV_LOAD_IMAGE_GRAYSCALE);

	// Organizing code to run experiments on imp-1 vs. imp-4
	// Step 1: Create separate file for imp-4
	/// Step 2: Create separate file for imp-1
	// Step 3: Modify both to use normalized taps
	// Step 4: Fix imp-1 to write to buffer already 
	//			zero-padded to match imp-4
	// Step 5: Scale up imp-1 to images


	/// Imp-1
									 // prototype
	cv::Mat z_1 = naive::imp_4_general(x); // final

	/// Imp-4
	tiled::imp_4(); // prototype
	float* z_4 = tiled::imp_4_general_tile6x6(x); // final
	
#ifdef MATLAB
	// Compare result against golden reference
	Matlab::Matlab matlab;
	matlab.pass_2D_into_matlab(z_4, x.rows, x.cols, "cpp");

	// TODO: Modify script to test imp-1
	matlab.command("box_blur");
#endif

	getchar();
	delete[] z_4;
	return 0;
}