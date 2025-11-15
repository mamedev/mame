// license:BSD-3-Clause
// copyright-holders:David Broman
/*********************************************************************

    srcdbg_format_writer.h

    Internal implementation of srcdbg_api.h.

    WARNING: Tools external to MAME should only use functionality
    declared in srcdg_format.h and srcdbg_api.h.

***************************************************************************/

#include "srcdbg_api.h"
#include "srcdbg_format_writer.h"
#include "srcdbg_format.h"

#include <stdlib.h>

// Wrapper to ensure exceptions don't escape into this library's client

template<typename Func>
static int exception_boundary(Func && f)
{
	try
	{
		return f();
	}
	catch(const std::bad_alloc &)
	{
		return MAME_SRCDBG_E_OUTOFMEMORY;
	}
	catch(const std::exception &)
	{
		return MAME_SRCDBG_E_EXCEPTION;
	}

	// unreachable
}

#define BEGIN_EXCEPTION_BOUNDARY return exception_boundary([&]() -> int {
#define END_EXCEPTION_BOUNDARY });

extern const char build_version[];

// ------------------------------------------------------------------
// Public API - C Interface for assemblers / compilers.  These
// are simple wrappers around public member functions from
// srcdbg_simple_generator
// ------------------------------------------------------------------

LIB_PUBLIC int mame_srcdbg_get_version_info(int * srcdbg_lib_major, int * srcdbg_lib_minor, const char ** mame_build)
{
	BEGIN_EXCEPTION_BOUNDARY
	*srcdbg_lib_major = SRCDBG_LIB_MAJOR;
	*srcdbg_lib_minor = SRCDBG_LIB_MINOR;
	*mame_build = build_version;
	return MAME_SRCDBG_E_SUCCESS;
	END_EXCEPTION_BOUNDARY
}

LIB_PUBLIC int mame_srcdbg_simp_open_new(const char * file_path, void ** handle_ptr)
{
	BEGIN_EXCEPTION_BOUNDARY
	srcdbg_simple_generator * generator = new srcdbg_simple_generator;
	RET_IF_FAIL(generator->open(file_path));
	*handle_ptr = (void *) generator;
	return MAME_SRCDBG_E_SUCCESS;
	END_EXCEPTION_BOUNDARY
}

LIB_PUBLIC int mame_srcdbg_simp_add_source_file_path(void * srcdbg_simp_state, const char * source_file_path, unsigned int * index_ptr)
{
	BEGIN_EXCEPTION_BOUNDARY
	return ((srcdbg_simple_generator *) srcdbg_simp_state)->add_source_file_path(source_file_path, *index_ptr);
	END_EXCEPTION_BOUNDARY
}

LIB_PUBLIC int mame_srcdbg_simp_add_line_mapping(void * srcdbg_simp_state, unsigned short address_first, unsigned short address_last, unsigned int source_file_index, unsigned int line_number)
{
	BEGIN_EXCEPTION_BOUNDARY
	return ((srcdbg_simple_generator *) srcdbg_simp_state)->add_line_mapping(address_first, address_last, source_file_index, line_number);
	END_EXCEPTION_BOUNDARY
}

LIB_PUBLIC int mame_srcdbg_simp_add_global_fixed_symbol(void * srcdbg_simp_state, const char * symbol_name, int symbol_value, unsigned int symbol_flags)
{
	BEGIN_EXCEPTION_BOUNDARY
	return ((srcdbg_simple_generator *) srcdbg_simp_state)->add_global_fixed_symbol(symbol_name, symbol_value, symbol_flags);
	END_EXCEPTION_BOUNDARY
}

LIB_PUBLIC int mame_srcdbg_simp_add_local_fixed_symbol(void * srcdbg_simp_state, const char * symbol_name, unsigned short address_first, unsigned short address_last, int symbol_value)
{
	BEGIN_EXCEPTION_BOUNDARY
	return ((srcdbg_simple_generator *) srcdbg_simp_state)->add_local_fixed_symbol(symbol_name, address_first, address_last, symbol_value);
	END_EXCEPTION_BOUNDARY
}

LIB_PUBLIC int mame_srcdbg_simp_add_local_relative_symbol(void * srcdbg_simp_state, const char * symbol_name, unsigned short address_first, unsigned short address_last, unsigned char reg, int reg_offset)
{
	BEGIN_EXCEPTION_BOUNDARY
	return ((srcdbg_simple_generator *) srcdbg_simp_state)->add_local_relative_symbol(symbol_name, address_first, address_last, reg, reg_offset);
	END_EXCEPTION_BOUNDARY
}

LIB_PUBLIC int mame_srcdbg_simp_import(void * srcdbg_simp_state, const char * srcdbg_file_path_to_import, short offset, char * error_details, unsigned int num_bytes_error_details)
{
	BEGIN_EXCEPTION_BOUNDARY
	return ((srcdbg_simple_generator *) srcdbg_simp_state)->import(srcdbg_file_path_to_import, offset, error_details, num_bytes_error_details);
	END_EXCEPTION_BOUNDARY
}

LIB_PUBLIC int mame_srcdbg_simp_close(void * srcdbg_simp_state)
{
	BEGIN_EXCEPTION_BOUNDARY
	srcdbg_simple_generator * generator = (srcdbg_simple_generator *) srcdbg_simp_state;
	RET_IF_FAIL(generator->close());
	delete generator;
	return MAME_SRCDBG_E_SUCCESS;
	END_EXCEPTION_BOUNDARY
}
