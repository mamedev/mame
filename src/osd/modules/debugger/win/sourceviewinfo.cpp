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

#include "strconv.h"

#include "winutil.h"

namespace osd::debugger::win {

// combo box styles
#define COMBO_BOX_STYLE     WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL
#define COMBO_BOX_STYLE_EX  0


sourceview_info::sourceview_info(debugger_windows_interface &debugger, debugwin_info &owner, HWND parent) :
	disasmview_info(debugger, owner, parent, true /* source_code_debugging */)
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
	const srcdbg_provider_base * debug_info = dv_source->srcdbg_provider();

	// create a combo box
	HWND const result = CreateWindowEx(COMBO_BOX_STYLE_EX, TEXT("COMBOBOX"), nullptr, COMBO_BOX_STYLE,
			0, 0, 100, 1000, parent, nullptr, GetModuleHandleUni(), nullptr);
	SetWindowLongPtr(result, GWLP_USERDATA, userdata);
	SendMessage(result, WM_SETFONT, (WPARAM)metrics().debug_font(), (LPARAM)FALSE);

	if (debug_info == nullptr)
	{
		// hide combobox when source-level debugging is off
		smart_show_window(result, false);
	}
	else
	{
		// populate the combobox with source file paths when present
		size_t maxlength = 0;
		std::size_t num_files = debug_info->num_files();
		for (std::size_t i = 0; i < num_files; i++)
		{
			const char * entry_text = debug_info->file_index_to_path(i).built();
			size_t const length = strlen(entry_text);
			if (length > maxlength)
			{
				maxlength = length;
			}
			auto t_name = osd::text::to_tstring(entry_text);
			SendMessage(result, CB_ADDSTRING, 0, (LPARAM) t_name.c_str());
		}

		SendMessage(result, CB_SETCURSEL, dv_source->cur_src_index(), 0);
		SendMessage(result, CB_SETDROPPEDWIDTH, ((maxlength + 2) * metrics().debug_font_width()) + metrics().vscroll_width(), 0);
	}

	m_combownd = result;
	return result;
}

// Overriding update so source-file combo box can auto-select the new
// file that the PC has stepped into view
void sourceview_info::update()
{
	disasmview_info::update();
	const debug_view_sourcecode * dv_source = view<debug_view_sourcecode>();
	// TODO: Keeping my own copy of m_combownd and making update() virtual seems
	// inconsistent with rest of dbg arch.
	// What is the proper way to update its selection whenever the PC changes?
	SendMessage(m_combownd, CB_SETCURSEL, dv_source->cur_src_index(), 0);
}



} // namespace osd::debugger::win
