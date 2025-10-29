// license:BSD-3-Clause
// copyright-holders:David Broman
/*********************************************************************

    srcdbg_format_writer.h

    Internal implementation of portions of srcdbg_api.h.

    WARNING: Tools external to MAME should only use functionality
    declared in srcdg_format.h and srcdbg_api.h.

***************************************************************************/

#ifndef MAME_SRCDBG_FORMAT_WRITER_H
#define MAME_SRCDBG_FORMAT_WRITER_H

#pragma once

#include "srcdbg_format.h"

#include <stdio.h>
#include <string>
#include <vector>


#define RET_IF_FAIL(expr)                              \
	{                                                  \
		int _err = (expr);                             \
		if (_err != MAME_SRCDBG_E_SUCCESS)             \
			return _err;                               \
	}

#define FWRITE_OR_RETURN(BUF, SIZE, COUNT, STREAM)     \
	{                                                  \
		int _ret = fwrite(BUF, SIZE, COUNT, STREAM);   \
		if (_ret < COUNT)                              \
			return MAME_SRCDBG_E_FWRITE_ERROR;         \
	}

#define FCLOSE_OR_RETURN(STREAM)                       \
{                                                      \
	if (fclose(STREAM) != 0)                           \
		return MAME_SRCDBG_E_FCLOSE_ERROR;             \
}


// Interim storage of local fixed variables
class local_fixed : public local_fixed_symbol_value
{
public:
	local_fixed() : m_ranges() {}
	std::vector<address_range> m_ranges;
};


// Interim storage of local relative variables
struct local_relative : local_relative_symbol_value
{
	local_relative() : m_values() {}
	std::vector<local_relative_eval_rule> m_values;
};


// Class providing implementation of source-level debugging information file writer API
class srcdbg_simple_generator
{
public:
	srcdbg_simple_generator();
	~srcdbg_simple_generator();
	int open(const char * file_path);
	int add_source_file_path(const char * source_file_path, unsigned int & index);
	int add_line_mapping(unsigned short address_first, unsigned short address_last, unsigned int source_file_index, unsigned int line_number);
	int add_global_fixed_symbol(const char * symbol_name, int symbol_value, unsigned int symbol_flags);
	int add_local_fixed_symbol(const char * symbol_name, unsigned short address_first, unsigned short address_last, int symbol_value);
	int add_local_relative_symbol(const char * symbol_name, unsigned short address_first, unsigned short address_last, unsigned char reg, int reg_offset);
	int import(const char * srcdbg_file_path_to_import, short offset, char * error_details, unsigned int num_bytes_error_details);
	int close();

private:
	FILE *                                 m_output;
	mame_debug_info_simple_header          m_header;
	std::vector<std::string>               m_source_file_paths;
	std::vector<srcdbg_line_mapping>       m_line_mappings;
	std::vector<std::string>               m_symbol_names;
	std::vector<global_fixed_symbol_value> m_global_fixed_symbol_values;
	std::vector<local_fixed>               m_local_fixed_symbol_values;
	std::vector<local_relative>            m_local_relative_symbol_values;
};


#endif /* MAME_SRCDBG_FORMAT_WRITER_H */
