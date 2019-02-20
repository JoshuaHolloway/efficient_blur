#pragma once
#include <chrono>

template<class Resolution = std::chrono::milliseconds>
class Timer
{
public:
	using Clock = std::conditional_t<
		std::chrono::high_resolution_clock::is_steady,
		std::chrono::high_resolution_clock,
		std::chrono::steady_clock>;
	
	Timer() = default;
	~Timer() = default;

	void end()
	{
		std::cout << "Elapsed time: "
			<< std::chrono::duration_cast<Resolution>(Clock::now() - mStart).count()
			<< std::endl;
	}

	void example()
	{
		//Timer<> timer1;
		//Timer<std::chrono::seconds> timer2;
		//
		//std::vector<int> v(5e6);
		//std::sort(std::begin(v), std::end(v));
		//timer1.end();
	}

private:
	Clock::time_point mStart = Clock::now();
};