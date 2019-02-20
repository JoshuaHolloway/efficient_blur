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
	constexpr size_t P = 1;		  // Zero-padding on each side
	constexpr size_t K = 2*P + 1; // Size of kernel in each dim
	// - - - - - - - - - - - - - - - - 
	inline size_t lin(size_t i, size_t j, size_t cols)
	{
		return i * cols + j;
	}
	// - - - - - - - - - - - - - - - - 
	float* pad(const float* x, size_t N_, size_t P_, size_t Q_)
	{
		auto loop_print = [=](size_t i, size_t j, float val, string name) -> void
		{cout << name << "(" << i << "," << j << ") = " << val << "\t"; };

		float* x_zp = new float[Q_ * Q_];
		for (int i = 0; i < Q_*Q_; ++i)
			x_zp[i] = 0;

		float* local_arr = new float[Q_ * Q_];
		for (size_t n1 = P; n1 != Q_ - P_; ++n1)
		{
			for (size_t n2 = P; n2 != Q_ - P_; ++n2)
			{
				//s.arr[n1][n2] = x[n1 - P][n2 - P];
				x_zp[lin(n1, n2, Q_)] = x[lin(n1 - P_, n2 - P_, N_)];
			}
		}

		return x_zp;
	}
	// - - - - - - - - - - - - - - - -
	float* conv(float* x_zp, size_t N_, size_t K_, size_t P_, size_t Q_)
	{
		// Same as previous conv() just with 1D-array
		// Also, added the non-unity filter taps
		//  to avoid the DC-offset introduced during filtering

		// Coefficients in the 2D-FIR filter
		float box_amplitude = 1 / float(K_);

		float* y = new float[N_ * N_];
		for (size_t n1 = P_; n1 < Q_ - P_; n1++)
			for (size_t n2 = 0; n2 < N_; n2++)
			{
				float sum = 0.f;
				for (size_t k2 = 0; k2 != K_; k2++)
				{
					int i = n1;
					int j = n2 + k2;
					sum += x_zp[lin(i, j, Q_)];
				}
				y[lin(n2, n1 - P_, N_)] = sum * box_amplitude;
			}

		return y;
	}
	// - - - - - - - - - - - - - - - - 
	cv::Mat imp_4_general(const cv::Mat& x)
	{
		const size_t rows = x.rows;
		const size_t cols = x.cols;

		float* x_1D = new float[rows * cols];
		cv::Mat x_f(rows, cols, CV_32FC1);
		x.convertTo(x_f, CV_32FC1);
		for (int i = 0; i < rows; ++i)
			for (int j = 0; j < cols; ++j)
				x_1D[lin(i, j, cols)] = x_f.at<float>(i, j);


		const size_t N_ = cols;
		const size_t P_ = K / 2;	    // Zero-padding on each side
		const size_t Q_ = N_ + 2 * P_; // Size of zero-padded in each dim

		// pad
		//float* x_1D = mat_2_arr(x);
		//print("x_arr", x_1D, N, N);
		float* x_1D_zp = pad(x_1D, N_, P_, Q_);
		float* y_1D = conv(x_1D_zp, N_, K, P_, Q_);
		float* y_1D_zp = pad(y_1D, N_, P_, Q_);
		float* z_1D = conv(y_1D_zp, N_, K, P_, Q_);

		// convert back to mat

		cv::Mat z(rows, cols, CV_8UC1);
		for (int i = 0; i < rows*cols; ++i)
			z.data[i] = (unsigned char)z_1D[i];

		cv::imshow("debug_mat", z);
		cv::waitKey(0);


		return z;
	}
}