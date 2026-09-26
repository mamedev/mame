// license:BSD-3-Clause
// copyright-holders:David Broman
/*********************************************************************

    dvsourcecode.cpp

    Source code debugger view.

***************************************************************************/

#include "emu.h"

#include "dvsourcecode.h"
#include "debugger.h"
#include "srcdbg_info.h"
#include "srcdbg_provider.h"

#include "emuopts.h"
#include "line_idx_file.h"


//-------------------------------------------------
//  debug_view_sourcecode - constructor
//-------------------------------------------------

debug_view_sourcecode::debug_view_sourcecode(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate) :
	debug_view_disasm(machine, osdupdate, osdprivate, true /* source_code_debugging */),
	m_state(nullptr),
	m_srcdbg_info(machine.debugger().get_srcdbg_info()),
	m_cur_src_index(0),
	m_displayed_src_index(-1),
	m_displayed_src_file(std::make_unique<util::line_indexed_file>()),
	m_line_for_cur_pc(),
	m_gui_needs_full_refresh(true)
{
	if (m_srcdbg_info != nullptr)
	{
		m_supports_cursor = true;
	}
}


//-------------------------------------------------
//  ~debug_view_sourcecode - destructor
//-------------------------------------------------

debug_view_sourcecode::~debug_view_sourcecode()
{
}


//-------------------------------------------------
//  selected_address - return the PC of the
//  currently selected address in the view
//-------------------------------------------------

std::optional<offs_t> debug_view_sourcecode::selected_address()
{
	if (m_srcdbg_info == nullptr)
	{
		return std::optional<offs_t>();
	}

	flush_updates();
	u32 line = m_cursor.y + 1;

	std::vector<srcdbg_provider_base::address_range> ranges;
	m_srcdbg_info->file_line_to_address_ranges(m_cur_src_index, line, ranges);

	if (ranges.size() == 0)
	{
		return std::optional<offs_t>();
	}

	return std::optional<offs_t>(ranges[0].first);
}


//-------------------------------------------------
// update_opened_file - Modifies state to ensure
// the file identified by m_cur_src_index is shown
//-------------------------------------------------

bool debug_view_sourcecode::update_opened_file()
{
	assert(m_srcdbg_info != nullptr);
	if (m_cur_src_index == m_displayed_src_index)
	{
		return true;
	}

	const srcdbg_provider_base::source_file_path * path;
	if (!m_srcdbg_info->file_index_to_path(m_cur_src_index, &path))
	{
		print_file_unavailable_error();
		return false;
	}
	const char * local_path = path->local();
	if (local_path == nullptr)
	{
		print_file_open_error(*path);
		return false;
	}

	int spaces_per_tab = machine().options().srcdbg_spaces_per_tab();
	std::error_condition err = m_displayed_src_file->open(local_path, spaces_per_tab);
	if (err)
	{
		print_file_open_error(*path, err);
		return false;
	}

	m_displayed_src_index = m_cur_src_index;
	m_total.y = m_displayed_src_file->num_lines();
	return true;
}


//-------------------------------------------------
// set_source - Update m_state with the
// specified debug_view_source
//-------------------------------------------------

void debug_view_sourcecode::set_source(const debug_view_source &source)
{
	source.device()->interface(m_state);
}


//-------------------------------------------------
// view_update - update the contents of the
// source code view
//-------------------------------------------------

void debug_view_sourcecode::view_update()
{
	// Show explanatory text if source-level debugging is not active
	if (m_srcdbg_info == nullptr)
	{
		print_line(0, "Source-level debugging is not active", DCA_CHANGED);
		print_line(1, "Specify option '" OPTION_SRCDBGINFO "' to enable", DCA_NORMAL);
		clear_from_row_onward(2);
		return;
	}

	bool pc_changed = false;
	offs_t pc = 0;
	if (m_state != nullptr)
	{
		pc = m_state->pcbase();
		pc_changed = update_previous_pc(pc);
	}

	if (m_srcdbg_info && m_srcdbg_info->update_view_needs_full_refresh())
	{
		// The update actually did something, so reset state given that the list
		// of enabled providers has changed.
		m_cur_src_index = 0;
		m_displayed_src_index = -1;
		m_displayed_src_file = std::make_unique<util::line_indexed_file>();
		m_line_for_cur_pc = std::optional<u32>();
		pc_changed = true;
		m_gui_needs_full_refresh = true;
	}

	// If pc has changed, find its file & line number if possible
	if (pc_changed)
	{
		file_line loc;
		if (m_srcdbg_info->address_to_file_line(pc, loc))
		{
			m_cur_src_index = loc.file_index();
			m_line_for_cur_pc = loc.line_number();
		}
		else
		{
			pc_changed = false;
		}
	}

	viewdata_text_update(pc_changed, pc);
}


