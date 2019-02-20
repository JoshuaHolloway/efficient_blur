#pragma once
#include "engine.h"  // MATLAB Engine Header File required for building in Visual Studio 
#include "mex.h"
// - - - - - - - - - - - - - - - - - - - -
#include <windows.h>
#include <string>
#include <iostream>
// - - - - - - - - - - - - - - - - - - - -
namespace Matlab
{
	class Matlab
	{
	private:
		Engine*  ep;      // Pointer to a MATLAB Engine
		//mxArray* mx_Arr;  // To store the image data inside MATLAB
	public:
		Matlab();
		~Matlab();
		void command(std::string);
		void working_directory();
		void pass_2D_into_matlab(float*, size_t, size_t, std::string);
	};
}