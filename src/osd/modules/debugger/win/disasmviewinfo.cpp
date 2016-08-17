// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  disasmviewinfo.c - Win32 debug window handling
//
//============================================================

#include "disasmviewinfo.h"


disasmview_info::disasmview_info(debugger_windows_interface &debugger, debugwin_info &owner, HWND parent) :
	debugview_info(debugger, owner, parent, DVT_DISASSEMBLY)
{
}


disasmview_info::~disasmview_info()
{
}


disasm_right_column disasmview_info::right_column() const
{
	return view<debug_view_disasm>()->right_column();
}


offs_t disasmview_info::selected_address() const
{
	return view<debug_view_disasm>()->selected_address();
}


void disasmview_info::set_expression(char const *string)
{
	view<debug_view_disasm>()->set_expression(string);
}


void disasmview_info::set_right_column(disasm_right_column contents)
{
	view<debug_view_disasm>()->set_right_column(contents);
}
