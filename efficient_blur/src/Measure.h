#pragma once
#include <windows.h>

class Measure
{
public:
	Measure();
	~Measure();

	void tic();
	void toc();

	void inc_add();
	void inc_mult();
	void inc_buffer_write();
	void inc_buffer_read();
	void inc_memory_read();
	void inc_memory_write();

	size_t get_memory_writes();
	size_t get_memory_reads();

	size_t get_buffer_writes();
	size_t get_buffer_reads();

	size_t get_adds();
	size_t get_mults();

private:
	size_t adds, mults;
	size_t buffer_writes, buffer_reads;
	size_t memory_writes, memory_reads;
	
	size_t num_tiles;

	//double FLOPs, FLOPS;  // quantity 
	//double MFLOPs, MFLOPS;// rate

	LARGE_INTEGER perf_counter_start{ 0 };
	LARGE_INTEGER perf_counter_stop{ 0 };
	double perf_counter_elapsed{ 0 };
	double perf_counter_freq{ 0 };
	double perf_counter_s{ 0 };  // seconds
	double perf_counter_ms{ 0 }; // milli-seconds
	double perf_counter_us{ 0 }; // micro-seconds
	double perf_counter_ns{ 0 }; // nano-seconds

	size_t fps{ 0 };

	__int64 cycle_counter_start{ 0 };
	__int64 cycle_counter_stop{ 0 };
	__int64 cycle_elapsed{ 0 };   // cycles
	double cycle_elapsed_M{ 0 }; // Mega-cycles
	double cycle_elapsed_G{ 0 }; // Giga-cycles

	double clock_speed_measured{ 0 };


	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//BOOL WINAPI QueryPerformanceFrequency(
	//				_Out_ LARGE_INTEGER *lpFrequency);
	// -Description:
	//		-Retrieves the frequency of the performance counter. 
	//		-The frequency of the performance counter is fixed at system 
	//		 boot and is consistent across all processors. 
	//		-Therefore, the frequency need only be queried upon application 
	//		 initialization, and the result can be cached.
	// -Parameters:
	//		-lpFrequency[out]:
	//			-A pointer to a variable that receives the current 
	//			 performance-counter frequency, in counts per second.
	// -Return value:
	//		-If the installed hardware supports a high-resolution 
	//		 performance counter, the return value is nonzero.
	//		-If the function fails, the return value is zero.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//BOOL WINAPI QueryPerformanceCounter(
	//				_Out_ LARGE_INTEGER *lpPerformanceCount);
	// -Description:
	//		-Retrieves the current value of the performance counter, 
	//		 which is a high-resolution (<1us) time stamp that can be 
	//		 used for time-interval measurements.
	// -Parameters:
	//		-lpPerformanceCount[out]:
	//			-A pointer to a variable that receives the current 
	//			 performance - counter value, in counts.
	// -Return value:
	//		!0 => function succeeds
	//		 0 => function fails
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//unsigned __int64 __rdtsc();
	// -Description:
	//		-Generates the rdtsc instruction, which returns 
	//		 the processor time stamp.
	//		-The processor time stamp records the number of clock 
	//		 cycles since the last reset.
	// -Intel documentation:
	//		-RDTSC—Read Time-Stamp Counter (page 4-543 in Intel Manual Vol. 2B)
	// 
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//clock_t clock(void);
	// -Description:
	//		-Calculates the wall-clock time used by the calling process.
	// -Parameters:
	//		-The elapsed time since the C Run-Time Libraries (CRT) 
	//		 initialization at the start of the process, 
	//		 measured in CLOCKS_PER_SEC units per second.
	//		-If the elapsed time is unavailable or has exceeded 
	//		 the maximum positive time that can be recorded as a 
	//		 clock_t type, the function returns the value(clock_t)(-1).
	// -Remarks:
	//		-The clock function tells how much wall-clock time has 
	//		 passed since the CRT initialization during process start. 
	//		-Note that this function does not strictly conform to ISO C, 
	//		 which specifies net CPU time as the return value. 
	//		-To obtain CPU times, use the Win32 GetProcessTimes function. 
	//		-To determine the elapsed time in seconds, divide the value 
	//		 returned by the clock function by the macro CLOCKS_PER_SEC.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//https://docs.microsoft.com/en-us/windows/desktop/SysInfo/acquiring-high-resolution-time-stamps
	//https://docs.microsoft.com/en-us/visualstudio/profiling/?view=vs-2017

	// Intel Processor Events Reference:
	// https://software.intel.com/en-us/vtune-amplifier-help-intel-processor-events-reference
};