// license:BSD-3-Clause
// copyright-holders:David Broman
/***************************************************************************

    line_idx_file.h

    File indexed by line number

***************************************************************************/
#ifndef MAME_LIB_UTIL_LINE_IDX_FILE_H
#define MAME_LIB_UTIL_LINE_IDX_FILE_H

#pragma once

#include "utilfwd.h"

#include <cstdint>
#include <system_error>
#include <vector>


namespace util {

// Encapsulates the contents of a text file, indexed by line number.
class line_indexed_file
{
public:
	line_indexed_file();
	~line_indexed_file() { };
	std::error_condition open(const char * file_path, int spaces_per_tab);
	int num_lines() const { return m_line_starts.size(); };
	const char * get_line_text(unsigned int n) const { return (const char *) &m_data[m_line_starts[n-1]]; };

private:
	std::vector<uint8_t> m_data;
	std::vector<unsigned int> m_line_starts;
};

} // namespace util

#endif // MAME_LIB_UTIL_LINE_IDX_FILE_H
