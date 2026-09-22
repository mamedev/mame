// license:BSD-3-Clause
// copyright-holders:David Broman
/***************************************************************************

    ioprocs.cpp

    File indexed by line number

***************************************************************************/

#include "line_idx_file.h"

#include <corefile.h>

namespace util {

//-------------------------------------------------
// line_indexed_file - constructor
//-------------------------------------------------

line_indexed_file::line_indexed_file() :
	m_data(),
	m_line_starts()
{
}


//-------------------------------------------------
// open - Reads full contents of text file,
// and initializes line index
//-------------------------------------------------

std::error_condition line_indexed_file::open(const char * file_path, int spaces_per_tab)
{
	m_data.resize(0);
	m_line_starts.resize(0);
	std::error_condition err = util::core_file::load(file_path, m_data);
	if (err)
	{
		return err;
	}

	unsigned int cur_line_start = 0;
    for (unsigned int i = 0; i < m_data.size() - 1; i++)                 // Ignore final char, enable [i+1] in body
	{
		// Replace tabs with spaces for more consistent alignment
		if (m_data[i] == '\t')
		{
            unsigned int col = i - cur_line_start;
            int num_spaces_until_next_tab_stop = spaces_per_tab - (col % spaces_per_tab);
            m_data[i] = ' ';									// Tab char -> first space
			for (int j = 0; j < num_spaces_until_next_tab_stop - 1; j++)
			{
                m_data.insert(m_data.cbegin() + i, ' ');		// Insert remaining spaces
			}
            i += num_spaces_until_next_tab_stop - 1;			// Skip over inserted spaces
			continue;
		}
		
		// Check for line endings
		bool crlf = (m_data[i] == '\r' && m_data[i+1] == '\n');
		bool line_end = crlf || (m_data[i] == '\n');
		if (!line_end)
		{
			continue;
		}

		m_data[i] = '\0';                                       // Terminate line
		m_line_starts.push_back(cur_line_start);                // Record line's starting index
		if (crlf)
		{
			i++;                                                // Skip \n in \r\n
		}
		cur_line_start = i+1;                                   // Prepare for next line
	}

	m_line_starts.push_back(cur_line_start);
	m_data.push_back('\0');
	return std::error_condition();
}

} // namespace util
