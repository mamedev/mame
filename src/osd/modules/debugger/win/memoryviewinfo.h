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

#include "debug/dvmemory.h"

#include <string>


namespace osd::debugger::win {

class memoryview_info : public debugview_info
{
public:
	memoryview_info(debugger_windows_interface &debugger, debugwin_info &owner, HWND parent);
	virtual ~memoryview_info();

	char const *expression() const;
	debug_view_memory::data_format data_format() const;
	uint32_t chunks_per_row() const;
	bool reverse() const;
	bool physical() const;
	int address_radix() const;

	void set_expression(const std::string &string);
	void set_data_format(debug_view_memory::data_format dataformat);
	void set_chunks_per_row(uint32_t rowchunks);
	void set_reverse(bool reverse);
	void set_physical(bool physical);
	void set_address_radix(int radix);

	virtual void restore_configuration_from_node(util::xml::data_node const &node) override;
	virtual void save_configuration_to_node(util::xml::data_node &node) override;

protected:
	enum
	{
		ID_CONTEXT_LAST_PC = 101
	};

	virtual void add_items_to_context_menu(HMENU menu) override;
	virtual void update_context_menu(HMENU menu) override;
	virtual void handle_context_menu(unsigned command) override;

private:
	std::string m_lastpc;
};

} // namespace osd::debugger::win

#endif // MAME_DEBUGGER_WIN_MEMORYVIEWINFO_H