//-------------------------------------------------
// viewdata_text_update - Helper for view_update
// to do the actual printing of the source file 
//-------------------------------------------------

void debug_view_sourcecode::viewdata_text_update(bool pc_changed, offs_t pc)
{
	// Ensure the correct file is open.  First time view is displayed, this opens
	// the top of file index 0
	if (!update_opened_file())
	{
		return;
	}

	// Scroll current line into view, but only if the user has been
	// stepping or running so we don't trample on manual scrolling
	if (pc_changed)
	{
		update_visible_lines(pc);
	}

	// Populate view with the correct text
	for (u32 row = 0; row < m_visible.y; row++)
	{
		u32 line = row + m_topleft.y + 1;
		if (line > m_displayed_src_file->num_lines())
		{
			print_line(row, " ", DCA_NORMAL);
		}
		else
		{
			u8 attrib = DCA_NORMAL;

			if (m_line_for_cur_pc.has_value() && line == m_line_for_cur_pc.value())
			{
				// on the line with the PC: highlight
				attrib = DCA_CURRENT;
			}
			else if (exists_bp_for_line(m_cur_src_index, line))
			{
				// on a line with a breakpoint: tag it changed
				attrib = DCA_CHANGED;
			}

			if (m_cursor_visible && (line - 1) == m_cursor.y)
			{
				// We're on the cursored line and cursor is visible: highlight
				attrib |= DCA_SELECTED;
			}

			print_line(
				row,
				line,
				m_displayed_src_file->get_line_text(line),
				attrib);
		}
	}
}


//-------------------------------------------------
// clear_from_row_onward - Pad with empty space
// from specified row to end of visible view
//-------------------------------------------------

void debug_view_sourcecode::clear_from_row_onward(s32 row_start)
{
	for (u32 row = row_start; row < m_visible.y; row++)
	{
		print_line(row, " ", DCA_NORMAL);
	}
	m_total.y = row_start;
}


//-------------------------------------------------
// print_file_open_error - Helper to print
// explanatory text when file opening fails
//-------------------------------------------------

void debug_view_sourcecode::print_file_open_error(const srcdbg_provider_base::source_file_path & path, std::error_condition err)
{
	s32 row = 0;
	print_line(row++, "Error opening file", DCA_CHANGED);
	if (path.local() == nullptr)
	{
		print_line(row++, "Could not find local file matching originally built source", DCA_CHANGED);
	}
	else if (err)
	{
		print_line(row++, path.local(), DCA_CHANGED);
		print_line(row++, err.message().c_str(), DCA_CHANGED);
	}

	std::string s = util::string_format("Originally built source: %s", path.built());
	print_line(row++, s.c_str(), DCA_NORMAL);
	s = util::string_format("Source search path (%s): %s", OPTION_SRCDBGSEARCHPATH, machine().options().srcdbg_search_path());
	print_line(row++, s.c_str(), DCA_NORMAL);
	s = util::string_format("Source path prefix map (%s): %s", OPTION_SRCDBGPREFIXMAP, machine().options().srcdbg_prefix_map());
	print_line(row++, s.c_str(), DCA_NORMAL);
	clear_from_row_onward(row);
	m_total.y = row;
}


// -------------------------------------------------
// print_file_unavailable_error - Helper to print
// explanatory text when referenced file is not
// in any enabled MDI
// -------------------------------------------------

void debug_view_sourcecode::print_file_unavailable_error()
{
	print_line(0, "The current source file does not appear", DCA_CHANGED);
	print_line(1, "in enabled source debugging information.", DCA_CHANGED);
	print_line(2, "Try running sdlist / sdenable to increase available debugging info.", DCA_CHANGED);
	clear_from_row_onward(3);
}


//-------------------------------------------------
// exists_bp_for_line - Helper to determine if
// there is a breakpoint on the specified line
//-------------------------------------------------

