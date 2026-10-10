// license:BSD-3-Clause
// copyright-holders:David Broman
//============================================================
//
//  sourceviewinfo.cpp - Win32 debug window handling
//
//============================================================

#include "emu.h"
#include "sourceviewinfo.h"
#include "uimetrics.h"

#include "debug/dvsourcecode.h"
#include "debug/srcdbg_info.h"

#include "strconv.h"

#include "winutil.h"

namespace osd::debugger::win {

// combo box styles
#define COMBO_BOX_STYLE     WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL
#define COMBO_BOX_STYLE_EX  0


sourceview_info::sourceview_info(debugger_windows_interface &debugger, debugwin_info &owner, HWND parent) :
	disasmview_info(debugger, owner, parent, DVT_SOURCE)
	, m_combownd(nullptr)
{
}


void sourceview_info::set_src_index(u16 new_src_index)
{
	view<debug_view_sourcecode>()->set_src_index(new_src_index);
}


u32 sourceview_info::cur_src_index()
{
	return view<debug_view_sourcecode>()->cur_src_index();
}


std::optional<offs_t> sourceview_info::selected_address() const
{
	return view<debug_view_sourcecode>()->selected_address();
}


HWND sourceview_info::create_source_file_combobox(HWND parent, LONG_PTR userdata)
{
	const debug_view_sourcecode * dv_source = view<debug_view_sourcecode>();
	const srcdbg_info * debug_info = dv_source->get_srcdbg_info();

	// create a combo box
	HWND const result = CreateWindowEx(COMBO_BOX_STYLE_EX, TEXT("COMBOBOX"), nullptr, COMBO_BOX_STYLE,
			0, 0, 100, 1000, parent, nullptr, GetModuleHandleUni(), nullptr);
	SetWindowLongPtr(result, GWLP_USERDATA, userdata);
	SendMessage(result, WM_SETFONT, (WPARAM) metrics().debug_font(), (LPARAM)FALSE);

	if (debug_info == nullptr)
	{
		// hide combobox when source-level debugging is off
		smart_show_window(result, false);
	}

	m_combownd = result;			// Cache a non-owning reference to update as PC changes
	return result;
}

void sourceview_info::populate_source_file_combo()
{
	// populate the combobox with source file paths when present
	SendMessage(m_combownd, CB_RESETCONTENT, 0, 0);
	const debug_view_sourcecode * dv_source = view<debug_view_sourcecode>();
	const srcdbg_info * debug_info = dv_source->get_srcdbg_info();
	if (debug_info == nullptr)
	{
		return;
	}

	size_t maxlength = 0;
	std::size_t num_files = debug_info->num_files();
	u32 cur_sel_idx = 0;
	for (std::size_t i = 0; i < num_files; i++)
	{
		const srcdbg_provider_base::source_file_path * path;
		if (!debug_info->file_index_to_path(i, &path))
		{
			continue;
		}
		const char * entry_text = path->built();
		size_t const length = strlen(entry_text);
		if (length > maxlength)
		{
			maxlength = length;
		}
		auto t_name = osd::text::to_tstring(entry_text);
		LRESULT combo_idx = SendMessage(m_combownd, CB_ADDSTRING, 0, (LPARAM) t_name.c_str());
		SendMessage(m_combownd, CB_SETITEMDATA, (WPARAM) combo_idx, (LPARAM) i);
		if (dv_source->cur_src_index() == i)
		{
			cur_sel_idx = u32(combo_idx);
		}
	}
	SendMessage(m_combownd, CB_SETDROPPEDWIDTH, ((maxlength + 2) * metrics().debug_font_width()) + metrics().vscroll_width(), 0);
	SendMessage(m_combownd, CB_SETCURSEL, (WPARAM) cur_sel_idx, 0);
}


// Overriding update so source-file combo box can auto-select the new
// file that the PC has stepped into view
void sourceview_info::update()
{
	disasmview_info::update();
	debug_view_sourcecode * dv_source = view<debug_view_sourcecode>();
	if (dv_source->update_gui_needs_full_refresh())
	{
		populate_source_file_combo();
		return;
	}
	
	// Current source-code-file can change just by stepping.  Update
	// combobox selection to match whatever the current source-code-file is now.
	LRESULT num_items = SendMessage(m_combownd, CB_GETCOUNT, 0, 0);
	for (u32 combo_idx = 0; combo_idx < num_items; combo_idx++)
	{
		LRESULT item_data = SendMessage(m_combownd, CB_GETITEMDATA, combo_idx, 0);
		if (item_data == dv_source->cur_src_index())
		{
			SendMessage(m_combownd, CB_SETCURSEL, combo_idx, 0);
			return;
		}
	}
}


} // namespace osd::debugger::win
