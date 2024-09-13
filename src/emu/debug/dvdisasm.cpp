// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Olivier Galibert
/*********************************************************************

    dvdisasm.cpp

    Disassembly debugger view.

***************************************************************************/

#include "emu.h"
#include "debugvw.h"
#include "dvdisasm.h"
#include "debugcpu.h"
#include "debugger.h"

//**************************************************************************
//  DEBUG VIEW DISASM SOURCE
//**************************************************************************

//-------------------------------------------------
//  debug_view_disasm_source - constructor
//-------------------------------------------------

debug_view_disasm_source::debug_view_disasm_source(std::string &&name, device_t &device)
	: debug_view_source(std::move(name), &device),
		m_space(device.memory().space(AS_PROGRAM)),
		m_decrypted_space(device.memory().has_space(AS_OPCODES) ? device.memory().space(AS_OPCODES) : device.memory().space(AS_PROGRAM)),
		m_pcbase(nullptr)
{
	const device_state_interface *state;
	if (device.interface(state))
		m_pcbase = state->state_find_entry(STATE_GENPCBASE);
}



//**************************************************************************
//  DEBUG VIEW DISASM
//**************************************************************************

//-------------------------------------------------
//  debug_view_disasm - constructor
//-------------------------------------------------

debug_view_disasm::debug_view_disasm(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate)
	: debug_view(machine, DVT_DISASSEMBLY, osdupdate, osdprivate),
		m_right_column(DASM_RIGHTCOL_RAW),
		m_backwards_steps(3),
		m_dasm_width(DEFAULT_DASM_WIDTH),
		m_previous_pc(1),
		m_expression(machine)
{
	// fail if no available sources
	enumerate_sources();
	if(m_source_list.empty())
		throw std::bad_alloc();

	// count the number of comments
	//int total_comments = 0;
	//for(auto &source : m_source_list)
	//{
		//const debug_view_disasm_source &dasmsource = downcast<const debug_view_disasm_source &>(*source);
		//total_comments += dasmsource.device()->debug()->comment_count();
	//}

	// configure the view
	m_total.y = DEFAULT_DASM_LINES;
	m_supports_cursor = true;
}


//-------------------------------------------------
//  ~debug_view_disasm - destructor
//-------------------------------------------------

debug_view_disasm::~debug_view_disasm()
{
}


//-------------------------------------------------
//  enumerate_sources - enumerate all possible
//  sources for a disassembly view
//-------------------------------------------------

void debug_view_disasm::enumerate_sources()
{
	// start with an empty list
	m_source_list.clear();

	// iterate over devices with disassembly interfaces
	for (device_disasm_interface &dasm : disasm_interface_enumerator(machine().root_device()))
	{
		if (dasm.device().memory().space_config(AS_PROGRAM))
		{
			m_source_list.emplace_back(
					std::make_unique<debug_view_disasm_source>(
						util::string_format("%s '%s'", dasm.device().name(), dasm.device().tag()),
						dasm.device()));
		}
	}

	// reset the source to a known good entry
	if (!m_source_list.empty())
		set_source(*m_source_list[0]);
}


//-------------------------------------------------
//  view_notify - handle notification of updates
//  to cursor changes
//-------------------------------------------------

void debug_view_disasm::view_notify(debug_view_notification type)
{
	if((type == VIEW_NOTIFY_CURSOR_CHANGED) && (m_cursor_visible == true))
		adjust_visible_y_for_cursor();

	else if(type == VIEW_NOTIFY_SOURCE_CHANGED)
	{
		const debug_view_disasm_source &source = downcast<const debug_view_disasm_source &>(*m_source);
		m_expression.set_context(&source.device()->debug()->symtable());
		m_expression.set_default_base(source.space().is_octal() ? 8 : 16);
	}
}


//-------------------------------------------------
//  view_char - handle a character typed within
//  the current view
//-------------------------------------------------

