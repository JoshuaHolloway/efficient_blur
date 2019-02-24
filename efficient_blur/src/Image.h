#pragma once
#include <windows.h>
#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"

// TODO: Only use zero padded dimensions - remove non-zero padded info

class Image
{
public:
	Image(size_t, size_t);
	Image(const float*, size_t, size_t, size_t);
	Image(const cv::Mat&, size_t);
	~Image();

	size_t height() const;
	size_t width() const;
	size_t stride() const;

	void view();

	void print();

private:
	size_t M{};
	size_t N{};
	size_t pitch{};
public:
	float *data;
};