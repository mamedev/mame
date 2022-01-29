// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  windir.c - Win32 OSD core directory access functions
//
//============================================================

// standard windows headers
#include <windows.h>
#include <shlwapi.h>
#include <tchar.h>

// MAME headers
#include "osdfile.h"
#include "strformat.h"

// MAMEOS headers
#include "strconv.h"
#include "winutil.h"

// standard C headers
#include <cstdio>
#include <cctype>
#include <cassert>
#include <cstring>


namespace osd {
namespace {
//============================================================
//  TYPE DEFINITIONS
//============================================================

class win_directory : public directory
{
public:
	win_directory();
	virtual ~win_directory() override;

	virtual const entry *read() override;

	bool open_impl(std::string const &dirname);

private:
	HANDLE              m_find;                   // handle to the finder
	bool                m_is_first;               // true if this is the first entry
	entry               m_entry;                  // current entry's data
	WIN32_FIND_DATA     m_data;                   // current raw data
	std::string         m_name;                   // converted name of directory
};

//============================================================
//  win_directory::win_directory
//============================================================

win_directory::win_directory()
	: m_find(INVALID_HANDLE_VALUE)
	, m_is_first(true)
{
	m_entry.name = nullptr;
	std::memset(&m_data, 0, sizeof(m_data));
}


//============================================================
//  win_directory::~win_directory
//============================================================

win_directory::~win_directory()
{
	// free any data associated
	if (m_find != INVALID_HANDLE_VALUE)
		FindClose(m_find);
}


//============================================================
//  win_directory::read
//============================================================

const directory::entry *win_directory::read()
{
	// if this isn't the first file, do a find next
	if (!m_is_first)
	{
		if (!FindNextFile(m_find, &m_data))
			return nullptr;
	}
	m_is_first = false;

	// extract the data
	osd::text::from_tstring(m_name, m_data.cFileName);
	m_entry.name = m_name.c_str();
	m_entry.type = win_attributes_to_entry_type(m_data.dwFileAttributes);
	m_entry.size = m_data.nFileSizeLow | (std::uint64_t(m_data.nFileSizeHigh) << 32);
	m_entry.last_modified = win_time_point_from_filetime(&m_data.ftLastWriteTime);
	return (m_entry.name != nullptr) ? &m_entry : nullptr;
}


//============================================================
//  win_directory::open_impl
//============================================================

bool win_directory::open_impl(std::string const &dirname)
{
	assert(m_find == INVALID_HANDLE_VALUE);

	// append \*.* to the directory name
	std::string dirfilter = util::string_format("%s\\*.*", dirname);

	// convert the path to TCHARs
	osd::text::tstring t_dirfilter = osd::text::to_tstring(dirfilter);

	// attempt to find the first file
	m_find = FindFirstFileEx(t_dirfilter.c_str(), FindExInfoStandard, &m_data, FindExSearchNameMatch, nullptr, 0);
	return m_find != INVALID_HANDLE_VALUE;
}

} // anonymous namespace


//============================================================
//  osd::directory::open
//============================================================

directory::ptr directory::open(std::string const &dirname)
{
	// allocate memory to hold the osd_tool_directory structure
	std::unique_ptr<win_directory> dir;
	try { dir.reset(new win_directory()); }
	catch (...) { return nullptr; }

	if (!dir->open_impl(dirname))
		return nullptr;

	return ptr(std::move(dir));
}

} // namespace osd