void debug_view_disasm::view_char(int chval)
{
	debug_view_xy origcursor = m_cursor;
	u8 end_buffer = 3;
	s32 temp;

	switch(chval)
	{
		case DCH_UP:
			if(m_cursor.y > 0)
				m_cursor.y--;
			break;

		case DCH_DOWN:
			if(m_cursor.y < m_total.y - 1)
				m_cursor.y++;
			break;

		case DCH_PUP:
			temp = m_cursor.y -(m_visible.y - end_buffer);
			if(temp < 0)
				m_cursor.y = 0;
			else
				m_cursor.y = temp;
			break;

		case DCH_PDOWN:
			temp = m_cursor.y +(m_visible.y - end_buffer);
			if(temp > m_total.y - 1)
				m_cursor.y = m_total.y - 1;
			else
				m_cursor.y = temp;
			break;

		case DCH_HOME:              // set the active column to the PC
		{
			const debug_view_disasm_source &source = downcast<const debug_view_disasm_source &>(*m_source);
			offs_t pc = source.pcbase();

			// figure out which row the pc is on
			for(unsigned int curline = 0; curline < m_dasm.size(); curline++)
				if(m_dasm[curline].m_address == pc)
					m_cursor.y = curline;
			break;
		}

		case DCH_CTRLHOME:
			m_cursor.y = 0;
			break;

		case DCH_CTRLEND:
			m_cursor.y = m_total.y - 1;
			break;
	}

	/* send a cursor changed notification */
	if(m_cursor.y != origcursor.y)
	{
		begin_update();
		view_notify(VIEW_NOTIFY_CURSOR_CHANGED);
		m_update_pending = true;
		end_update();
	}
}


//-------------------------------------------------
//  view_click - handle a mouse click within the
//  current view
//-------------------------------------------------

void debug_view_disasm::view_click(const int button, const debug_view_xy& pos)
{
	const debug_view_xy origcursor = m_cursor;
	m_cursor = pos;

	/* cursor popup|toggle */
	bool cursorVisible = true;
	if(m_cursor.y == origcursor.y)
	{
		cursorVisible = !m_cursor_visible;
	}

	/* send a cursor changed notification */
	begin_update();
	m_cursor_visible = cursorVisible;
	view_notify(VIEW_NOTIFY_CURSOR_CHANGED);
	m_update_pending = true;
	end_update();
}

void debug_view_disasm::generate_from_address(debug_disasm_buffer &buffer, offs_t address)
{
	m_dasm.clear();
	for(int i=0; i != m_total.y; i++) {
		std::string dasm;
		offs_t size;
		offs_t next_address;
		u32 info;
		buffer.disassemble(address, dasm, next_address, size, info);
		m_dasm.emplace_back(address, size, dasm);
		address = next_address;
	}
	m_recompute = false;
}

bool debug_view_disasm::generate_with_pc(debug_disasm_buffer &buffer, offs_t pc)
{
	// Consider that instructions are 64 bytes max
	const debug_view_disasm_source &source = downcast<const debug_view_disasm_source &>(*m_source);
	int shift = source.m_space.addr_shift();

	offs_t backwards_offset;
	if(shift < 0)
		backwards_offset = 64 >> -shift;
	else if(shift == 0)
		backwards_offset = 64;
	else
		backwards_offset = 64 << shift;

	m_dasm.clear();
	m_recompute = false;
	offs_t address = (pc - m_backwards_steps*backwards_offset) & source.m_space.logaddrmask();
	// Handle wrap at 0
	if(address > pc)
		address = 0;

	util::disasm_interface &intf(dynamic_cast<device_disasm_interface &>(*source.device()).get_disassembler());
	if(intf.interface_flags() & util::disasm_interface::NONLINEAR_PC) {
		offs_t lpc = intf.pc_real_to_linear(pc);
		while(intf.pc_real_to_linear(address) < lpc) {
			std::string dasm;
			offs_t size;
			offs_t next_address;
			u32 info;
			buffer.disassemble(address, dasm, next_address, size, info);
			m_dasm.emplace_back(address, size, dasm);
			if(intf.pc_real_to_linear(address) > intf.pc_real_to_linear(next_address))
				return false;
			address = next_address;
		}

	} else {
		while(address < pc) {
			std::string dasm;
			offs_t size;
			offs_t next_address;
			u32 info;
			buffer.disassemble(address, dasm, next_address, size, info);
			m_dasm.emplace_back(address, size, dasm);
			if(address > next_address)
				return false;
			address = next_address;
		}
	}

	if(address != pc)
		return false;

	if(m_dasm.size() > m_backwards_steps)
		m_dasm.erase(m_dasm.begin(), m_dasm.begin() + (m_dasm.size() - m_backwards_steps));

	while(m_dasm.size() < m_total.y) {
		std::string dasm;
		offs_t size;
		offs_t next_address;
		u32 info;
		buffer.disassemble(address, dasm, next_address, size, info);
		m_dasm.emplace_back(address, size, dasm);
		address = next_address;
	}
	return true;
}

