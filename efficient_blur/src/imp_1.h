#pragma once
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include <iostream>
#include <string>
using std::cout;
using std::string;

namespace naive
{
	// - - - - - - - - - - - - - - - - 
	constexpr size_t P = 1;		    // Zero-padding on each side
	constexpr size_t K = 2 * P + 1; // Size of kernel in each dim
	// - - - - - - - - - - - - - - - - 
	inline size_t lin(size_t i, size_t j, size_t cols)
	{
		return i * cols + j;
	}
	// - - - - - - - - - - - - - - - - 
	float* pad(const float* x, const size_t N)
	{
		auto loop_print = [=](size_t i, size_t j, float val, string name) -> void
		{cout << name << "(" << i << "," << j << ") = " << val << "\t"; };
		
		const size_t Q = N + 2 * P; // Full zero-padded size

		float* x_zp = new float[Q * Q];
		for (int i = 0; i < Q*Q; ++i)
			x_zp[i] = 0;

		float* local_arr = new float[Q * Q];
		for (size_t n1 = P; n1 != Q - P; ++n1)
			for (size_t n2 = P; n2 != Q - P; ++n2)
				x_zp[lin(n1, n2, Q)] = x[lin(n1 - P, n2 - P, N)];
		return x_zp;
	}
	// - - - - - - - - - - - - - - - -
	float* conv(float* x_zp, size_t N)
	{
		// Coefficients of the 2D-FIR filter
		float box_amplitude = 1 / float(K);

		const size_t Q = N + 2 * P; // Full zero-padded size

		float* y = new float[N * N];
		for (size_t n1 = P; n1 < Q - P; n1++)
			for (size_t n2 = 0; n2 < N; n2++)
			{
				float sum = 0.f;
				for (size_t k2 = 0; k2 != K; k2++)
				{
					int i = n1;
					int j = n2 + k2;
					sum += x_zp[lin(i, j, Q)];
				}
				y[lin(n2, n1 - P, N)] = sum * box_amplitude;
			}

		return y;
	}
	// - - - - - - - - - - - - - - - - 
	float* imp_1_general(const float* x_f, const size_t N)
	{
		float* x_f_zp = pad(x_f, N);
		float* y_f = conv(x_f_zp, N);
		float* y_f_zp = pad(y_f, N);
		float* z_f = conv(y_f_zp, N);

		delete[] x_f_zp, y_f, y_f_zp;
		return z_f;
	}
}