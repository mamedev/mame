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

// view_info class for managing source-code-level debugging.  Shares
// code with disasmview_info, especially for breakpoint handling,
// but with some customizations
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
	void populate_source_file_combo();

	// Non-owning reference to the source-code-file-selector combobox.
	// Used to update its selection whenever the PC changes
	HWND    m_combownd;
};

} // namespace osd::debugger::win

#endif // MAME_DEBUGGER_WIN_SOURCEVIEWINFO_H
