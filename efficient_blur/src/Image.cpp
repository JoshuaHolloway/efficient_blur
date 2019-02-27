#include "Image.h"

Image::Image(size_t M_, size_t N_)
	: M(M_), N(N_), // zero-padded dimensions
	pitch(N),
	data{ new float[M * N] }
{
	for (int i = 0; i < M * N; ++i)
		*(data + i) = 0;
}

// zero-padded image
Image::Image(const float* arr, size_t M_ , size_t N_, size_t K)
	: M(M_ + 2*(K/2)), N(N_ + 2*(K/2)), // zero-padded dimensions
	pitch(N),
	data{ new float[M * N] }
{
	auto index = [](size_t i, size_t j, size_t N) -> int
	{ return i * N + j; };

	for (int i = 0; i < M * N; ++i)
		*(data + i) = 0;

	size_t P = K / 2; // Ammount of zero-padding on each size
	for (size_t n1 = P; n1 != M - P; ++n1)
		for (size_t n2 = P; n2 != N - P; ++n2)
			data[index(n1, n2, pitch)] = arr[index(n1 - P, n2 - P, N_)];
}

// zero-padded extra on right and bottom to evenly fit tiles
Image::Image(const float* arr, size_t M_, size_t N_, size_t K, size_t N_f)
	: M(N_f), N(N_f), // Full size needed for zero-padding and to evenly fit tiles
	pitch(N),
	data{ new float[M * N] }
{
	auto index = [](size_t i, size_t j, size_t N) -> int
	{ return i * N + j; };

	for (int i = 0; i < M * N; ++i)
		*(data + i) = 0;

	size_t P = K / 2; // Ammount of zero-padding on each size
	for (size_t n1 = P; n1 != M_ + P; ++n1)
		for (size_t n2 = P; n2 != N_ + P; ++n2)
			data[index(n1, n2, pitch)] = arr[index(n1 - P, n2 - P, N_)];
}

Image::Image(const cv::Mat& mat, size_t K)
	: M(mat.rows + K/2), N(mat.cols + K/2), // zero-padded dimensions
	pitch(N),
	data{ new float[M * N] }
{
	auto index = [](size_t i, size_t j, size_t N) -> int
	{ return i * N + j; };

	// TODO: Remove
	cv::Mat mat_f(M, N, CV_32FC1);
	mat.convertTo(mat_f, CV_32FC1);
	
	// TODO: Remove
	//float* non_padded = new float[mat.rows * mat.cols];
	//for (int i = 0; i < mat.rows; ++i)
	//	for (int j = 0; j < mat.cols; ++j)
	//		non_padded[index(i, j, mat.cols)] = mat_f.at<float>(i, j);

	// Zero-pad
	for (int i = 0; i < M * N; ++i)
		*(data + i) = 0;

	// TODO: Loop on pointers
	size_t P = K / 2; // Ammount of zero-padding on each size
	for (size_t n1 = P; n1 != M - P; ++n1)
		for (size_t n2 = P; n2 != N - P; ++n2)
			data[index(n1, n2, pitch)] = mat_f.at<float>(n1 - P, n2 - P);
}

Image::~Image()
{
	delete[] data;
}

size_t Image::height() const { return M; }
size_t Image::width() const { return N; }
size_t Image::stride() const { return pitch; }

void Image::view()
{
	// TODO: Loop on pointers
	cv::Mat mat(M, N, CV_8UC1);
	for (size_t n1 = 0; n1 != M; ++n1)
		for (size_t n2 = 0; n2 != N; ++n2)
			mat.at<unsigned char>(n1, n2) = data[n1 * N + n2];

	cv::imshow("view()", mat);
	cv::waitKey(0);
}

void Image::print(std::string s) const
{
	using std::cout;
	auto loop_print = [=](size_t i, size_t j, float val, std::string name) -> void
	{cout << name << "(" << i << "," << j << ") = " << val << "\t"; };

	auto index = [](size_t i, size_t j, size_t N) -> int
	{ return i * N + j; };

	for (size_t i = 0; i < M; ++i)
	{
		for (size_t j = 0; j < N; ++j)
		{
			auto temp = data[index(i, j, pitch)];
			loop_print(i, j, temp, s.c_str());
		}
		cout << std::endl;
	}
	cout << "\n";
}

void Image::truncate(size_t M_, size_t N_, Image& y)
{
	auto index = [](size_t i, size_t j, size_t N) -> int
	{ return i * N + j; };

	for (int i = 0; i < M_; i++)
		for (int j = 0; j < N_; j++)
			y.data[index(i, j, N_)] = data[index(i, j, N)];

}
