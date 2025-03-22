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
#include "srcdbg_provider.h"


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
	const srcdbg_provider_base * srcdbg_provider() const { return m_srcdbg_provider; }
	u16 cur_src_index() const { return m_cur_src_index; }
	virtual std::optional<offs_t> selected_address() override;

	// setters
	void set_src_index(u16 new_src_index);

protected:
	// construction/destruction
	debug_view_sourcecode(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate);
	virtual ~debug_view_sourcecode();

	// view overrides
	virtual void set_source(const debug_view_source &source) override;
	virtual void view_update() override;

private:
	// Helper class to encapsulate the contents of a text file, indexed
	// by line number.  The view is populated with data from this class.
	class line_indexed_file
	{
	public:
		line_indexed_file();
		~line_indexed_file() { };
		const std::error_condition & open(const char * file_path);
		const std::error_condition & last_open_error() { return m_err; };
		u32 num_lines() { return m_line_starts.size(); };
		const char * get_line_text(u32 n) { return (const char *) &m_data[m_line_starts[n-1]]; };

	private:
		std::error_condition m_err;
		std::vector<uint8_t> m_data;
		std::vector<u32> m_line_starts;
	};

	void print_line(u32 row, const char * text, u8 attrib) { print_line( row, std::optional<u32>(), text, attrib); };
	void print_line(u32 row, std::optional<u32> line_number, const char * text, u8 attrib);
	void print_file_open_error(const srcdbg_provider_base::source_file_path & path);

	u32 first_visible_line() { return m_topleft.y + 1; }
	bool is_visible(u32 line) { return (first_visible_line() <= line && line < first_visible_line() + m_visible.y); }
	void update_opened_file();
	void update_visible_lines(offs_t pc);
	bool exists_bp_for_line(u16 src_index, u32 line);

	const device_state_interface *                    m_state;                 // state interface, if present
	const srcdbg_provider_base *                      m_srcdbg_provider;       // Interface to the loaded debugging info file, can be null!
	u16                                               m_cur_src_index;         // Identifies which source file we should now show / switch to
	u16                                               m_displayed_src_index;   // Identifies which source file is currently shown
	std::unique_ptr<line_indexed_file>                m_displayed_src_file;    // File object currently printed to the view
	std::optional<u32>                                m_line_for_cur_pc;       // Line number to be highlighted
};

#endif // MAME_EMU_DEBUG_DVSOURCE_H
