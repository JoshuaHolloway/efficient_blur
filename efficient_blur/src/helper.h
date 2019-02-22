#pragma once
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"
// - - - - - - - - - - - - - - - -
namespace Helper
{
	float* mat2arr(const cv::Mat& mat)
	{
		auto lin = [](size_t i, size_t j, size_t N)
		{ return i * N + j; };

		const size_t M = mat.rows;
		const size_t N = mat.cols;

		float* x_1D = new float[M * N];
		cv::Mat mat_f(M, N, CV_32FC1);
		mat.convertTo(mat_f, CV_32FC1);
		for (int i = 0; i < M; ++i)
			for (int j = 0; j < N; ++j)
				x_1D[lin(i, j, N)] = mat_f.at<float>(i, j);
		return x_1D;
	}
}