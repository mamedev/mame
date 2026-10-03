// license:BSD-3-Clause
// copyright-holders:David Broman
/*********************************************************************

    dvsourcecode.h

    Source code debugger view.

***************************************************************************/

#ifndef MAME_EMU_DEBUG_DVSOURCE_H
#define MAME_EMU_DEBUG_DVSOURCE_H

#pragma once

#include "dvdisasm.h"
#include "srcdbg_info.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// debug view for source-level debugging.  Shares code with debug_view_disasm, especially
// for cursor handling, clicks, and navigational keypresses
class debug_view_sourcecode : public debug_view_disasm
{
	friend class debug_view_manager;

public:
	// getters
	const srcdbg_info * get_srcdbg_info() const { return m_srcdbg_info; }
	u16 cur_src_index() const { return m_cur_src_index; }
	virtual std::optional<offs_t> selected_address() override;

	// setters
	void set_src_index(u16 new_src_index);

	bool update_gui_needs_full_refresh();

protected:
	// construction/destruction
	debug_view_sourcecode(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate);
	virtual ~debug_view_sourcecode();

	// view overrides
	virtual void set_source(const debug_view_source &source) override;
	virtual void view_update() override;

private:
	void viewdata_text_update(bool pc_changed, offs_t pc);
	void print_line(u32 row, const char * text, u8 attrib) { print_line( row, std::optional<u32>(), text, attrib); };
	void print_line(u32 row, std::optional<u32> line_number, const char * text, u8 attrib);
	void print_file_open_error(const srcdbg_provider_base::source_file_path & path, std::error_condition err = std::error_condition());
	void print_file_unavailable_error();
	void clear_from_row_onward(s32 row_start);

	u32 first_visible_line() { return m_topleft.y + 1; }
	bool is_visible(u32 line) { return (first_visible_line() <= line && line < first_visible_line() + m_visible.y); }
	bool update_opened_file();
	void update_visible_lines(offs_t pc);
	bool exists_bp_for_line(u16 src_index, u32 line);

	const device_state_interface *                    m_state;                  // state interface, if present
	srcdbg_info *                                     m_srcdbg_info;            // Interface to the loaded debugging info files, can be null!
	u16                                               m_cur_src_index;          // Identifies which source file we should now show / switch to
	u16                                               m_displayed_src_index;    // Identifies which source file is currently shown
	std::unique_ptr<util::line_indexed_file>          m_displayed_src_file;     // File object currently printed to the view
	std::optional<u32>                                m_line_for_cur_pc;        // Line number to be highlighted
	bool                                              m_gui_needs_full_refresh; // Internal bool tracking whether OSD GUI needs to do a full refresh
};

#endif // MAME_EMU_DEBUG_DVSOURCE_H
