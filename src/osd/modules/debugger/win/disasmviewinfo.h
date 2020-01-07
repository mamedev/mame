// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  disasmviewinfo.h - Win32 debug window handling
//
//============================================================
#ifndef MAME_DEBUGGER_WIN_DISASMVIEWINFO_H
#define MAME_DEBUGGER_WIN_DISASMVIEWINFO_H

#pragma once

#include "debugwin.h"

#include "debugviewinfo.h"

#include "debug/dvdisasm.h"


class disasmview_info : public debugview_info
{
public:
	disasmview_info(debugger_windows_interface &debugger, debugwin_info &owner, HWND parent);
	virtual ~disasmview_info();

	disasm_right_column right_column() const;
	offs_t selected_address() const;

	void set_expression(const std::string &expression);
	void set_right_column(disasm_right_column contents);
};

#endif
