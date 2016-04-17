// license:BSD-3-Clause
// copyright-holders:Brad Hughes
//====================================================================
//
//  none.cpp - Null implementation of diagnostic module
//
//====================================================================

#include "diagnostics_module.h"

class diagnostics_none : public diagnostics_module
{
public:
	diagnostics_none()
	{
	}

	int init_crash_diagnostics() override
	{
		return 0;
	}

	void start_profiler(std::uint32_t max_seconds, std::uint8_t stack_depth) override
	{
	}

	void stop_profiler() override
	{
	}

	void print_profiler_results() override
	{
	}
};

// Determine if diagnostics_none should be used based on OSD
#if defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)
#include <winapifamily.h>
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define USE_DIAG_NONE 0 // Desktop Windows
#else
#define USE_DIAG_NONE 1 // Universal Windows
#endif
#else
#define USE_DIAG_NONE 1 // SDL and others
#endif

// We we should use diagnostics_none, create the static accessor
#if USE_DIAG_NONE
diagnostics_module* diagnostics_module::get_instance()
{
	static diagnostics_none s_instance;
	return &s_instance;
}
#endif