bool debug_view_sourcecode::exists_bp_for_line(u16 src_index, u32 line)
{
	assert(m_srcdbg_info != nullptr);
	std::vector<srcdbg_provider_base::address_range> ranges;
	m_srcdbg_info->file_line_to_address_ranges(m_cur_src_index, line, ranges);
	const device_debug * debug = source()->device()->debug();
	for (offs_t i = 0; i < ranges.size(); i++)
	{
		for (offs_t addr = ranges[i].first; addr <= ranges[i].second; addr++)
		{
			if (debug->breakpoint_find(addr) != nullptr)
			{
				return true;
			}
		}
	}
	return false;
}


//-------------------------------------------------
// update_visible_lines - center m_line_for_cur_pc
// vertically in view (with corner cases to account
// for file size and to minimize unnecessary
// movement).
//-------------------------------------------------

void debug_view_sourcecode::update_visible_lines(offs_t pc)
{
	if (!m_line_for_cur_pc.has_value() || is_visible(m_line_for_cur_pc.value()))
	{
		return;
	}

	u32 line_for_cur_pc = m_line_for_cur_pc.value();

	if (m_displayed_src_file->num_lines() <= m_visible.y)
	{
		// Entire file fits in visible view.  Start at begining
		m_topleft.y = 0;
	}
	else if (line_for_cur_pc <= m_visible.y / 2)
	{
		// line_for_cur_pc close to top, start at top
		m_topleft.y = 0;
	}
	else if (line_for_cur_pc + m_visible.y / 2 > m_displayed_src_file->num_lines())
	{
		// line_for_cur_pc close to bottom, so bottom line at bottom
		m_topleft.y = m_displayed_src_file->num_lines() - 1 - m_visible.y;
	}
	else
	{
		// Main case, center line_for_cur_pc in view
		m_topleft.y = line_for_cur_pc - 1 - m_visible.y / 2;
	}
}


//-------------------------------------------------
// print_line - Helper to print a single line
// plus optional line number to view
//-------------------------------------------------

void debug_view_sourcecode::print_line(u32 row, std::optional<u32> line_number, const char * text, u8 attrib)
{
	const char LINE_NUMBER_PADDING[] = "     ";
	const s32 LINE_NUMBER_WIDTH = sizeof(LINE_NUMBER_PADDING)-1;

	// Left side shows line number (or space padding)
	std::string line_str =  (line_number.has_value()) ? std::to_string(line_number.value()) : LINE_NUMBER_PADDING;
	for(s32 visible_col=m_topleft.x; visible_col < m_topleft.x + std::min(LINE_NUMBER_WIDTH, m_visible.x); visible_col++)
	{
		s32 viewdata_col = visible_col - m_topleft.x;
		m_viewdata[row * m_visible.x + viewdata_col] =
		{
			(viewdata_col < line_str.size()) ? u8(line_str[viewdata_col]) : u8(' '),
			u8(DCA_DISABLED | attrib)
		};
	}

	// Right side shows line from file
	for(s32 visible_col=m_topleft.x + LINE_NUMBER_WIDTH; visible_col < m_topleft.x + m_visible.x; visible_col++)
	{
		s32 viewdata_col = visible_col - m_topleft.x;
		s32 text_idx = visible_col - LINE_NUMBER_WIDTH;

		if (text_idx >= strlen(text))
		{
			m_viewdata[row * m_visible.x + viewdata_col] = { ' ', attrib };
		}
		else
		{
			m_viewdata[row * m_visible.x + viewdata_col] = { u8(text[text_idx]), attrib };
		}
	}
}


//-------------------------------------------------
// set_src_index - Called when user selects a
// file to show in the view
//-------------------------------------------------

void debug_view_sourcecode::set_src_index(u16 new_src_index)
{
	if (m_srcdbg_info == nullptr ||
		m_cur_src_index == new_src_index ||
		new_src_index >= m_srcdbg_info->num_files())
	{
		return;
	}

	begin_update();
	m_cur_src_index = new_src_index;
	m_update_pending = true;
	// No need to call view_notify()
	end_update();
}


//-------------------------------------------------
// update_gui_needs_full_refresh - Returns tracked bool
// indicating whether OSD GUI should do a full refresh
// (including repopulating the source file combo box),
// then resets the tracked bool.
//-------------------------------------------------

bool debug_view_sourcecode::update_gui_needs_full_refresh()
{
	bool ret = m_gui_needs_full_refresh;
	m_gui_needs_full_refresh = false;
	return ret;
}
