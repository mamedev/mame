// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  memoryviewinfo.h - Win32 debug window handling
//
//============================================================
#ifndef MAME_DEBUGGER_WIN_MEMORYVIEWINFO_H
#define MAME_DEBUGGER_WIN_MEMORYVIEWINFO_H

#pragma once

#include "debugwin.h"

#include "debugviewinfo.h"


class memoryview_info : public debugview_info
{
public:
	memoryview_info(debugger_windows_interface &debugger, debugwin_info &owner, HWND parent);
	virtual ~memoryview_info();

	uint8_t data_format() const;
	uint32_t chunks_per_row() const;
	bool reverse() const;
	bool physical() const;

	void set_expression(const std::string &string);
	void set_data_format(uint8_t dataformat);
	void set_chunks_per_row(uint32_t rowchunks);
	void set_reverse(bool reverse);
	void set_physical(bool physical);
};

#endif
