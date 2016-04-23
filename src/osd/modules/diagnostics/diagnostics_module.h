// license:BSD-3-Clause
// copyright-holders:Brad Hughes
/***************************************************************************

diagnostics_module.h

Diagnostics module interface

***************************************************************************/
#ifndef DIAGNOSTICS_MODULE_H_
#define DIAGNOSTICS_MODULE_H_

#include "osdepend.h"

//============================================================
//  CONSTANTS
//============================================================

class diagnostics_module
{
public:
	diagnostics_module() { }

	virtual ~diagnostics_module() { }

	// initializes crash diagnostics for MAME
	virtual int init_crash_diagnostics() = 0;

	// starts the profiler
	virtual void start_profiler(std::uint32_t max_seconds, std::uint8_t stack_depth) = 0;
	
	// stops the currently active profiler
	virtual void stop_profiler() = 0;

	// prints the results of the profiling operation
	virtual void print_profiler_results() = 0;

	// Gets the instance of the diagnostic module
	static diagnostics_module* get_instance();
};

#endif