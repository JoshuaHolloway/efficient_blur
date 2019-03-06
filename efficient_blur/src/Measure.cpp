#include "Measure.h"

Measure::Measure()
	: adds(0), mults(0), 
	buffer_writes(0), buffer_reads(0),
	memory_writes(0), memory_reads(0)
{
	// frequency
	LARGE_INTEGER perf_counter_freq_union{ 0 };
	QueryPerformanceFrequency(&perf_counter_freq_union);
	perf_counter_freq = (double)perf_counter_freq_union.QuadPart;
}

Measure::~Measure()
{}

void Measure::tic()
{
	cycle_counter_start = __rdtsc();
	QueryPerformanceCounter(&perf_counter_start);
}

#include <iostream>
#include <iomanip>
void Measure::toc()
{
	// refere to Measure.h to see documentation
	//
	// 2 tools:
	//	-A. QueryPerformanceCounter
	//		-Units: seconds (after using QueryPerformanceFrequency and scaling)
	//		-Resolution: nano-seconds
	//		-This is used to quantify ammount of time elapsed between two instances irregardless of work.
	//		-This is used to ask windows what its actual wall clock time is
	//		-QueryPerformanceFrequency is used in confunction with QueryPerformanceCounter.
	//			-Used to translate the performance counter back to human readable time
	//			-Units [performance counter increments / second]
	//		-seconds are computed as follows:
	//			time-elapsed [seconds] = ([counter increments]) * ([counter increments]/s.)^-1
	//			=> perf_counter_s  =       perf_counter_elapsed / perf_counter_freq
	//			=> perf_counter_ms = 1e3 * perf_counter_elapsed / perf_counter_freq  -- because 1e3 incs/ms.
	//			=> perf_counter_us = 1e6 * perf_counter_elapsed / perf_counter_freq  -- because 1e6 incs/us.
	//			=> perf_counter_ns = 1e9 * perf_counter_elapsed / perf_counter_freq  -- because 1e9 incs/ns.
	//		-Note: To guard against loss-of-precision scale up before division.
	//
	//	-B. __rdtsc() intrinsic returns the time stamp from rdtsc increments every time a clock-cyle is retired
	//		-Units: [cycles]
	//		-This is used to quantify ammount of work done between two instances irregardless of time.
	//		Note: There is a number of factors needed to note:
	//			1. Not all OS's save and resture the time-stamp counter across task-switches.
	//			   Therefore, if your process is running and you do an rdtsc at the beginning and end
	//			   of some code you won't know if the OS woke up in the middle of this code and swapped out
	//			   the code and ran some other code and then on the same processor swapped in our code again.
	//				Therefore, when using the rdtcs we need to ensure we are timing everything the processor did between these two points and not just the thing that our code did.
	//				This behavior is dependent on the OS.
	//			2. We don't actually know where the reported cycles are coming from.
	//			   These cycles don't nessesarily mean instructions, they mean cycles which may be
	//			   dependent on how warm the cache was, how much time it took to retrieve memory, etc.

	QueryPerformanceCounter(&perf_counter_stop);
	perf_counter_elapsed 
		= (double)perf_counter_stop.QuadPart
		- (double)perf_counter_start.QuadPart;

	// We now have the elapsed number of ticks, along with the
	// number of ticks-per-second. We use these values
	// to convert to the number of elapsed microseconds.
	perf_counter_s =         perf_counter_elapsed / perf_counter_freq;
	perf_counter_ms = 1.e3 * perf_counter_elapsed / perf_counter_freq;
	perf_counter_us = 1.e6 * perf_counter_elapsed / perf_counter_freq;
	perf_counter_ns = 1.e9 * perf_counter_elapsed / perf_counter_freq;

	fps = 1. / perf_counter_s; // 1 / [s./frame]

	cycle_counter_stop = __rdtsc();
	cycle_elapsed = cycle_counter_stop - cycle_counter_start;
	cycle_elapsed_M = (double)cycle_elapsed / 1.e6;
	cycle_elapsed_G = (double)cycle_elapsed / 1.e9;

	//  ( Frames / s. ) x ( Cycles / frame. ) = Clock Speed
	double clock_speed_approximate_GHz = (double)fps * cycle_elapsed_G;
	// Note: clock_speed_measured is measured more accurately

	std::cout << "\n\nINSIDE toc():\n";
	std::cout << std::fixed << std::setprecision(2);
	std::cout << "ms/frame = " << perf_counter_ms << "\t";
	std::cout << "Frames Per Second = " << fps << "\t";
	std::cout << "Cycles elapsed = " << cycle_elapsed_M << " MHz\t";
	std::cout << "Approximate clock-speed: " << clock_speed_approximate_GHz << " GHz\n";
	std::cout << "\n\n\n";
}

void Measure::inc_add()          { adds++; }
void Measure::inc_mult()         { mults++; }

void Measure::inc_buffer_write() { buffer_writes++; }
void Measure::inc_buffer_read()  { buffer_reads++; }

void Measure::inc_memory_write() { memory_writes++; }
void Measure::inc_memory_read() { memory_reads++; }

size_t Measure::get_memory_writes() { return memory_writes; }
size_t Measure::get_memory_reads()  { return memory_reads; }
size_t Measure::get_buffer_writes() { return buffer_writes; }
size_t Measure::get_buffer_reads()  { return buffer_reads; }
size_t Measure::get_adds()			{ return adds; }
size_t Measure::get_mults()			{ return mults; }