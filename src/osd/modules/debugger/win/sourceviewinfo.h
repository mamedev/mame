// license:BSD-3-Clause
// copyright-holders:David Broman
//============================================================
//
//  sourceviewinfo.h - Win32 debug window handling
//
//============================================================
#ifndef MAME_DEBUGGER_WIN_SOURCEVIEWINFO_H
#define MAME_DEBUGGER_WIN_SOURCEVIEWINFO_H

#pragma once

#include "disasmviewinfo.h"


namespace osd::debugger::win {

// Manages GUI half of source code view.  Shares code with disasmview_info, especially
// for breakpoint handling, but with some customizations
class sourceview_info : public disasmview_info
{
public:
	sourceview_info(debugger_windows_interface &debugger, debugwin_info &owner, HWND parent);
	virtual ~sourceview_info() {}
	HWND create_source_file_combobox(HWND parent, LONG_PTR userdata);

	virtual std::optional<offs_t> selected_address() const override;

	// Helpers to access portions of debug_view_sourcecode
	void set_src_index(u16 new_src_index);
	u32 cur_src_index();

protected:
	virtual void update() override;

private:
	// TODO: Keeping my own copy of this HWND and making update() virtual seems
	// inconsistent with rest of dbg arch.  Is there a more appropriate way to
	// update its selection whenever the PC changes?
	HWND    m_combownd;             // Selects from list of source files
};

} // namespace osd::debugger::win

#endif // MAME_DEBUGGER_WIN_SOURCEVIEWINFO_H
