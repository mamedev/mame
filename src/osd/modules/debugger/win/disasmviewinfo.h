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


namespace osd::debugger::win {

class disasmview_info : public debugview_info
{
public:
	disasmview_info(debugger_windows_interface &debugger, debugwin_info &owner, HWND parent, debug_view_type type = DVT_DISASSEMBLY);
	virtual ~disasmview_info();

	char const *expression() const;
	disasm_right_column right_column() const;
	virtual std::optional<offs_t> selected_address() const;

	void set_expression(const std::string &expression);
	void set_right_column(disasm_right_column contents);

	virtual void restore_configuration_from_node(util::xml::data_node const &node) override;
	virtual void save_configuration_to_node(util::xml::data_node &node) override;
};

} // namespace osd::debugger::win

#endif // MAME_DEBUGGER_WIN_DISASMVIEWINFO_H
