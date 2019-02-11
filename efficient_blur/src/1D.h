#pragma once
namespace one_D
{
	constexpr size_t N = 4;         // Size of input image in each dim
	constexpr size_t K = 3;         // Size of kernel in each dim
	constexpr size_t P = K / 2;		// Zero-padding on each side
	constexpr size_t Q = N + 2 * P; // Size of zero-padded in each dim
	struct ArrStruct {int arr[Q]; };
	// - - - - - - - - - - - - - - - - 
	template <size_t cols>
	ArrStruct pad(int(&x1)[cols])
	{
		// Input: Row-vector
		ArrStruct s = {};
		for (size_t n = P; n != Q - P; n++)
			s.arr[n] = x1[n - P];
		return s;
	}
	// - - - - - - - - - - - - - - - - 
	template <size_t cols>
	void print(std::string str, int(&array)[cols])
	{
		using std::cout;
		cout << __func__ << std::endl;
		cout << str.c_str() << "\n";

		for (size_t j = 0; j < cols; ++j)
			cout << array[j] << '\t';
		cout << std::endl;
	}
	// - - - - - - - - - - - - - - - - 
	void one_D()
	{
		// Initialize
		int x[N] = { 1,  2,  3, 4};

		// Zero-pad
		ArrStruct x_zp = pad(x);

		// Diplay
		print("After zero-padding", x_zp.arr);
	}
} // namespace one_D