int debug_view_disasm::address_position(offs_t pc) const
{
	for(int i=0; i != int(m_dasm.size()); i++)
		if(m_dasm[i].m_address == pc)
			return i;
	return -1;
}

void debug_view_disasm::generate_dasm(debug_disasm_buffer &buffer, offs_t pc)
{
	bool pc_changed = pc != m_previous_pc;
	m_previous_pc = pc;
	if(strcmp(m_expression.string(), "curpc")) {
		if(m_expression.dirty()) {
			m_topleft.x = 0;
			m_topleft.y = 0;
		}
		const debug_view_disasm_source &source = downcast<const debug_view_disasm_source &>(*m_source);
		generate_from_address(buffer, m_expression.value() & source.m_space.logaddrmask());
		return;
	}

	if(!m_recompute && address_position(pc) != -1) {
		generate_from_address(buffer, m_dasm[0].m_address);
		int pos = address_position(pc);
		if(pos != -1) {
			if(!pc_changed)
				return;
			if(pos >= m_topleft.y && pos < m_topleft.y + m_visible.y - 2)
				return;
			if(pos < m_total.y - m_visible.y) {
				m_topleft.x = 0;
				m_topleft.y = pos - m_backwards_steps;
				return;
			}
		}
	}

	m_topleft.x = 0;
	m_topleft.y = 0;

	if(generate_with_pc(buffer, pc))
		return;

	generate_from_address(buffer, pc);
}

void debug_view_disasm::complete_information(const debug_view_disasm_source &source, debug_disasm_buffer &buffer, offs_t pc)
{
	for(auto &dasm : m_dasm) {
		offs_t adr = dasm.m_address;

		dasm.m_tadr = buffer.pc_to_string(adr);
		dasm.m_topcodes = buffer.data_to_string(adr, dasm.m_size, true);
		dasm.m_tparams  = buffer.data_to_string(adr, dasm.m_size, false);

		dasm.m_is_pc = adr == pc;

		dasm.m_is_bp = source.device()->debug()->breakpoint_find(adr) != nullptr;
		dasm.m_is_visited = source.device()->debug()->track_pc_visited(adr);

		const char *comment = source.device()->debug()->comment_text(adr);
		if(comment)
			dasm.m_comment = comment;
	}
}

//-------------------------------------------------
//  view_update - update the contents of the
//  disassembly view
//-------------------------------------------------

void debug_view_disasm::view_update()
{
	const debug_view_disasm_source &source = downcast<const debug_view_disasm_source &>(*m_source);
	debug_disasm_buffer buffer(*source.device());
	offs_t pc = source.pcbase();

	generate_dasm(buffer, pc);

	complete_information(source, buffer, pc);
	redraw();
}


//-------------------------------------------------
//  print - print a string in the disassembly view
//-------------------------------------------------

void debug_view_disasm::print(u32 row, std::string text, s32 start, s32 end, u8 attrib)
{
	s32 view_end = end - m_topleft.x;
	if(view_end < 0)
		return;

	s32 string_0 = start - m_topleft.x;
	if(string_0 >= m_visible.x)
		return;

	s32 view_start = string_0 > 0 ? string_0 : 0;
	debug_view_char *dest = &m_viewdata[row * m_visible.x + view_start];

	if(view_end >= m_visible.x)
		view_end = m_visible.x;

	for(s32 pos = view_start; pos < view_end; pos++) {
		s32 spos = pos - string_0;
		if(spos >= s32(text.size()))
			*dest++ = { ' ', attrib };
		else
			*dest++ = { u8(text[spos]), attrib };
	}
}


//-------------------------------------------------
//  redraw - update the view from the data
//-------------------------------------------------

