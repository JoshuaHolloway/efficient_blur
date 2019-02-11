#pragma once
namespace one_D
{
	constexpr size_t N = 4;         // Size of input image in each dim
	constexpr size_t K = 3;         // Size of kernel in each dim
	constexpr size_t P = K / 2;		// Zero-padding on each side
	constexpr size_t Q = N + 2 * P; // Size of zero-padded in each dim
	// - - - - - - - - - - - - - - - - 
	
	// L1 cache-line is 64-Bytes
	__declspec(align(64))
	struct ArrStruct_zp { int arr[Q]; };

	__declspec(align(64))
	struct ArrStruct { int arr[N]; };
	// - - - - - - - - - - - - - - - - 
	template <size_t cols, typename T>
	void print(std::string str, T(&array)[cols])
	{
		using std::cout;
		cout << __func__ << std::endl;
		cout << str.c_str() << "\n";

		for (size_t j = 0; j < cols; ++j)
			cout << array[j] << '\t';
		cout << std::endl;
	}
	// - - - - - - - - - - - - - - - - 
	template <size_t cols, typename T>
	ArrStruct_zp pad(T(&x1)[cols])
	{
		// Input: Row-vector
		ArrStruct_zp s = {};
		for (size_t n = P; n != Q - P; ++n)
			s.arr[n] = x1[n - P];
		return s;
	}
	// - - - - - - - - - - - - - - - -
	template <size_t N_, typename T>
	ArrStruct conv(T(&x_zp)[N_])
	{
		// 1D-conv of (1xK) box-filter with (1xN_) signal
		// MATLAB: conv(x,ones(1,K),'same')

		// Do Convolution
		ArrStruct y = {};
		//cout << "cols = " << N_ << "\n";

		// 1D-conv (row-vector)
		for (size_t n2 = 0; n2 != N_; ++n2)
		{
			int sum = 0;
			for (size_t k2 = 0; k2 != K; ++k2)
			{
				sum += x_zp[n2 + k2];
				//cout << "(n2):(" << n2 << ")=" << x_zp[n2 + k2] << "\n";
			}
			y.arr[n2] = sum;
			//cout << "sum = " << sum << "\n";
			//getchar();
		}
		return y;
	}
	// - - - - - - - - - - - - - - - - 
	void one_D()
	{
		// Initialize
		int x[N] = { 1, 2, 3, 4 };

		// Zero-pad
		ArrStruct_zp x_zp = pad(x);

		// Diplay
		print("After zero-padding", x_zp.arr);

		// Do conv
		ArrStruct y = conv(x_zp.arr);

		// Display
		print("After conv", y.arr);
		getchar();
	}
} // namespace one_D