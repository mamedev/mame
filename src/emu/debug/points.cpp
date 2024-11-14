// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    points.cpp

    Debugger breakpoints, watchpoints, etc.

***************************************************************************/

#include "emu.h"
#include "points.h"

#include "debugger.h"
#include "debugcon.h"
#include "debugcpu.h"


//**************************************************************************
//  DEBUG BREAKPOINT
//**************************************************************************

//-------------------------------------------------
//  debug_breakpoint - constructor
//-------------------------------------------------

debug_breakpoint::debug_breakpoint(
		device_debug *debugInterface,
		symbol_table &symbols,
		int index,
		offs_t address,
		const char *condition,
		std::string_view action) :
	m_debugInterface(debugInterface),
	m_index(index),
	m_enabled(true),
	m_address(address),
	m_condition(symbols, condition ? condition : "1"),
	m_action(action)
{
}


//-------------------------------------------------
//  hit - detect a hit
//-------------------------------------------------

bool debug_breakpoint::hit(offs_t pc)
{
	// don't hit if disabled
	if (!m_enabled)
		return false;

	// must match our address
	if (m_address != pc)
		return false;

	// must satisfy the condition
	if (!m_condition.is_empty())
	{
		try
		{
			return (m_condition.execute() != 0);
		}
		catch (expression_error const &)
		{
			return false;
		}
	}

	return true;
}



//**************************************************************************
//  DEBUG WATCHPOINT
//**************************************************************************

//-------------------------------------------------
//  debug_watchpoint - constructor
//-------------------------------------------------

debug_watchpoint::debug_watchpoint(
		device_debug* debugInterface,
		symbol_table &symbols,
		int index,
		address_space &space,
		read_or_write type,
		offs_t address,
		offs_t length,
		const char *condition,
		std::string_view action) :
	m_debugInterface(debugInterface),
	m_phr(nullptr),
	m_phw(nullptr),
	m_space(space),
	m_index(index),
	m_enabled(true),
	m_type(type),
	m_address(address & space.addrmask()),
	m_length(length),
	m_condition(symbols, condition ? condition : "1"),
	m_action(action),
	m_installing(false)
{
	std::fill(std::begin(m_start_address), std::end(m_start_address), 0);
	std::fill(std::begin(m_end_address), std::end(m_end_address), 0);
	std::fill(std::begin(m_masks), std::end(m_masks), 0);

	int ashift = m_space.addr_shift();
	endianness_t endian = m_space.endianness();
	offs_t subamask = m_space.alignment() - 1;
	offs_t unit_size = ashift <= 0 ? 8 << -ashift : 8 >> ashift;
	offs_t start = m_address;
	offs_t end = (m_address + m_length - 1) & space.addrmask();
	if (end < start)
		end = space.addrmask();
	offs_t rstart = start & ~subamask;
	offs_t rend = end | subamask;
	u64 smask, mmask, emask;
	smask = mmask = emask = make_bitmask<u64>(m_space.data_width());
	if (start != rstart)
	{
		if (endian == ENDIANNESS_LITTLE)
			smask &= ~make_bitmask<u64>((start - rstart) * unit_size);
		else
			smask &= make_bitmask<u64>((rstart + subamask + 1 - start) * unit_size);
	}
	if (end != rend)
	{
		if (endian == ENDIANNESS_LITTLE)
			emask &= make_bitmask<u64>((subamask + 1 + end - rend) * unit_size);
		else
			emask &= ~make_bitmask<u64>((rend - end) * unit_size);
	}

	if (rend == (rstart | subamask) || smask == emask)
	{
		m_start_address[0] = rstart;
		m_end_address[0] = rend;
		m_masks[0] = smask & emask;
	}
	else
	{
		int idx = 0;
		if (smask != mmask)
		{
			m_start_address[idx] = rstart;
			m_end_address[idx] = rstart | subamask;
			m_masks[idx] = smask;
			idx++;
			rstart += subamask + 1;
		}
		if (mmask == emask)
		{
			m_start_address[idx] = rstart;
			m_end_address[idx] = rend;
			m_masks[idx] = emask;
		}
		else
		{
			if (rstart < rend - subamask)
			{
				m_start_address[idx] = rstart;
				m_end_address[idx] = rend - subamask - 1;
				m_masks[idx] = mmask;
				idx++;
			}
			m_start_address[idx] = rend - subamask;
			m_end_address[idx] = rend;
			m_masks[idx] = emask;
		}
	}

	install(read_or_write::READWRITE);
	m_notifier = m_space.add_change_notifier(
			[this] (read_or_write mode)
			{
				if (m_enabled)
				{
					install(mode);
				}
			});
}

debug_watchpoint::~debug_watchpoint()
{
	m_notifier.reset();
	m_phr.remove();
	m_phw.remove();
}

