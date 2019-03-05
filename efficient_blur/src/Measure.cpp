#include "Measure.h"

Measure::Measure()
	: adds(0), mults(0), 
	buffer_writes(0), buffer_reads(0),
	memory_writes(0), memory_reads(0)
{}

Measure::~Measure()
{}

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