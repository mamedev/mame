// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    dvdisasm.h

    Disassembly debugger view.

***************************************************************************/

#ifndef MAME_EMU_DEBUG_DVDISASM_H
#define MAME_EMU_DEBUG_DVDISASM_H

#pragma once

#include "debugvw.h"
#include "debugbuf.h"

#include "vecstream.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// selections for what goes into the right-hand column
enum disasm_right_column
{
	DASM_RIGHTCOL_NONE,
	DASM_RIGHTCOL_RAW,
	DASM_RIGHTCOL_ENCRYPTED,
	DASM_RIGHTCOL_COMMENTS
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// a disassembly view_source
class debug_view_disasm_source : public debug_view_source
{
	friend class debug_view_disasm;

public:
	// construction/destruction
	debug_view_disasm_source(std::string &&name, device_t &device);

	// getters
	address_space &space() const { return m_space; }
	offs_t pcbase() const { return m_pcbase != nullptr ? m_pcbase->value() & m_space.logaddrmask() : 0; }

private:
	// internal state
	address_space &     m_space;                // address space to display
	address_space &     m_decrypted_space;      // address space to display for decrypted opcodes
	const device_state_entry *m_pcbase;
};


// debug view for disassembly
class debug_view_disasm : public debug_view
{
	friend class debug_view_manager;

	// construction/destruction
	debug_view_disasm(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate);
	virtual ~debug_view_disasm();

public:
	// getters
	const char *expression() const { return m_expression.string(); }
	disasm_right_column right_column() const { return m_right_column; }
	u32 backward_steps() const { return m_backwards_steps; }
	u32 disasm_width() const { return m_dasm_width; }
	offs_t selected_address();

	// setters
	void set_expression(const std::string &expression);
	void set_right_column(disasm_right_column contents);
	void set_backward_steps(u32 steps);
	void set_disasm_width(u32 width);
	void set_selected_address(offs_t address);
	virtual void set_source(const debug_view_source &source) override;

protected:
	// view overrides
	virtual void view_update() override;
	virtual void view_notify(debug_view_notification type) override;
	virtual void view_char(int chval) override;
	virtual void view_click(const int button, const debug_view_xy& pos) override;

private:
	// The information of one disassembly line. May become the actual
	// external interface at one point.
	struct dasm_line {
		offs_t m_address;                       // address of the instruction
		offs_t m_size;                          // size of the instruction

		std::string m_tadr;                     // instruction address as a string
		std::string m_dasm;                     // disassembly
		std::string m_topcodes;                 // textual representation of opcode/default values
		std::string m_tparams;                  // textual representation of parameter values
		std::string m_comment;                  // comment, when present

		bool m_is_pc;                           // this line's address is PC
		bool m_is_bp;                           // this line's address is a breakpoint
		bool m_is_visited;                      // this line has been visited

		dasm_line(offs_t address, offs_t size, std::string dasm) : m_address(address), m_size(size), m_dasm(dasm), m_is_pc(false), m_is_bp(false), m_is_visited(false) {}
	};

	// internal helpers
	void generate_from_address(debug_disasm_buffer &buffer, offs_t address);
	bool generate_with_pc(debug_disasm_buffer &buffer, offs_t pc);
	int address_position(offs_t pc) const;
	void generate_dasm(debug_disasm_buffer &buffer, offs_t pc);
	void complete_information(const debug_view_disasm_source &source, debug_disasm_buffer &buffer, offs_t pc);

	void enumerate_sources();
	void print(u32 row, std::string text, s32 start, s32 end, u8 attrib);
	void redraw();

	// internal state
	disasm_right_column    m_right_column;         // right column contents
	u32                    m_backwards_steps;      // number of backwards steps
	u32                    m_dasm_width;           // width of the disassembly area
	offs_t                 m_previous_pc;          // previous pc, to detect whether it changed
	debug_view_expression  m_expression;           // expression-related information
	std::vector<dasm_line> m_dasm;                 // disassembled instructions

	// constants
	static constexpr int DEFAULT_DASM_LINES = 1000;
	static constexpr int DEFAULT_DASM_WIDTH = 50;
	static constexpr int DASM_MAX_BYTES = 16;
};

#endif // MAME_EMU_DEBUG_DVDISASM_H
