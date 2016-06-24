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

// standard C headers
#include <stdio.h>
#include <ctype.h>

// MAME headers
#include "osdcore.h"

// MAMEOS headers
#include "strconv.h"
#include "winutil.h"


//============================================================
//  TYPE DEFINITIONS
//============================================================

namespace
{
	class win_directory : public osd::directory
	{
	public:
		win_directory();
		~win_directory();

		virtual const entry *read() override;

		HANDLE              m_find;                   // handle to the finder
		bool                m_is_first;               // true if this is the first entry
		entry				m_entry;                  // current entry's data
		WIN32_FIND_DATA     m_data;                   // current raw data
	};
};


//============================================================
//  osd::directory::open
//============================================================

osd::directory *osd::directory::open(const char *dirname)
{
	win_directory *dir = nullptr;
	TCHAR *t_dirname = nullptr;
	TCHAR *dirfilter = nullptr;
	size_t dirfilter_size;

	// allocate memory to hold the osd_tool_directory structure
	dir = new win_directory();
	if (dir == nullptr)
		goto error;

	// convert the path to TCHARs
	t_dirname = tstring_from_utf8(dirname);
	if (t_dirname == nullptr)
		goto error;

	// append \*.* to the directory name
	dirfilter_size = _tcslen(t_dirname) + 5;
	dirfilter = (TCHAR *)malloc(dirfilter_size * sizeof(*dirfilter));
	if (dirfilter == nullptr)
		goto error;
	_sntprintf(dirfilter, dirfilter_size, TEXT("%s\\*.*"), t_dirname);

	// attempt to find the first file
	dir->m_find = FindFirstFileEx(dirfilter, FindExInfoStandard, &dir->m_data, FindExSearchNameMatch, nullptr, 0);

error:
	// cleanup
	if (t_dirname != nullptr)
		osd_free(t_dirname);
	if (dirfilter != nullptr)
		free(dirfilter);
	if (dir != nullptr && dir->m_find == INVALID_HANDLE_VALUE)
	{
		delete dir;
		dir = nullptr;
	}
	return dir;
}


//============================================================
//  win_directory::win_directory
//============================================================

win_directory::win_directory()
	: m_find(INVALID_HANDLE_VALUE), m_is_first(true)
{
	memset(&m_entry, 0, sizeof(m_entry));
	memset(&m_data, 0, sizeof(m_data));
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

const osd::directory::entry *win_directory::read()
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
	m_entry.size = m_data.nFileSizeLow | ((UINT64) m_data.nFileSizeHigh << 32);
	m_entry.last_modified = win_time_point_from_filetime(&m_data.ftLastWriteTime);
	return (m_entry.name != nullptr) ? &m_entry : nullptr;
}