void debug_view_disasm::redraw()
{
	// determine how many characters we need for an address and set the divider
	s32 divider1 = 1 + m_dasm[0].m_tadr.size() + 1;

	// assume a fixed number of characters for the disassembly
	s32 divider2 = divider1 + 1 + m_dasm_width + 1;

	// set the width of the third column to max comment length
	m_total.x = divider2 + 1 + 50;        // DEBUG_COMMENT_MAX_LINE_LENGTH

	const s32 max_visible_col = m_topleft.x + m_visible.x;

	// loop over visible rows
	for(u32 row = 0; row < m_visible.y; row++)
	{
		u32 effrow = m_topleft.y + row;

		// if this visible row is valid, add it to the buffer
		u8 attrib = DCA_NORMAL;
		if(effrow < m_dasm.size())
		{
			// if we're on the line with the PC, hilight it
			if(m_dasm[effrow].m_is_pc)
				attrib = DCA_CURRENT;

			// if we're on a line with a breakpoint, tag it changed
			else if(m_dasm[effrow].m_is_bp)
				attrib = DCA_CHANGED;

			// if we're on the active column and everything is couth, highlight it
			if(m_cursor_visible && effrow == m_cursor.y)
				attrib |= DCA_SELECTED;

			// if we've visited this pc, mark it as such
			if(m_dasm[effrow].m_is_visited)
				attrib |= DCA_VISITED;

			print(row, ' ' + m_dasm[effrow].m_tadr, 0, divider1, attrib | DCA_ANCILLARY);
			print(row, ' ' + m_dasm[effrow].m_dasm, divider1, divider2, attrib);

			if(m_right_column == DASM_RIGHTCOL_RAW || m_right_column == DASM_RIGHTCOL_ENCRYPTED) {
				std::string text = ' ' +(m_right_column == DASM_RIGHTCOL_RAW ? m_dasm[effrow].m_topcodes : m_dasm[effrow].m_tparams);
				print(row, text, divider2, max_visible_col, attrib | DCA_ANCILLARY);
				if(s32(text.size()) > max_visible_col - divider2) {
					s32 base = max_visible_col - 3;
					if(base < divider2)
						base = divider2;
					print(row, "...", base, max_visible_col, attrib | DCA_ANCILLARY);
				}
			} else if(!m_dasm[effrow].m_comment.empty())
				print(row, " // " + m_dasm[effrow].m_comment, divider2, max_visible_col, attrib | DCA_COMMENT | DCA_ANCILLARY);
			else
				print(row, "", divider2, max_visible_col, attrib | DCA_COMMENT | DCA_ANCILLARY);
		}
	}
}


//-------------------------------------------------
//  selected_address - return the PC of the
//  currently selected address in the view
//-------------------------------------------------

offs_t debug_view_disasm::selected_address()
{
	flush_updates();
	return m_dasm[m_cursor.y].m_address;
}


//-------------------------------------------------
//  set_sexpression - set the expression string
//  describing the home address
//-------------------------------------------------

void debug_view_disasm::set_expression(const std::string &expression)
{
	begin_update();
	m_expression.set_string(expression);
	m_recompute = m_update_pending = true;
	end_update();
}


//-------------------------------------------------
//  set_right_column - set the contents of the
//  right column
//-------------------------------------------------

void debug_view_disasm::set_right_column(disasm_right_column contents)
{
	begin_update();
	m_right_column = contents;
	m_update_pending = true;
	end_update();
}


//-------------------------------------------------
//  set_backward_steps - set the number of
//  instructions displayed before the home address
//-------------------------------------------------

void debug_view_disasm::set_backward_steps(u32 steps)
{
	begin_update();
	m_backwards_steps = steps;
	m_recompute = m_update_pending = true;
	end_update();
}


//-------------------------------------------------
//  set_disasm_width - set the width in characters
//  of the main disassembly section
//-------------------------------------------------

void debug_view_disasm::set_disasm_width(u32 width)
{
	begin_update();
	m_dasm_width = width;
	m_update_pending = true;
	end_update();
}


//-------------------------------------------------
//  set_selected_address - set the PC of the
//  currently selected address in the view
//-------------------------------------------------

void debug_view_disasm::set_selected_address(offs_t address)
{
	const debug_view_disasm_source &source = downcast<const debug_view_disasm_source &>(*m_source);
	address = address & source.m_space.logaddrmask();
	for(int line = 0; line < m_total.y; line++)
		if(m_dasm[line].m_address == address) {
			m_cursor.y = line;
			set_cursor_position(m_cursor);
			break;
		}
}

//-------------------------------------------------
//  set_source - set the current subview
//-------------------------------------------------

void debug_view_disasm::set_source(const debug_view_source &source)
{
	if(&source != m_source) {
		m_recompute = true;
		debug_view::set_source(source);
	}
}
