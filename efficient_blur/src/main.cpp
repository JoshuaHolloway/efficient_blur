#include <matlab_engine.h>
#include "Timer.h"
#include "2D_arrays.h"
#include "2D.h"
#include "1D.h"
#include "imp_4.h"
// - - - - - - - - - - - - - - - - 
auto main() -> int
{
	// Organizing code to run experiments on imp-1 vs. imp-4
	/// Step 1: Create seperate file for imp-4
	// Step 2: Create seperate file for imp-1
	//			Step 2.a: Remove redundant code
	// Step 3: Scale up imp-1 to images


	// Prototype:
	tiled::imp_4();
	
	cv::Mat x = cv::imread("lena_512.png", CV_LOAD_IMAGE_GRAYSCALE);
	float* z = tiled::imp_4_general_tile6x6(x);
	
#ifdef MATLAB
	// Compare result against golden reference
	Matlab::Matlab matlab;
	matlab.pass_2D_into_matlab(z, x.rows, x.cols, "cpp");
	matlab.command("box_blur");
#endif

	getchar();
	delete[] z;
	return 0;
}