void debug_watchpoint::setEnabled(bool value)
{
	if (m_enabled != value)
	{
		m_enabled = value;
		if (m_enabled)
			install(read_or_write::READWRITE);
		else
		{
			m_installing = true;
			m_phr.remove();
			m_phw.remove();
			m_installing = false;
		}
	}
}

void debug_watchpoint::install(read_or_write mode)
{
	if (m_installing)
		return;
	m_installing = true;
	if (u32(mode) & u32(read_or_write::READ))
		m_phr.remove();
	if (u32(mode) & u32(read_or_write::WRITE))
		m_phw.remove();
	std::string name = util::string_format("wp@%x", m_address);
	switch (m_space.data_width())
	{
	case  8:
		if (u32(m_type) & u32(mode) & u32(read_or_write::READ))
			m_phr = m_space.install_read_tap(
					m_start_address[0], m_end_address[0], name,
					[this](offs_t offset, u8 &data, u8 mem_mask) {
						triggered(read_or_write::READ, offset, data, mem_mask);
					},
					&m_phr);
		if (u32(m_type) & u32(mode) & u32(read_or_write::WRITE))
			m_phw = m_space.install_write_tap(
					m_start_address[0], m_end_address[0], name,
					[this](offs_t offset, u8 &data, u8 mem_mask) {
						triggered(read_or_write::WRITE, offset, data, mem_mask);
					},
					&m_phw);
		break;

	case 16:
		for (int i=0; i != 3; i++)
			if (m_masks[i])
			{
				u16 mask = m_masks[i];
				if (u32(m_type) & u32(mode) & u32(read_or_write::READ))
					m_phr = m_space.install_read_tap(
							m_start_address[i], m_end_address[i], name,
							[this, mask](offs_t offset, u16 &data, u16 mem_mask) {
								if (mem_mask & mask)
									triggered(read_or_write::READ, offset, data, mem_mask);
							},
							&m_phr);
				if (u32(m_type) & u32(mode) & u32(read_or_write::WRITE))
					m_phw = m_space.install_write_tap(
							m_start_address[i], m_end_address[i], name,
							[this, mask](offs_t offset, u16 &data, u16 mem_mask) {
								if (mem_mask & mask)
									triggered(read_or_write::WRITE, offset, data, mem_mask);
							},
							&m_phw);
			}
		break;

	case 32:
		for (int i=0; i != 3; i++)
			if (m_masks[i])
			{
				u32 mask = m_masks[i];
				if (u32(m_type) & u32(mode) & u32(read_or_write::READ))
					m_phr = m_space.install_read_tap(
							m_start_address[i], m_end_address[i], name,
							[this, mask](offs_t offset, u32 &data, u32 mem_mask) {
								if (mem_mask & mask)
									triggered(read_or_write::READ, offset, data, mem_mask);
							},
							&m_phr);
				if (u32(m_type) & u32(mode) & u32(read_or_write::WRITE))
					m_phw = m_space.install_write_tap(
							m_start_address[i], m_end_address[i], name,
							[this, mask](offs_t offset, u32 &data, u32 mem_mask) {
								if (mem_mask & mask)
									triggered(read_or_write::WRITE, offset, data, mem_mask);
								},
								&m_phw);
			}
		break;

	case 64:
		for (int i=0; i != 3; i++)
			if (m_masks[i])
			{
				u64 mask = m_masks[i];
				if (u32(m_type) & u32(mode) & u32(read_or_write::READ))
					m_phr = m_space.install_read_tap(
							m_start_address[i], m_end_address[i], name,
							[this, mask](offs_t offset, u64 &data, u64 mem_mask) {
								if (mem_mask & mask)
									triggered(read_or_write::READ, offset, data, mem_mask);
							},
							&m_phr);
				if (u32(m_type) & u32(mode) & u32(read_or_write::WRITE))
					m_phw = m_space.install_write_tap(m_start_address[i], m_end_address[i], name,
							[this, mask](offs_t offset, u64 &data, u64 mem_mask) {
								if (mem_mask & mask)
									triggered(read_or_write::WRITE, offset, data, mem_mask);
							},
							&m_phw);
			}
		break;
	}
	m_installing = false;
}

