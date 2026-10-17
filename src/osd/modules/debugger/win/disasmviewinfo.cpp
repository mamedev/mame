// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  disasmviewinfo.cpp - Win32 debug window handling
//
//============================================================

#include "emu.h"
#include "disasmviewinfo.h"

#include "util/xmlfile.h"


namespace osd::debugger::win {

disasmview_info::disasmview_info(debugger_windows_interface &debugger, debugwin_info &owner, HWND parent, debug_view_type type /* = DVT_DISASSEMBLY */) :
	debugview_info(debugger, owner, parent, type)
{
}


disasmview_info::~disasmview_info()
{
}


char const *disasmview_info::expression() const
{
	return view<debug_view_disasm>()->expression();
}


disasm_right_column disasmview_info::right_column() const
{
	return view<debug_view_disasm>()->right_column();
}

std::optional<offs_t> disasmview_info::selected_address() const
{
	return std::optional<offs_t>(view<debug_view_disasm>()->selected_address());
}


void disasmview_info::set_expression(const std::string &string)
{
	view<debug_view_disasm>()->set_expression(string);
}


void disasmview_info::set_right_column(disasm_right_column contents)
{
	view<debug_view_disasm>()->set_right_column(contents);
}


void disasmview_info::restore_configuration_from_node(util::xml::data_node const &node)
{
	debug_view_disasm &dasmview(*view<debug_view_disasm>());
	dasmview.set_right_column(disasm_right_column(node.get_attribute_int(ATTR_WINDOW_DISASSEMBLY_RIGHT_COLUMN, dasmview.right_column())));

	debugview_info::restore_configuration_from_node(node);
}


void disasmview_info::save_configuration_to_node(util::xml::data_node &node)
{
	debugview_info::save_configuration_to_node(node);

	debug_view_disasm &dasmview(*view<debug_view_disasm>());
	node.set_attribute_int(ATTR_WINDOW_DISASSEMBLY_RIGHT_COLUMN, dasmview.right_column());
}

} // namespace osd::debugger::win
