#include "matlab_engine.h"

namespace Matlab
{
	Matlab::Matlab() // Default constructor
	{
		std::cout << "Opening MATLAB\nOne moment...\n\n";

		// Start the MATLAB engine
		if (!(ep = engOpen(NULL)))
		{
			std::cout << "Matlab is having trouble opening!\n\n";
			std::cout << "Make sure that $(MATLABROOT)\\bin\\win64 "
				"is added to the PATH environment variable.\n\n"
				"Execute the following command in MATLAB to see what MATLABROOT should be set to:\n"
				"fullfile(matlabroot,'bin',computer('arch'))\n\n";
			std::cout << "The following link describes setting the run-time library path on Windows systems:\n"
				"https://www.mathworks.com/help/matlab/matlab_external/building-and-running-engine-applications-on-windows-operating-systems.html" << "\n\n";
			std::cout << "If matlab engine still doesn't open then try to "
				"re-register MATLAB as a COM-server by entering "
				"the following as admin in the MATLAB command window:\n"
				"!matlab - regserver\n\n";
			std::cout << "The following link describes registering MATLAB as a COM Server:\n"
				"https://www.mathworks.com/help/matlab/matlab_external/registering-matlab-software-as-a-com-server.html" << "\n\n";

			MessageBox((HWND)NULL, (LPSTR)"Can't start MATLAB engine - view terminal...",
				(LPSTR) "Engwindemo.c", MB_OK);

			std::cerr << "failed to start matlab" << std::endl;
			exit(-1);
		}

		// Open MATLAB GUI:
		command("desktop");

		// Move to the active working directory
		working_directory();

		std::cout << "MATLAB is open\n";
	}
	//=====================================================
	Matlab::~Matlab() // Destructor
	{
		//free(ep);
		engEvalString(ep, "close all;");
	}
	//=====================================================
	void Matlab::command(std::string str)
	{
		auto error_flag = engEvalString(ep, str.c_str());
		//if (error_flag)
		//	command("lasterror");
	}
	//=====================================================
	std::string ExePath() {
		char buffer[MAX_PATH];
		GetModuleFileName(NULL, buffer, MAX_PATH);
		const auto pos = std::string(buffer).find_last_of("\\/");
		return std::string(buffer).substr(0, pos);
	}
	//=====================================================
	void Matlab::working_directory()
	{
		// Reset MATLAB Environment
		command("clearAllMemoizedCaches");
		command("clc, clear, close all;");

		// Change MATLAB's workspace directory
		std::string current_path = ExePath();
		command("cd " + current_path); // Change directory to where .exe is generated
		command("cd ../../matlab");
	}
	//=====================================================
	void Matlab::pass_2D_into_matlab(float* data, size_t M, size_t N, std::string name)
	{
		auto lin = [](size_t idx, size_t jdx, size_t K) -> size_t { return idx * K + jdx; };

		// Transpose for col-major MATLAB and convert to double
		double* data_t = new double[M * N];
		for (size_t i = 0; i != M; ++i)
			for (size_t j = 0; j != N; ++j)
				data_t[lin(j, i, M)] = (double)data[lin(i, j, N)];

		// Copy image data into an mxArray inside C++ environment
		mxArray* mx_Arr = mxCreateDoubleMatrix(M, N, mxREAL);
		memcpy(mxGetPr(mx_Arr), data_t, M * N * sizeof(double));

		/// C++ -> MATLAB
		// Put variable into MATLAB workstpace
		engPutVariable(ep, name.c_str(), mx_Arr);

		delete[] data_t;
	}
}