void debug_watchpoint::triggered(read_or_write type, offs_t address, u64 data, u64 mem_mask)
{
	running_machine &machine = m_debugInterface->device().machine();
	debugger_manager &debug = machine.debugger();

	// if we're within debugger code, don't trigger
	if (debug.cpu().within_instruction_hook() || machine.side_effects_disabled())
		return;

	// adjust address, size & value_to_write based on mem_mask.
	offs_t size = 0;
	int ashift = m_space.addr_shift();
	offs_t unit_size = ashift <= 0 ? 8 << -ashift : 8 >> ashift;
	u64 unit_mask = make_bitmask<u64>(unit_size);

	offs_t address_offset = 0;

	if(!mem_mask)
		mem_mask = 0xff;

	while (!(mem_mask & unit_mask))
	{
		address_offset++;
		data >>= unit_size;
		mem_mask >>= unit_size;
	}

	while (mem_mask)
	{
		size++;
		mem_mask >>= unit_size;
	}

	data &= make_bitmask<u64>(size * unit_size);

	if (m_space.endianness() == ENDIANNESS_LITTLE)
		address += address_offset;
	else
		address += m_space.alignment() - size - address_offset;

	// stash the value that will be written or has just been read
	debug.cpu().set_wpinfo(address, data, size * unit_size);

	// protect against recursion
	debug.cpu().set_within_instruction(true);

	// must satisfy the condition
	if (!m_condition.is_empty())
	{
		try
		{
			if (!m_condition.execute())
			{
				debug.cpu().set_within_instruction(false);
				return;
			}
		}
		catch (expression_error &)
		{
			debug.cpu().set_within_instruction(false);
			return;
		}
	}

	// halt in the debugger by default
	bool was_stopped = debug.cpu().is_stopped();
	debug.cpu().set_execution_stopped();

	// evaluate the action
	if (!m_action.empty())
		debug.console().execute_command(m_action, false);

	// print a notification, unless the action made us go again
	if (debug.cpu().is_stopped())
	{
		std::string buffer;

		if (m_space.is_octal())
			buffer = string_format(type == read_or_write::READ ?
								   "Stopped at watchpoint %X reading %0*o from %0*o" :
								   "Stopped at watchpoint %X writing %0*o to %0*o",
								   m_index,
								   (size * unit_size + 2) / 3,
								   data,
								   (m_space.addr_width() + 2) / 3,
								   address);
		else
			buffer = string_format(type == read_or_write::READ ?
								   "Stopped at watchpoint %X reading %0*X from %0*X" :
								   "Stopped at watchpoint %X writing %0*X to %0*X",
								   m_index,
								   size * unit_size / 4,
								   data,
								   (m_space.addr_width() + 3) / 4,
								   address);

		const device_state_interface *state;
		if (debug.cpu().live_cpu() == &m_debugInterface->device() && m_debugInterface->device().interface(state))
		{
			debug.console().printf("%s (PC=%s)\n", buffer, state->state_string(STATE_GENPCBASE));
			m_debugInterface->compute_debug_flags();
		}
		else if (!was_stopped)
		{
			debug.console().printf("%s\n", buffer);
			debug.cpu().set_execution_running();
			debug.cpu().set_break_cpu(&m_debugInterface->device());
		}
		m_debugInterface->set_triggered_watchpoint(this);
	}

	debug.cpu().set_within_instruction(false);
}

//**************************************************************************
//  DEBUG REGISTERPOINT
//**************************************************************************

//-------------------------------------------------
//  debug_registerpoint - constructor
//-------------------------------------------------

debug_registerpoint::debug_registerpoint(symbol_table &symbols, int index, const char *condition, std::string_view action)
	: m_index(index),
		m_enabled(true),
		m_condition(symbols, (condition != nullptr) ? condition : "1"),
		m_action(action)
{
}


//-------------------------------------------------
//  hit - detect a hit
//-------------------------------------------------

bool debug_registerpoint::hit()
{
	// don't hit if disabled
	if (!m_enabled)
		return false;

	// must satisfy the condition
	if (!m_condition.is_empty())
	{
		try
		{
			return (m_condition.execute() != 0);
		}
		catch (expression_error &)
		{
			return false;
		}
	}

	return true;
}


//**************************************************************************
//  DEBUG EXCEPTION POINT
//**************************************************************************

//-------------------------------------------------
//  debug_exceptionpoint - constructor
//-------------------------------------------------

debug_exceptionpoint::debug_exceptionpoint(
		device_debug *debugInterface,
		symbol_table &symbols,
		int index,
		int type,
		const char *condition,
		std::string_view action)
	: m_debugInterface(debugInterface),
		m_index(index),
		m_enabled(true),
		m_type(type),
		m_condition(symbols, (condition != nullptr) ? condition : "1"),
		m_action(action)
{
}


//-------------------------------------------------
//  hit - detect a hit
//-------------------------------------------------

bool debug_exceptionpoint::hit(int exception)
{
	// don't hit if disabled
	if (!m_enabled)
		return false;

	// must match our type
	if (m_type != exception)
		return false;

	// must satisfy the condition
	if (!m_condition.is_empty())
	{
		try
		{
			return (m_condition.execute() != 0);
		}
		catch (expression_error &)
		{
			return false;
		}
	}

	return true;
}
