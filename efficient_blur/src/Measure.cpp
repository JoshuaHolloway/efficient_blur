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
	process_time_start = clock();
}
void Measure::toc_internal()
{
	process_time_stop = clock();
	QueryPerformanceCounter(&perf_counter_stop);
	cycle_counter_stop = __rdtsc();
}

#include <iostream>
#include <iomanip>
double Measure::toc()
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

	toc_internal();

	// Process time elapsed
	process_time_elapsed_ms = (double)process_time_stop - (double)process_time_start; // apparently ms.

	// We now have the elapsed number of ticks, along with the
	// number of ticks-per-second. We use these values
	// to convert to the number of elapsed microseconds.
	perf_counter_elapsed
		= (double)perf_counter_stop.QuadPart
		- (double)perf_counter_start.QuadPart;
	perf_counter_s =         perf_counter_elapsed / perf_counter_freq;
	perf_counter_ms = 1.e3 * perf_counter_elapsed / perf_counter_freq;
	perf_counter_us = 1.e6 * perf_counter_elapsed / perf_counter_freq;
	perf_counter_ns = 1.e9 * perf_counter_elapsed / perf_counter_freq;

	fps = 1. / perf_counter_s; // 1 / [s./frame]

	
	cycle_elapsed = cycle_counter_stop - cycle_counter_start; // [cycles / frame]
	cycle_elapsed_M = (double)cycle_elapsed / 1.e6;           // [Mcycles / frame]
	cycle_elapsed_G = (double)cycle_elapsed / 1.e9;           // [Gcycles / frame]

	//  ( Frames / s. ) x ( Cycles / frame. ) = Clock Speed [cycles / s.]
	double clock_speed_approximate_GHz = (double)fps * cycle_elapsed_G; // [Gcycles / s.] = GHz
	// Note: clock_speed_measured is measured more accurately

	// - - - - - - - - - - - - - - - - - - - - - - 
	// Bandwidth
	total_writes = memory_writes + buffer_writes;
	total_reads = memory_reads + buffer_reads;

	double total_bytes_written = (double)total_writes * (double)elem_size;
	double total_bytes_read = (double)total_reads * (double)elem_size;

	read_bandwidth_Bps = total_bytes_read / perf_counter_s;
	write_bandwidth_Bps = total_bytes_written / perf_counter_s;

	// - - - - - - - - - - - - - - - - - - - - - - 
	// FLOPS
	FLOPs = adds; // total # of adds + mults [not per second!] - NOTE: Will add in mults when kernel is added
	FLOPS = FLOPs / perf_counter_s; // [Floating-point ops per second]

	using std::cout;
	using std::endl;
	cout << "\n\nINSIDE toc():\n";
	cout << std::fixed << std::setprecision(2);
	cout << "ms/frame = " << perf_counter_ms << "\t";
	cout << "Frames Per Second = " << fps << "\t";
	cout << "Cycles elapsed = " << cycle_elapsed_M << " MHz\t";
	cout << "Approximate clock-speed: " << clock_speed_approximate_GHz << " GHz";


	cout << endl;
	cout << "Process time elapsed: " << process_time_elapsed_ms << " ms.\t";
	cout << "Number of Memory Reads: " << total_reads << "\n";
	cout << "Number of Memory Writes: " << total_writes << "\n";
	cout << "Memory Read: " << total_bytes_read << " B.\n";
	cout << "Memory Written: " << total_bytes_written << " B.\n";
	cout << "Read Bandwidth " << read_bandwidth_Bps / 1.e6 << " MBps\n";
	cout << "Write Bandwidth " << write_bandwidth_Bps / 1.e6 << " MBps\n";
	cout << "FLOPs: " << FLOPs / 1.e6 << " MFLOP\n";
	cout << "FLOPS: " << FLOPS / 1.e6 << " MFLOPS\n";


	std::cout << "\n\n\n";

	return perf_counter_s;
}

double Measure::measure_clock_frequency()
{
	tic();
	int ms = 1000;
	Sleep(ms);
	toc();

	clock_speed_measured_Hz = cycle_elapsed;
	clock_speed_measured_Mhz = cycle_elapsed;
	clock_speed_measured_Ghz = cycle_elapsed;

	std::cout << std::fixed << std::setprecision(2);
	std::cout << "Measured clock-frequency:  " << clock_speed_measured_Ghz << " GHz.\n";

	// Return clock frequency estimate in MHz
	return clock_speed_measured_Hz;
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