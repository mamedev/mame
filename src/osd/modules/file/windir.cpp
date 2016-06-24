// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  windir.c - Win32 OSD core directory access functions
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#include <tchar.h>

// MAME headers
#include "osdcore.h"

// MAMEOS headers
#include "strconv.h"
#include "winutil.h"

// standard C headers
#include <stdio.h>
#include <ctype.h>
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

	virtual bool opendir(std::string const &dirname) override;

private:
	HANDLE              m_find;                   // handle to the finder
	bool                m_is_first;               // true if this is the first entry
	entry				m_entry;                  // current entry's data
	WIN32_FIND_DATA     m_data;                   // current raw data
};

//============================================================
//  win_directory::win_directory
//============================================================

win_directory::win_directory()
	: m_find(INVALID_HANDLE_VALUE)
	, m_is_first(true)
{
	std::memset(&m_data, 0, sizeof(m_data));
}


//============================================================
//  win_directory::~win_directory
//============================================================

win_directory::~win_directory()
{
	// free any data associated
	if (m_entry.name != nullptr)
		osd_free((void *)m_entry.name);
	if (m_find != INVALID_HANDLE_VALUE)
		FindClose(m_find);
}


//============================================================
//  win_directory::read
//============================================================

const directory::entry *win_directory::read()
{
	// if we've previously allocated a name, free it now
	if (m_entry.name != nullptr)
	{
		osd_free((void *)m_entry.name);
		m_entry.name = nullptr;
	}

	// if this isn't the first file, do a find next
	if (!m_is_first)
	{
		if (!FindNextFile(m_find, &m_data))
			return nullptr;
	}
	m_is_first = false;

	// extract the data
	m_entry.name = utf8_from_tstring(m_data.cFileName);
	m_entry.type = win_attributes_to_entry_type(m_data.dwFileAttributes);
	m_entry.size = m_data.nFileSizeLow | (std::uint64_t(m_data.nFileSizeHigh) << 32);
	m_entry.last_modified = win_time_point_from_filetime(&m_data.ftLastWriteTime);
	return (m_entry.name != nullptr) ? &m_entry : nullptr;
}


//============================================================
//  win_directory::opendir
//============================================================

bool win_directory::opendir(std::string const &dirname)
{
	assert(m_find == INVALID_HANDLE_VALUE);

	// convert the path to TCHARs
	std::unique_ptr<TCHAR, void (*)(void *)> const t_dirname(tstring_from_utf8(dirname.c_str()), &osd_free);
	if (!t_dirname)
		return false;

	// append \*.* to the directory name
	auto const dirfilter_size = _tcslen(t_dirname.get()) + 5;
	std::unique_ptr<TCHAR []> dirfilter;
	try { dirfilter = std::make_unique<TCHAR[]>(dirfilter_size); }
	catch (...) { return false; }
	_sntprintf(dirfilter.get(), dirfilter_size, TEXT("%s\\*.*"), t_dirname.get());

	// attempt to find the first file
	m_find = FindFirstFileEx(dirfilter.get(), FindExInfoStandard, &m_data, FindExSearchNameMatch, nullptr, 0);
	if (m_find == INVALID_HANDLE_VALUE)
		return false;

	return true;
}

} // anonymous namespace


//============================================================
//  osd::directory::open
//============================================================

directory::ptr directory::open(std::string const &dirname)
{
	// allocate memory to hold the osd_tool_directory structure
	ptr dir;
	try { dir = std::make_unique<win_directory>(); }
	catch (...) { return nullptr; }

	if (!dir->opendir(dirname))
		return false;

	return dir;
}

} // namesapce osd
