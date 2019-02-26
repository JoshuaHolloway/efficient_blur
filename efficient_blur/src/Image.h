#pragma once
#include <windows.h>
#include <iostream>
#include <string>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"

// TODO: Only use zero padded dimensions - remove non-zero padded info

class Image
{
public:
	Image(size_t, size_t);
	Image(const float*, size_t, size_t, size_t); // zero-padded image
	Image(const float*, size_t M_, size_t N_, size_t K, size_t N_f); // zero-padded extra on right and bottom to evenly fit tiles
	Image(const cv::Mat&, size_t);
	~Image();

	size_t height() const;
	size_t width() const;
	size_t stride() const;

	void view();

	void print(std::string);

	void truncate(size_t, size_t, Image&);

private:
	size_t M{};
	size_t N{};
	size_t pitch{};
public:
	float *data;
};