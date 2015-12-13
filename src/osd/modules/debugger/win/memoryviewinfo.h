// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  memoryviewinfo.h - Win32 debug window handling
//
//============================================================

#ifndef __DEBUG_WIN_MEMORY_VIEW_INFO_H__
#define __DEBUG_WIN_MEMORY_VIEW_INFO_H__

#include "debugwin.h"

#include "debugviewinfo.h"


class memoryview_info : public debugview_info
{
public:
	memoryview_info(debugger_windows_interface &debugger, debugwin_info &owner, HWND parent);
	virtual ~memoryview_info();

	UINT8 data_format() const;
	UINT32 chunks_per_row() const;
	bool reverse() const;
	bool ascii() const;
	bool physical() const;

	void set_expression(char const *string);
	void set_data_format(UINT8 dataformat);
	void set_chunks_per_row(UINT32 rowchunks);
	void set_reverse(bool reverse);
	void set_physical(bool physical);
};

#endif
