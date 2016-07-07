// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    devdebug.c

    Conditional device interface for debugging.

***************************************************************************/

#include "emu.h"
#include "osdepend.h"
#include "debugcpu.h"
#include "debugcon.h"
#include "devdebug.h"
#include "express.h"
#include "debugvw.h"
#include "debugger.h"
#include "xmlfile.h"
#include "coreutil.h"



//**************************************************************************
//  DEVICE DEBUG
//**************************************************************************

//-------------------------------------------------
//  device_debug - constructor
//-------------------------------------------------

device_debug::device_debug(device_t &device)
	: m_device(device)
	, m_exec(nullptr)
	, m_memory(nullptr)
	, m_state(nullptr)
	, m_disasm(nullptr)
	, m_flags(0)
	, m_symtable(&device, device.machine().debugger().cpu().get_global_symtable())
	, m_instrhook(nullptr)
	, m_stepaddr(0)
	, m_stepsleft(0)
	, m_stopaddr(0)
	, m_stoptime(attotime::zero)
	, m_stopirq(0)
	, m_stopexception(0)
	, m_endexectime(attotime::zero)
	, m_total_cycles(0)
	, m_last_total_cycles(0)
	, m_pc_history_index(0)
	, m_bplist(nullptr)
	, m_rplist(nullptr)
	, m_wpdata(0)
	, m_wpaddr(0)
	, m_trace(nullptr)
	, m_hotspot_threshhold(0)
	, m_track_pc_set()
	, m_track_pc(false)
	, m_comment_set()
	, m_comment_change(0)
	, m_track_mem_set()
	, m_track_mem(false)
{
	memset(m_pc_history, 0, sizeof(m_pc_history));
	memset(m_wplist, 0, sizeof(m_wplist));

	// find out which interfaces we have to work with
	device.interface(m_exec);
	device.interface(m_memory);
	device.interface(m_state);
	device.interface(m_disasm);

	// set up state-related stuff
	if (m_state != nullptr)
	{
		// add global symbol for cycles and totalcycles
		if (m_exec != nullptr)
		{
			m_symtable.add("cycles", nullptr, get_cycles);
			m_symtable.add("totalcycles", nullptr, get_totalcycles);
			m_symtable.add("lastinstructioncycles", nullptr, get_lastinstructioncycles);
		}

		// add entries to enable/disable unmap reporting for each space
		if (m_memory != nullptr)
		{
			if (m_memory->has_space(AS_PROGRAM))
				m_symtable.add("logunmap", (void *)&m_memory->space(AS_PROGRAM), get_logunmap, set_logunmap);
			if (m_memory->has_space(AS_DATA))
				m_symtable.add("logunmapd", (void *)&m_memory->space(AS_DATA), get_logunmap, set_logunmap);
			if (m_memory->has_space(AS_IO))
				m_symtable.add("logunmapi", (void *)&m_memory->space(AS_IO), get_logunmap, set_logunmap);

			m_symtable.add("wpaddr", symbol_table::READ_ONLY, &m_wpaddr);
			m_symtable.add("wpdata", symbol_table::READ_ONLY, &m_wpdata);
		}

		// add all registers into it
		std::string tempstr;
		for (auto &entry : m_state->state_entries())
		{
			strmakelower(tempstr.assign(entry->symbol()));
			m_symtable.add(tempstr.c_str(), (void *)(FPTR)entry->index(), get_state, set_state);
		}
	}

	// set up execution-related stuff
	if (m_exec != nullptr)
	{
		m_flags = DEBUG_FLAG_OBSERVING | DEBUG_FLAG_HISTORY;

		// if no curpc, add one
		if (m_state != nullptr && m_symtable.find("curpc") == nullptr)
			m_symtable.add("curpc", nullptr, get_current_pc);
	}
}


//-------------------------------------------------
//  ~device_debug - constructor
//-------------------------------------------------

device_debug::~device_debug()
{
	// free breakpoints and watchpoints
	breakpoint_clear_all();
	watchpoint_clear_all();
	registerpoint_clear_all();
}

//-------------------------------------------------
//  start_hook - the scheduler calls this hook
//  before beginning execution for the given device
//-------------------------------------------------

void device_debug::start_hook(const attotime &endtime)
{
	assert((m_device.machine().debug_flags & DEBUG_FLAG_ENABLED) != 0);

	m_device.machine().debugger().cpu().start_hook(&m_device, (m_flags & DEBUG_FLAG_STOP_VBLANK) != 0);

	// update the target execution end time
	m_endexectime = endtime;

	// recompute the debugging mode
	compute_debug_flags();
}


//-------------------------------------------------
//  stop_hook - the scheduler calls this hook when
//  ending execution for the given device
//-------------------------------------------------

void device_debug::stop_hook()
{
	m_device.machine().debugger().cpu().stop_hook(&m_device);
}


//-------------------------------------------------
//  interrupt_hook - called when an interrupt is
//  acknowledged
//-------------------------------------------------

void device_debug::interrupt_hook(int irqline)
{
	// see if this matches a pending interrupt request
	if ((m_flags & DEBUG_FLAG_STOP_INTERRUPT) != 0 && (m_stopirq == -1 || m_stopirq == irqline))
	{
		m_device.machine().debugger().cpu().set_execution_state(EXECUTION_STATE_STOPPED);
		m_device.machine().debugger().console().printf("Stopped on interrupt (CPU '%s', IRQ %d)\n", m_device.tag(), irqline);
		compute_debug_flags();
	}
}


//-------------------------------------------------
//  exception_hook - called when an exception is
//  generated
//-------------------------------------------------

void device_debug::exception_hook(int exception)
{
	// see if this matches a pending interrupt request
	if ((m_flags & DEBUG_FLAG_STOP_EXCEPTION) != 0 && (m_stopexception == -1 || m_stopexception == exception))
	{
		m_device.machine().debugger().cpu().set_execution_state(EXECUTION_STATE_STOPPED);
		m_device.machine().debugger().console().printf("Stopped on exception (CPU '%s', exception %d)\n", m_device.tag(), exception);
		compute_debug_flags();
	}
}


//-------------------------------------------------
//  instruction_hook - called by the CPU cores
//  before executing each instruction
//-------------------------------------------------

void device_debug::instruction_hook(offs_t curpc)
{
	running_machine &machine = m_device.machine();
	debugger_cpu& debugcpu = machine.debugger().cpu();

	// note that we are in the debugger code
	debugcpu.set_within_instruction(true);

	// update the history
	m_pc_history[m_pc_history_index++ % HISTORY_SIZE] = curpc;

	// update total cycles
	m_last_total_cycles = m_total_cycles;
	m_total_cycles = m_exec->total_cycles();

	// are we tracking our recent pc visits?
	if (m_track_pc)
	{
		const UINT32 crc = compute_opcode_crc32(curpc);
		m_track_pc_set.insert(dasm_pc_tag(curpc, crc));
	}

	// are we tracing?
	if (m_trace != nullptr)
		m_trace->update(curpc);

	// per-instruction hook?
	if (debugcpu.execution_state() != EXECUTION_STATE_STOPPED && (m_flags & DEBUG_FLAG_HOOKED) != 0 && (*m_instrhook)(m_device, curpc))
		debugcpu.set_execution_state(EXECUTION_STATE_STOPPED);

	// handle single stepping
	if (debugcpu.execution_state() != EXECUTION_STATE_STOPPED && (m_flags & DEBUG_FLAG_STEPPING_ANY) != 0)
	{
		// is this an actual step?
		if (m_stepaddr == ~0 || curpc == m_stepaddr)
		{
			// decrement the count and reset the breakpoint
			m_stepsleft--;
			m_stepaddr = ~0;

			// if we hit 0, stop
			if (m_stepsleft == 0)
				debugcpu.set_execution_state(EXECUTION_STATE_STOPPED);

			// update every 100 steps until we are within 200 of the end
			else if ((m_flags & DEBUG_FLAG_STEPPING_OUT) == 0 && (m_stepsleft < 200 || m_stepsleft % 100 == 0))
			{
				machine.debug_view().update_all();
				machine.debug_view().flush_osd_updates();
				machine.debugger().refresh_display();
			}
		}
	}

	// handle breakpoints
	if (debugcpu.execution_state() != EXECUTION_STATE_STOPPED && (m_flags & (DEBUG_FLAG_STOP_TIME | DEBUG_FLAG_STOP_PC | DEBUG_FLAG_LIVE_BP)) != 0)
	{
		// see if we hit a target time
		if ((m_flags & DEBUG_FLAG_STOP_TIME) != 0 && machine.time() >= m_stoptime)
		{
			machine.debugger().console().printf("Stopped at time interval %.1g\n", machine.time().as_double());
			debugcpu.set_execution_state(EXECUTION_STATE_STOPPED);
		}

		// check the temp running breakpoint and break if we hit it
		else if ((m_flags & DEBUG_FLAG_STOP_PC) != 0 && m_stopaddr == curpc)
		{
			machine.debugger().console().printf("Stopped at temporary breakpoint %X on CPU '%s'\n", m_stopaddr, m_device.tag());
			debugcpu.set_execution_state(EXECUTION_STATE_STOPPED);
		}

		// check for execution breakpoints
		else if ((m_flags & DEBUG_FLAG_LIVE_BP) != 0)
			breakpoint_check(curpc);
	}

	// if we are supposed to halt, do it now
	if (debugcpu.execution_state() == EXECUTION_STATE_STOPPED)
	{
		bool firststop = true;

		// load comments if we haven't yet
		debugcpu.ensure_comments_loaded();

		// reset any transient state
		debugcpu.reset_transient_flags();
		debugcpu.set_break_cpu(nullptr);

		// remember the last visible CPU in the debugger
		debugcpu.set_visible_cpu(&m_device);

		// update all views
		machine.debug_view().update_all();
		machine.debugger().refresh_display();

		// wait for the debugger; during this time, disable sound output
		m_device.machine().sound().debugger_mute(true);
		while (debugcpu.execution_state() == EXECUTION_STATE_STOPPED)
		{
			// flush any pending updates before waiting again
			machine.debug_view().flush_osd_updates();

			emulator_info::periodic_check();

			// clear the memory modified flag and wait
			debugcpu.set_memory_modified(false);
			if (machine.debug_flags & DEBUG_FLAG_OSD_ENABLED)
				machine.osd().wait_for_debugger(m_device, firststop);
			firststop = false;

			// if something modified memory, update the screen
			if (debugcpu.memory_modified())
			{
				machine.debug_view().update_all(DVT_DISASSEMBLY);
				machine.debugger().refresh_display();
			}

			// check for commands in the source file
			machine.debugger().cpu().process_source_file();

			// if an event got scheduled, resume
			if (machine.scheduled_event_pending())
				debugcpu.set_execution_state(EXECUTION_STATE_RUNNING);
		}
		m_device.machine().sound().debugger_mute(false);

		// remember the last visible CPU in the debugger
		debugcpu.set_visible_cpu(&m_device);
	}

	// handle step out/over on the instruction we are about to execute
	if ((m_flags & (DEBUG_FLAG_STEPPING_OVER | DEBUG_FLAG_STEPPING_OUT)) != 0 && m_stepaddr == ~0)
		prepare_for_step_overout(m_device.safe_pc());

	// no longer in debugger code
	debugcpu.set_within_instruction(false);
}


//-------------------------------------------------
//  memory_read_hook - the memory system calls
//  this hook when watchpoints are enabled and a
//  memory read happens
//-------------------------------------------------

void device_debug::memory_read_hook(address_space &space, offs_t address, UINT64 mem_mask)
{
	// check watchpoints
	watchpoint_check(space, WATCHPOINT_READ, address, 0, mem_mask);

	// check hotspots
	if (!m_hotspots.empty())
		hotspot_check(space, address);
}


//-------------------------------------------------
//  memory_write_hook - the memory system calls
//  this hook when watchpoints are enabled and a
//  memory write happens
//-------------------------------------------------

void device_debug::memory_write_hook(address_space &space, offs_t address, UINT64 data, UINT64 mem_mask)
{
	if (m_track_mem)
	{
		dasm_memory_access const newAccess(space.spacenum(), address, data, history_pc(0));
		std::pair<std::set<dasm_memory_access>::iterator, bool> trackedAccess = m_track_mem_set.insert(newAccess);
		if (!trackedAccess.second)
			trackedAccess.first->m_pc = newAccess.m_pc;
	}
	watchpoint_check(space, WATCHPOINT_WRITE, address, data, mem_mask);
}


//-------------------------------------------------
//  set_instruction_hook - set a hook to be
//  called on each instruction for a given device
//-------------------------------------------------

void device_debug::set_instruction_hook(debug_instruction_hook_func hook)
{
	// set the hook and also the CPU's flag for fast knowledge of the hook
	m_instrhook = hook;
	if (hook != nullptr)
		m_flags |= DEBUG_FLAG_HOOKED;
	else
		m_flags &= ~DEBUG_FLAG_HOOKED;
}


//-------------------------------------------------
//  ignore - ignore/observe a given device
//-------------------------------------------------

void device_debug::ignore(bool ignore)
{
	assert(m_exec != nullptr);

	if (ignore)
		m_flags &= ~DEBUG_FLAG_OBSERVING;
	else
		m_flags |= DEBUG_FLAG_OBSERVING;

	if (&m_device == m_device.machine().debugger().cpu().live_cpu() && ignore)
	{
		assert(m_exec != nullptr);
		go_next_device();
	}
}


//-------------------------------------------------
//  single_step - single step the device past the
//  requested number of instructions
//-------------------------------------------------

void device_debug::single_step(int numsteps)
{
	assert(m_exec != nullptr);

	m_stepsleft = numsteps;
	m_stepaddr = ~0;
	m_flags |= DEBUG_FLAG_STEPPING;
	m_device.machine().debugger().cpu().set_execution_state(EXECUTION_STATE_RUNNING);
}


//-------------------------------------------------
//  single_step_over - single step the device over
//  the requested number of instructions
//-------------------------------------------------

void device_debug::single_step_over(int numsteps)
{
	assert(m_exec != nullptr);

	m_stepsleft = numsteps;
	m_stepaddr = ~0;
	m_flags |= DEBUG_FLAG_STEPPING_OVER;
	m_device.machine().debugger().cpu().set_execution_state(EXECUTION_STATE_RUNNING);
}


//-------------------------------------------------
//  single_step_out - single step the device
//  out of the current function
//-------------------------------------------------

void device_debug::single_step_out()
{
	assert(m_exec != nullptr);

	m_stepsleft = 100;
	m_stepaddr = ~0;
	m_flags |= DEBUG_FLAG_STEPPING_OUT;
	m_device.machine().debugger().cpu().set_execution_state(EXECUTION_STATE_RUNNING);
}


//-------------------------------------------------
//  go - execute the device until it hits the given
//  address
//-------------------------------------------------

void device_debug::go(offs_t targetpc)
{
	assert(m_exec != nullptr);

	m_stopaddr = targetpc;
	m_flags |= DEBUG_FLAG_STOP_PC;
	m_device.machine().debugger().cpu().set_execution_state(EXECUTION_STATE_RUNNING);
}


//-------------------------------------------------
//  go_vblank - execute until the next VBLANK
//-------------------------------------------------

void device_debug::go_vblank()
{
	assert(m_exec != nullptr);

	m_flags |= DEBUG_FLAG_STOP_VBLANK;
	m_device.machine().debugger().cpu().go_vblank();
}


//-------------------------------------------------
//  go_interrupt - execute until the specified
//  interrupt fires on the device
//-------------------------------------------------

void device_debug::go_interrupt(int irqline)
{
	assert(m_exec != nullptr);

	m_stopirq = irqline;
	m_flags |= DEBUG_FLAG_STOP_INTERRUPT;
	m_device.machine().debugger().cpu().set_execution_state(EXECUTION_STATE_RUNNING);
}

void device_debug::go_next_device()
{
	m_device.machine().debugger().cpu().go_next_device(&m_device);
}

//-------------------------------------------------
//  go_exception - execute until the specified
//  exception fires on the visible CPU
//-------------------------------------------------

void device_debug::go_exception(int exception)
{
	assert(m_exec != nullptr);

	m_stopexception = exception;
	m_flags |= DEBUG_FLAG_STOP_EXCEPTION;
	m_device.machine().debugger().cpu().set_execution_state(EXECUTION_STATE_RUNNING);
}


//-------------------------------------------------
//  go_milliseconds - execute until the specified
//  delay elapses
//-------------------------------------------------

void device_debug::go_milliseconds(UINT64 milliseconds)
{
	assert(m_exec != nullptr);

	m_stoptime = m_device.machine().time() + attotime::from_msec(milliseconds);
	m_flags |= DEBUG_FLAG_STOP_TIME;
	m_device.machine().debugger().cpu().set_execution_state(EXECUTION_STATE_RUNNING);
}


//-------------------------------------------------
//  halt_on_next_instruction_impl - halt in the
//  debugger on the next instruction, internal
//  implementation which is necessary solely due
//  to templates in C++ being janky as all get out
//-------------------------------------------------

void device_debug::halt_on_next_instruction_impl(util::format_argument_pack<std::ostream> &&args)
{
	assert(m_exec != nullptr);
	m_device.machine().debugger().cpu().halt_on_next_instruction(&m_device, std::move(args));
}

//-------------------------------------------------
//  breakpoint_set - set a new breakpoint,
//  returning its index
//-------------------------------------------------

int device_debug::breakpoint_set(offs_t address, const char *condition, const char *action)
{
	// allocate a new one
	UINT32 id = m_device.machine().debugger().cpu().get_breakpoint_index();
	breakpoint *bp = auto_alloc(m_device.machine(), breakpoint(this, m_symtable, id, address, condition, action));

	// hook it into our list
	bp->m_next = m_bplist;
	m_bplist = bp;

	// update the flags and return the index
	breakpoint_update_flags();
	return bp->m_index;
}


//-------------------------------------------------
//  breakpoint_clear - clear a breakpoint by index,
//  returning true if we found it
//-------------------------------------------------

bool device_debug::breakpoint_clear(int index)
{
	// scan the list to see if we own this breakpoint
	for (breakpoint **bp = &m_bplist; *bp != nullptr; bp = &(*bp)->m_next)
		if ((*bp)->m_index == index)
		{
			breakpoint *deleteme = *bp;
			*bp = deleteme->m_next;
			auto_free(m_device.machine(), deleteme);
			breakpoint_update_flags();
			return true;
		}

	// we don't own it, return false
	return false;
}


//-------------------------------------------------
//  breakpoint_clear_all - clear all breakpoints
//-------------------------------------------------

void device_debug::breakpoint_clear_all()
{
	// clear the head until we run out
	while (m_bplist != nullptr)
		breakpoint_clear(m_bplist->index());
}


//-------------------------------------------------
//  breakpoint_enable - enable/disable a breakpoint
//  by index, returning true if we found it
//-------------------------------------------------

bool device_debug::breakpoint_enable(int index, bool enable)
{
	// scan the list to see if we own this breakpoint
	for (breakpoint *bp = m_bplist; bp != nullptr; bp = bp->next())
		if (bp->m_index == index)
		{
			bp->m_enabled = enable;
			breakpoint_update_flags();
			return true;
		}

	// we don't own it, return false
	return false;
}


//-------------------------------------------------
//  breakpoint_enable_all - enable/disable all
//  breakpoints
//-------------------------------------------------

void device_debug::breakpoint_enable_all(bool enable)
{
	// apply the enable to all breakpoints we own
	for (breakpoint *bp = m_bplist; bp != nullptr; bp = bp->next())
		breakpoint_enable(bp->index(), enable);
}


//-------------------------------------------------
//  watchpoint_set - set a new watchpoint,
//  returning its index
//-------------------------------------------------

int device_debug::watchpoint_set(address_space &space, int type, offs_t address, offs_t length, const char *condition, const char *action)
{
	assert(space.spacenum() < ARRAY_LENGTH(m_wplist));

	// allocate a new one
	UINT32 id = m_device.machine().debugger().cpu().get_watchpoint_index();
	watchpoint *wp = auto_alloc(m_device.machine(), watchpoint(this, m_symtable, id, space, type, address, length, condition, action));

	// hook it into our list
	wp->m_next = m_wplist[space.spacenum()];
	m_wplist[space.spacenum()] = wp;

	// update the flags and return the index
	watchpoint_update_flags(wp->m_space);
	return wp->m_index;
}


//-------------------------------------------------
//  watchpoint_clear - clear a watchpoint by index,
//  returning true if we found it
//-------------------------------------------------

bool device_debug::watchpoint_clear(int index)
{
	// scan the list to see if we own this breakpoint
	for (address_spacenum spacenum = AS_0; spacenum < ARRAY_LENGTH(m_wplist); ++spacenum)
		for (watchpoint **wp = &m_wplist[spacenum]; *wp != nullptr; wp = &(*wp)->m_next)
			if ((*wp)->m_index == index)
			{
				watchpoint *deleteme = *wp;
				address_space &space = deleteme->m_space;
				*wp = deleteme->m_next;
				auto_free(m_device.machine(), deleteme);
				watchpoint_update_flags(space);
				return true;
			}

	// we don't own it, return false
	return false;
}


//-------------------------------------------------
//  watchpoint_clear_all - clear all watchpoints
//-------------------------------------------------

void device_debug::watchpoint_clear_all()
{
	// clear the head until we run out
	for (address_spacenum spacenum = AS_0; spacenum < ARRAY_LENGTH(m_wplist); ++spacenum)
		while (m_wplist[spacenum] != nullptr)
			watchpoint_clear(m_wplist[spacenum]->index());
}


//-------------------------------------------------
//  watchpoint_enable - enable/disable a watchpoint
//  by index, returning true if we found it
//-------------------------------------------------

bool device_debug::watchpoint_enable(int index, bool enable)
{
	// scan the list to see if we own this watchpoint
	for (address_spacenum spacenum = AS_0; spacenum < ARRAY_LENGTH(m_wplist); ++spacenum)
		for (watchpoint *wp = m_wplist[spacenum]; wp != nullptr; wp = wp->next())
			if (wp->m_index == index)
			{
				wp->m_enabled = enable;
				watchpoint_update_flags(wp->m_space);
				return true;
			}

	// we don't own it, return false
	return false;
}


//-------------------------------------------------
//  watchpoint_enable_all - enable/disable all
//  watchpoints
//-------------------------------------------------

void device_debug::watchpoint_enable_all(bool enable)
{
	// apply the enable to all watchpoints we own
	for (address_spacenum spacenum = AS_0; spacenum < ARRAY_LENGTH(m_wplist); ++spacenum)
		for (watchpoint *wp = m_wplist[spacenum]; wp != nullptr; wp = wp->next())
			watchpoint_enable(wp->index(), enable);
}


//-------------------------------------------------
//  registerpoint_set - set a new registerpoint,
//  returning its index
//-------------------------------------------------

int device_debug::registerpoint_set(const char *condition, const char *action)
{
	// allocate a new one
	UINT32 id = m_device.machine().debugger().cpu().get_registerpoint_index();
	registerpoint *rp = auto_alloc(m_device.machine(), registerpoint(m_symtable, id, condition, action));

	// hook it into our list
	rp->m_next = m_rplist;
	m_rplist = rp;

	// update the flags and return the index
	breakpoint_update_flags();
	return rp->m_index;
}


//-------------------------------------------------
//  registerpoint_clear - clear a registerpoint by index,
//  returning true if we found it
//-------------------------------------------------

bool device_debug::registerpoint_clear(int index)
{
	// scan the list to see if we own this registerpoint
	for (registerpoint **rp = &m_rplist; *rp != nullptr; rp = &(*rp)->m_next)
		if ((*rp)->m_index == index)
		{
			registerpoint *deleteme = *rp;
			*rp = deleteme->m_next;
			auto_free(m_device.machine(), deleteme);
			breakpoint_update_flags();
			return true;
		}

	// we don't own it, return false
	return false;
}


//-------------------------------------------------
//  registerpoint_clear_all - clear all registerpoints
//-------------------------------------------------

void device_debug::registerpoint_clear_all()
{
	// clear the head until we run out
	while (m_rplist != nullptr)
		registerpoint_clear(m_rplist->index());
}


//-------------------------------------------------
//  registerpoint_enable - enable/disable a registerpoint
//  by index, returning true if we found it
//-------------------------------------------------

bool device_debug::registerpoint_enable(int index, bool enable)
{
	// scan the list to see if we own this conditionpoint
	for (registerpoint *rp = m_rplist; rp != nullptr; rp = rp->next())
		if (rp->m_index == index)
		{
			rp->m_enabled = enable;
			breakpoint_update_flags();
			return true;
		}

	// we don't own it, return false
	return false;
}


//-------------------------------------------------
//  registerpoint_enable_all - enable/disable all
//  registerpoints
//-------------------------------------------------

void device_debug::registerpoint_enable_all(bool enable)
{
	// apply the enable to all registerpoints we own
	for (registerpoint *rp = m_rplist; rp != nullptr; rp = rp->next())
		registerpoint_enable(rp->index(), enable);
}


//-------------------------------------------------
//  hotspot_track - enable/disable tracking of
//  hotspots
//-------------------------------------------------

void device_debug::hotspot_track(int numspots, int threshhold)
{
	// if we already have tracking enabled, kill it
	m_hotspots.clear();

	// only start tracking if we have a non-zero count
	if (numspots > 0)
	{
		// allocate memory for hotspots
		m_hotspots.resize(numspots);
		memset(&m_hotspots[0], 0xff, numspots*sizeof(m_hotspots[0]));

		// fill in the info
		m_hotspot_threshhold = threshhold;
	}

	// update the watchpoint flags to include us
	if (m_memory != nullptr && m_memory->has_space(AS_PROGRAM))
		watchpoint_update_flags(m_memory->space(AS_PROGRAM));
}


//-------------------------------------------------
//  history_pc - return an entry from the PC
//  history
//-------------------------------------------------

offs_t device_debug::history_pc(int index) const
{
	if (index > 0)
		index = 0;
	if (index <= -HISTORY_SIZE)
		index = -HISTORY_SIZE + 1;
	return m_pc_history[(m_pc_history_index + ARRAY_LENGTH(m_pc_history) - 1 + index) % ARRAY_LENGTH(m_pc_history)];
}


//-------------------------------------------------
//  track_pc_visited - returns a boolean stating
//  if this PC has been visited or not.  CRC32 is
//  done in this function on currently active CPU.
//  TODO: Take a CPU context as input
//-------------------------------------------------

bool device_debug::track_pc_visited(const offs_t& pc) const
{
	if (m_track_pc_set.empty())
		return false;
	const UINT32 crc = compute_opcode_crc32(pc);
	return m_track_pc_set.find(dasm_pc_tag(pc, crc)) != m_track_pc_set.end();
}


//-------------------------------------------------
//  set_track_pc_visited - set this pc as visited.
//  TODO: Take a CPU context as input
//-------------------------------------------------

void device_debug::set_track_pc_visited(const offs_t& pc)
{
	const UINT32 crc = compute_opcode_crc32(pc);
	m_track_pc_set.insert(dasm_pc_tag(pc, crc));
}


//-------------------------------------------------
//  track_mem_pc_from_address_data - returns the pc that
//  wrote the data to this address or (offs_t)(-1) for
//  'not available'.
//-------------------------------------------------

offs_t device_debug::track_mem_pc_from_space_address_data(const address_spacenum& space,
															const offs_t& address,
															const UINT64& data) const
{
	const offs_t missing = (offs_t)(-1);
	if (m_track_mem_set.empty())
		return missing;
	std::set<dasm_memory_access>::iterator const mem_access = m_track_mem_set.find(dasm_memory_access(space, address, data, 0));
	if (mem_access == m_track_mem_set.end()) return missing;
	return mem_access->m_pc;
}


//-------------------------------------------------
//  comment_add - adds a comment to the list at
//  the given address
//-------------------------------------------------

void device_debug::comment_add(offs_t addr, const char *comment, rgb_t color)
{
	// create a new item for the list
	UINT32 const crc = compute_opcode_crc32(addr);
	dasm_comment const newComment = dasm_comment(addr, crc, comment, color);
	std::pair<std::set<dasm_comment>::iterator, bool> const inserted = m_comment_set.insert(newComment);
	if (!inserted.second)
	{
		// Insert returns false if comment exists
		m_comment_set.erase(inserted.first);
		m_comment_set.insert(newComment);
	}

	// force an update
	m_comment_change++;
}


//-------------------------------------------------
//  comment_remove - removes a comment at the
//  given address with a matching CRC
//-------------------------------------------------

bool device_debug::comment_remove(offs_t addr)
{
	const UINT32 crc = compute_opcode_crc32(addr);
	size_t const removed = m_comment_set.erase(dasm_comment(addr, crc, "", 0xffffffff));
	if (removed != 0U) m_comment_change++;
	return removed != 0U;
}


//-------------------------------------------------
//  comment_text - return the text of a comment
//-------------------------------------------------

const char *device_debug::comment_text(offs_t addr) const
{
	const UINT32 crc = compute_opcode_crc32(addr);
	auto comment = m_comment_set.find(dasm_comment(addr, crc, "", 0));
	if (comment == m_comment_set.end()) return nullptr;
	return comment->m_text.c_str();
}


//-------------------------------------------------
//  comment_export - export the comments to the
//  given XML data node
//-------------------------------------------------

bool device_debug::comment_export(xml_data_node &curnode)
{
	// iterate through the comments
	for (const auto & elem : m_comment_set)
	{
		xml_data_node *datanode = xml_add_child(&curnode, "comment", xml_normalize_string(elem.m_text.c_str()));
		if (datanode == nullptr)
			return false;
		xml_set_attribute_int(datanode, "address", elem.m_address);
		xml_set_attribute_int(datanode, "color", elem.m_color);
		xml_set_attribute(datanode, "crc", string_format("%08X", elem.m_crc).c_str());
	}
	return true;
}


//-------------------------------------------------
//  comment_import - import the comments from the
//  given XML data node
//-------------------------------------------------

bool device_debug::comment_import(xml_data_node &cpunode,bool is_inline)
{
	// iterate through nodes
	for (xml_data_node *datanode = xml_get_sibling(cpunode.child, "comment"); datanode; datanode = xml_get_sibling(datanode->next, "comment"))
	{
		// extract attributes
		offs_t address = xml_get_attribute_int(datanode, "address", 0);
		rgb_t color = xml_get_attribute_int(datanode, "color", 0);

		UINT32 crc;
		sscanf(xml_get_attribute_string(datanode, "crc", nullptr), "%08X", &crc);

		// add the new comment
		if(is_inline == true)
			m_comment_set.insert(dasm_comment(address, crc, datanode->value, color));
		else
			m_device.machine().debugger().console().printf(" %08X - %s\n", address, datanode->value);
	}
	return true;
}


//-------------------------------------------------
//  compute_opcode_crc32 - determine the CRC of
//  the opcode bytes at the given address
//-------------------------------------------------

UINT32 device_debug::compute_opcode_crc32(offs_t pc) const
{
	// Basically the same thing as dasm_wrapped, but with some tiny savings
	assert(m_memory != nullptr);

	// determine the adjusted PC
	address_space &decrypted_space = m_memory->has_space(AS_DECRYPTED_OPCODES) ? m_memory->space(AS_DECRYPTED_OPCODES) : m_memory->space(AS_PROGRAM);
	address_space &space = m_memory->space(AS_PROGRAM);
	offs_t pcbyte = space.address_to_byte(pc) & space.bytemask();

	// fetch the bytes up to the maximum
	UINT8 opbuf[64], argbuf[64];
	int maxbytes = (m_disasm != nullptr) ? m_disasm->max_opcode_bytes() : 1;
	for (int numbytes = 0; numbytes < maxbytes; numbytes++)
	{
		opbuf[numbytes] = m_device.machine().debugger().cpu().read_opcode(decrypted_space, pcbyte + numbytes, 1);
		argbuf[numbytes] = m_device.machine().debugger().cpu().read_opcode(space, pcbyte + numbytes, 1);
	}

	UINT32 numbytes = maxbytes;
	if (m_disasm != nullptr)
	{
		// disassemble to our buffer
		char diasmbuf[200];
		memset(diasmbuf, 0x00, 200);
		numbytes = m_disasm->disassemble(diasmbuf, pc, opbuf, argbuf) & DASMFLAG_LENGTHMASK;
	}

	// return a CRC of the exact count of opcode bytes
	return core_crc32(0, opbuf, numbytes);
}


//-------------------------------------------------
//  trace - trace execution of a given device
//-------------------------------------------------

void device_debug::trace(FILE *file, bool trace_over, bool detect_loops, const char *action)
{
	// delete any existing tracers
	m_trace = nullptr;

	// if we have a new file, make a new tracer
	if (file != nullptr)
		m_trace = std::make_unique<tracer>(*this, *file, trace_over, detect_loops, action);
}


//-------------------------------------------------
//  trace_printf - output data into the given
//  device's tracefile, if tracing
//-------------------------------------------------

void device_debug::trace_printf(const char *fmt, ...)
{
	if (m_trace != nullptr)
	{
		va_list va;
		va_start(va, fmt);
		m_trace->vprintf(fmt, va);
		va_end(va);
	}
}


//-------------------------------------------------
//  compute_debug_flags - compute the global
//  debug flags for optimal efficiency
//-------------------------------------------------

void device_debug::compute_debug_flags()
{
	running_machine &machine = m_device.machine();
	debugger_cpu& debugcpu = machine.debugger().cpu();

	// clear out global flags by default, keep DEBUG_FLAG_OSD_ENABLED
	machine.debug_flags &= DEBUG_FLAG_OSD_ENABLED;
	machine.debug_flags |= DEBUG_FLAG_ENABLED;

	// if we are ignoring this CPU, or if events are pending, we're done
	if ((m_flags & DEBUG_FLAG_OBSERVING) == 0 || machine.scheduled_event_pending() || machine.save_or_load_pending())
		return;

	// if we're stopped, keep calling the hook
	if (debugcpu.execution_state() == EXECUTION_STATE_STOPPED)
		machine.debug_flags |= DEBUG_FLAG_CALL_HOOK;

	// if we're tracking history, or we're hooked, or stepping, or stopping at a breakpoint
	// make sure we call the hook
	if ((m_flags & (DEBUG_FLAG_HISTORY | DEBUG_FLAG_HOOKED | DEBUG_FLAG_STEPPING_ANY | DEBUG_FLAG_STOP_PC | DEBUG_FLAG_LIVE_BP)) != 0)
		machine.debug_flags |= DEBUG_FLAG_CALL_HOOK;

	// also call if we are tracing
	if (m_trace != nullptr)
		machine.debug_flags |= DEBUG_FLAG_CALL_HOOK;

	// if we are stopping at a particular time and that time is within the current timeslice, we need to be called
	if ((m_flags & DEBUG_FLAG_STOP_TIME) && m_endexectime <= m_stoptime)
		machine.debug_flags |= DEBUG_FLAG_CALL_HOOK;
}


//-------------------------------------------------
//  prepare_for_step_overout - prepare things for
//  stepping over an instruction
//-------------------------------------------------

void device_debug::prepare_for_step_overout(offs_t pc)
{
	// disassemble the current instruction and get the flags
	std::string dasmbuffer;
	offs_t dasmresult = dasm_wrapped(dasmbuffer, pc);

	// if flags are supported and it's a call-style opcode, set a temp breakpoint after that instruction
	if ((dasmresult & DASMFLAG_SUPPORTED) != 0 && (dasmresult & DASMFLAG_STEP_OVER) != 0)
	{
		int extraskip = (dasmresult & DASMFLAG_OVERINSTMASK) >> DASMFLAG_OVERINSTSHIFT;
		pc += dasmresult & DASMFLAG_LENGTHMASK;

		// if we need to skip additional instructions, advance as requested
		while (extraskip-- > 0)
			pc += dasm_wrapped(dasmbuffer, pc) & DASMFLAG_LENGTHMASK;
		m_stepaddr = pc;
	}

	// if we're stepping out and this isn't a step out instruction, reset the steps until stop to a high number
	if ((m_flags & DEBUG_FLAG_STEPPING_OUT) != 0)
	{
		if ((dasmresult & DASMFLAG_SUPPORTED) != 0 && (dasmresult & DASMFLAG_STEP_OUT) == 0)
			m_stepsleft = 100;
		else
			m_stepsleft = 1;
	}
}


//-------------------------------------------------
//  breakpoint_update_flags - update the device's
//  breakpoint flags
//-------------------------------------------------

void device_debug::breakpoint_update_flags()
{
	// see if there are any enabled breakpoints
	m_flags &= ~DEBUG_FLAG_LIVE_BP;
	for (breakpoint *bp = m_bplist; bp != nullptr; bp = bp->m_next)
		if (bp->m_enabled)
		{
			m_flags |= DEBUG_FLAG_LIVE_BP;
			break;
		}

	if ( ! ( m_flags & DEBUG_FLAG_LIVE_BP ) )
	{
		// see if there are any enabled registerpoints
		for (registerpoint *rp = m_rplist; rp != nullptr; rp = rp->m_next)
		{
			if (rp->m_enabled)
			{
				m_flags |= DEBUG_FLAG_LIVE_BP;
			}
		}
	}

	// push the flags out globally
	if (m_device.machine().debugger().cpu().live_cpu() != nullptr)
		m_device.machine().debugger().cpu().live_cpu()->debug()->compute_debug_flags();
}


//-------------------------------------------------
//  breakpoint_check - check the breakpoints for
//  a given device
//-------------------------------------------------

void device_debug::breakpoint_check(offs_t pc)
{
	debugger_cpu& debugcpu = m_device.machine().debugger().cpu();

	// see if we match
	for (breakpoint *bp = m_bplist; bp != nullptr; bp = bp->m_next)
		if (bp->hit(pc))
		{
			// halt in the debugger by default
			debugcpu.set_execution_state(EXECUTION_STATE_STOPPED);

			// if we hit, evaluate the action
			if (!bp->m_action.empty())
				m_device.machine().debugger().console().execute_command(bp->m_action.c_str(), false);

			// print a notification, unless the action made us go again
			if (debugcpu.execution_state() == EXECUTION_STATE_STOPPED)
				m_device.machine().debugger().console().printf("Stopped at breakpoint %X\n", bp->m_index);
			break;
		}

	// see if we have any matching registerpoints
	for (registerpoint *rp = m_rplist; rp != nullptr; rp = rp->m_next)
	{
		if (rp->hit())
		{
			// halt in the debugger by default
			debugcpu.set_execution_state(EXECUTION_STATE_STOPPED);

			// if we hit, evaluate the action
			if (!rp->m_action.empty())
			{
				m_device.machine().debugger().console().execute_command(rp->m_action.c_str(), false);
			}

			// print a notification, unless the action made us go again
			if (debugcpu.execution_state() == EXECUTION_STATE_STOPPED)
			{
				m_device.machine().debugger().console().printf("Stopped at registerpoint %X\n", rp->m_index);
			}
			break;
		}
	}
}


//-------------------------------------------------
//  watchpoint_update_flags - update the device's
//  watchpoint flags
//-------------------------------------------------

void device_debug::watchpoint_update_flags(address_space &space)
{
	// if hotspots are enabled, turn on all reads
	bool enableread = false;
	if (!m_hotspots.empty())
		enableread = true;

	// see if there are any enabled breakpoints
	bool enablewrite = false;
	for (watchpoint *wp = m_wplist[space.spacenum()]; wp != nullptr; wp = wp->m_next)
		if (wp->m_enabled)
		{
			if (wp->m_type & WATCHPOINT_READ)
				enableread = true;
			if (wp->m_type & WATCHPOINT_WRITE)
				enablewrite = true;
		}

	// push the flags out globally
	space.enable_read_watchpoints(enableread);
	space.enable_write_watchpoints(enablewrite);
}


//-------------------------------------------------
//  watchpoint_check - check the watchpoints
//  for a given CPU and address space
//-------------------------------------------------

void device_debug::watchpoint_check(address_space& space, int type, offs_t address, UINT64 value_to_write, UINT64 mem_mask)
{
	debugger_cpu &debugcpu = m_device.machine().debugger().cpu();

	// if we're within debugger code, don't stop
	if (debugcpu.within_instruction_hook() || debugcpu.accessing_memory())
		return;

	debugcpu.set_within_instruction(true);

	// adjust address, size & value_to_write based on mem_mask.
	offs_t size = 0;
	if (mem_mask != 0)
	{
		int bus_size = space.data_width() / 8;
		int address_offset = 0;

		while (address_offset < bus_size && (mem_mask & 0xff) == 0)
		{
			address_offset++;
			value_to_write >>= 8;
			mem_mask >>= 8;
		}

		while (mem_mask != 0)
		{
			size++;
			mem_mask >>= 8;
		}

		// (1<<(size*8))-1 won't work when size is 8; let's just use a lut
		static const UINT64 masks[] = {0,
										0xff,
										0xffff,
										0xffffff,
										0xffffffff,
									U64(0xffffffffff),
									U64(0xffffffffffff),
									U64(0xffffffffffffff),
									U64(0xffffffffffffffff)};
		value_to_write &= masks[size];

		if (space.endianness() == ENDIANNESS_LITTLE)
			address += address_offset;
		else
			address += bus_size - size - address_offset;
	}

	// if we are a write watchpoint, stash the value that will be written
	m_wpaddr = address;
	if (type & WATCHPOINT_WRITE)
		m_wpdata = value_to_write;

	// see if we match
	for (watchpoint *wp = m_wplist[space.spacenum()]; wp != nullptr; wp = wp->next())
		if (wp->hit(type, address, size))
		{
			// halt in the debugger by default
			debugcpu.set_execution_state(EXECUTION_STATE_STOPPED);

			// if we hit, evaluate the action
			if (strlen(wp->action()) > 0)
				m_device.machine().debugger().console().execute_command(wp->action(), false);

			// print a notification, unless the action made us go again
			if (debugcpu.is_stopped())
			{
				static const char *const sizes[] =
				{
					"0bytes", "byte", "word", "3bytes", "dword", "5bytes", "6bytes", "7bytes", "qword"
				};
				offs_t pc = m_device.safe_pc();
				std::string buffer;

				if (type & WATCHPOINT_WRITE)
				{
					buffer = string_format("Stopped at watchpoint %X writing %s to %08X (PC=%X)", wp->index(), sizes[size], space.byte_to_address(address), pc);
					if (value_to_write >> 32)
						buffer.append(string_format(" (data=%X%08X)", (UINT32)(value_to_write >> 32), (UINT32)value_to_write));
					else
						buffer.append(string_format(" (data=%X)", (UINT32)value_to_write));
				}
				else
					buffer = string_format("Stopped at watchpoint %X reading %s from %08X (PC=%X)", wp->index(), sizes[size], space.byte_to_address(address), pc);
				m_device.machine().debugger().console().printf("%s\n", buffer.c_str());
				compute_debug_flags();
			}
			break;
		}

	debugcpu.set_within_instruction(false);
}


//-------------------------------------------------
//  hotspot_check - check for hotspots on a
//  memory read access
//-------------------------------------------------

void device_debug::hotspot_check(address_space &space, offs_t address)
{
	offs_t curpc = m_device.safe_pc();

	// see if we have a match in our list
	unsigned int hotindex;
	for (hotindex = 0; hotindex < m_hotspots.size(); hotindex++)
		if (m_hotspots[hotindex].m_access == address && m_hotspots[hotindex].m_pc == curpc && m_hotspots[hotindex].m_space == &space)
			break;

	// if we didn't find any, make a new entry
	if (hotindex == m_hotspots.size())
	{
		// if the bottom of the list is over the threshold, print it
		hotspot_entry &spot = m_hotspots[m_hotspots.size() - 1];
		if (spot.m_count > m_hotspot_threshhold)
			space.machine().debugger().console().printf("Hotspot @ %s %08X (PC=%08X) hit %d times (fell off bottom)\n", space.name(), spot.m_access, spot.m_pc, spot.m_count);

		// move everything else down and insert this one at the top
		memmove(&m_hotspots[1], &m_hotspots[0], sizeof(m_hotspots[0]) * (m_hotspots.size() - 1));
		m_hotspots[0].m_access = address;
		m_hotspots[0].m_pc = curpc;
		m_hotspots[0].m_space = &space;
		m_hotspots[0].m_count = 1;
	}

	// if we did find one, increase the count and move it to the top
	else
	{
		m_hotspots[hotindex].m_count++;
		if (hotindex != 0)
		{
			hotspot_entry temp = m_hotspots[hotindex];
			memmove(&m_hotspots[1], &m_hotspots[0], sizeof(m_hotspots[0]) * hotindex);
			m_hotspots[0] = temp;
		}
	}
}


//-------------------------------------------------
//  dasm_wrapped - wraps calls to the disassembler
//  by fetching the opcode bytes to a temporary
//  buffer and then disassembling them
//-------------------------------------------------

UINT32 device_debug::dasm_wrapped(std::string &buffer, offs_t pc)
{
	assert(m_memory != nullptr && m_disasm != nullptr);

	// determine the adjusted PC
	address_space &decrypted_space = m_memory->has_space(AS_DECRYPTED_OPCODES) ? m_memory->space(AS_DECRYPTED_OPCODES) : m_memory->space(AS_PROGRAM);
	address_space &space = m_memory->space(AS_PROGRAM);
	offs_t pcbyte = space.address_to_byte(pc) & space.bytemask();

	// fetch the bytes up to the maximum
	UINT8 opbuf[64], argbuf[64];
	int maxbytes = m_disasm->max_opcode_bytes();
	for (int numbytes = 0; numbytes < maxbytes; numbytes++)
	{
		opbuf[numbytes] = m_device.machine().debugger().cpu().read_opcode(decrypted_space, pcbyte + numbytes, 1);
		argbuf[numbytes] = m_device.machine().debugger().cpu().read_opcode(space, pcbyte + numbytes, 1);
	}

	// disassemble to our buffer
	char diasmbuf[200];
	memset(diasmbuf, 0x00, 200);
	UINT32 result = m_disasm->disassemble(diasmbuf, pc, opbuf, argbuf);
	buffer.assign(diasmbuf);
	return result;
}


//-------------------------------------------------
//  get_current_pc - getter callback for a device's
//  current instruction pointer
//-------------------------------------------------

UINT64 device_debug::get_current_pc(symbol_table &table, void *ref)
{
	device_t *device = reinterpret_cast<device_t *>(table.globalref());
	return device->safe_pc();
}


//-------------------------------------------------
//  get_cycles - getter callback for the
//  'cycles' symbol
//-------------------------------------------------

UINT64 device_debug::get_cycles(symbol_table &table, void *ref)
{
	device_t *device = reinterpret_cast<device_t *>(table.globalref());
	return device->debug()->m_exec->cycles_remaining();
}


//-------------------------------------------------
//  get_totalcycles - getter callback for the
//  'totalcycles' symbol
//-------------------------------------------------

UINT64 device_debug::get_totalcycles(symbol_table &table, void *ref)
{
	device_t *device = reinterpret_cast<device_t *>(table.globalref());
	return device->debug()->m_total_cycles;
}


//-------------------------------------------------
//  get_lastinstructioncycles - getter callback for the
//  'lastinstructioncycles' symbol
//-------------------------------------------------

UINT64 device_debug::get_lastinstructioncycles(symbol_table &table, void *ref)
{
	device_t *device = reinterpret_cast<device_t *>(table.globalref());
	device_debug *debug = device->debug();
	return debug->m_total_cycles - debug->m_last_total_cycles;
}


//-------------------------------------------------
//  get_logunmap - getter callback for the logumap
//  symbols
//-------------------------------------------------

UINT64 device_debug::get_logunmap(symbol_table &table, void *ref)
{
	address_space &space = *reinterpret_cast<address_space *>(table.globalref());
	return space.log_unmap();
}


//-------------------------------------------------
//  set_logunmap - setter callback for the logumap
//  symbols
//-------------------------------------------------

void device_debug::set_logunmap(symbol_table &table, void *ref, UINT64 value)
{
	address_space &space = *reinterpret_cast<address_space *>(table.globalref());
	space.set_log_unmap(value ? true : false);
}


//-------------------------------------------------
//  get_state - getter callback for a device's
//  state symbols
//-------------------------------------------------

UINT64 device_debug::get_state(symbol_table &table, void *ref)
{
	device_t *device = reinterpret_cast<device_t *>(table.globalref());
	return device->debug()->m_state->state_int(reinterpret_cast<FPTR>(ref));
}


//-------------------------------------------------
//  set_state - setter callback for a device's
//  state symbols
//-------------------------------------------------

void device_debug::set_state(symbol_table &table, void *ref, UINT64 value)
{
	device_t *device = reinterpret_cast<device_t *>(table.globalref());
	device->debug()->m_state->set_state_int(reinterpret_cast<FPTR>(ref), value);
}



//**************************************************************************
//  DEBUG BREAKPOINT
//**************************************************************************

//-------------------------------------------------
//  breakpoint - constructor
//-------------------------------------------------

device_debug::breakpoint::breakpoint(device_debug* debugInterface,
										symbol_table &symbols,
										int index,
										offs_t address,
										const char *condition,
										const char *action)
	: m_debugInterface(debugInterface),
		m_next(nullptr),
		m_index(index),
		m_enabled(true),
		m_address(address),
		m_condition(&symbols, (condition != nullptr) ? condition : "1"),
		m_action((action != nullptr) ? action : "")
{
}


//-------------------------------------------------
//  hit - detect a hit
//-------------------------------------------------

bool device_debug::breakpoint::hit(offs_t pc)
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
		catch (expression_error &)
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
//  watchpoint - constructor
//-------------------------------------------------

device_debug::watchpoint::watchpoint(device_debug* debugInterface,
										symbol_table &symbols,
										int index,
										address_space &space,
										int type,
										offs_t address,
										offs_t length,
										const char *condition,
										const char *action)
	: m_debugInterface(debugInterface),
		m_next(nullptr),
		m_space(space),
		m_index(index),
		m_enabled(true),
		m_type(type),
		m_address(space.address_to_byte(address) & space.bytemask()),
		m_length(space.address_to_byte(length)),
		m_condition(&symbols, (condition != nullptr) ? condition : "1"),
		m_action((action != nullptr) ? action : "")
{
}


//-------------------------------------------------
//  hit - detect a hit
//-------------------------------------------------

bool device_debug::watchpoint::hit(int type, offs_t address, int size)
{
	// don't hit if disabled
	if (!m_enabled)
		return false;

	// must match the type
	if ((m_type & type) == 0)
		return false;

	// must match our address
	if (address + size <= m_address || address >= m_address + m_length)
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
//  DEBUG REGISTERPOINT
//**************************************************************************

//-------------------------------------------------
//  registerpoint - constructor
//-------------------------------------------------

device_debug::registerpoint::registerpoint(symbol_table &symbols, int index, const char *condition, const char *action)
	: m_next(nullptr),
		m_index(index),
		m_enabled(true),
		m_condition(&symbols, (condition != nullptr) ? condition : "1"),
		m_action((action != nullptr) ? action : "")
{
}


//-------------------------------------------------
//  hit - detect a hit
//-------------------------------------------------

bool device_debug::registerpoint::hit()
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
//  TRACER
//**************************************************************************

//-------------------------------------------------
//  tracer - constructor
//-------------------------------------------------

device_debug::tracer::tracer(device_debug &debug, FILE &file, bool trace_over, bool detect_loops, const char *action)
	: m_debug(debug)
	, m_file(file)
	, m_action((action != nullptr) ? action : "")
	, m_detect_loops(detect_loops)
	, m_loops(0)
	, m_nextdex(0)
	, m_trace_over(trace_over)
	, m_trace_over_target(~0)
{
	memset(m_history, 0, sizeof(m_history));
}


//-------------------------------------------------
//  ~tracer - destructor
//-------------------------------------------------

device_debug::tracer::~tracer()
{
	// make sure we close the file if we can
	fclose(&m_file);
}


//-------------------------------------------------
//  update - log to the tracefile the data for a
//  given instruction
//-------------------------------------------------

void device_debug::tracer::update(offs_t pc)
{
	// are we in trace over mode and in a subroutine?
	if (m_trace_over && m_trace_over_target != ~0)
	{
		if (m_trace_over_target != pc)
			return;
		m_trace_over_target = ~0;
	}

	if (m_detect_loops)
	{
		// check for a loop condition
		int count = 0;
		for (auto & elem : m_history)
			if (elem == pc)
				count++;

		// if more than 1 hit, just up the loop count and get out
		if (count > 1)
		{
			m_loops++;
			return;
		}

		// if we just finished looping, indicate as much
		if (m_loops != 0)
			fprintf(&m_file, "\n   (loops for %d instructions)\n\n", m_loops);
		m_loops = 0;
	}

	// execute any trace actions first
	if (!m_action.empty())
		m_debug.m_device.machine().debugger().console().execute_command(m_action.c_str(), false);

	// print the address
	std::string buffer;
	int logaddrchars = m_debug.logaddrchars();
	buffer = string_format("%0*X: ", logaddrchars, pc);

	// print the disassembly
	std::string dasm;
	offs_t dasmresult = m_debug.dasm_wrapped(dasm, pc);
	buffer.append(dasm);

	// output the result
	fprintf(&m_file, "%s\n", buffer.c_str());

	// do we need to step the trace over this instruction?
	if (m_trace_over && (dasmresult & DASMFLAG_SUPPORTED) != 0 && (dasmresult & DASMFLAG_STEP_OVER) != 0)
	{
		int extraskip = (dasmresult & DASMFLAG_OVERINSTMASK) >> DASMFLAG_OVERINSTSHIFT;
		offs_t trace_over_target = pc + (dasmresult & DASMFLAG_LENGTHMASK);

		// if we need to skip additional instructions, advance as requested
		while (extraskip-- > 0)
			trace_over_target += m_debug.dasm_wrapped(dasm, trace_over_target) & DASMFLAG_LENGTHMASK;

		m_trace_over_target = trace_over_target;
	}

	// log this PC
	m_nextdex = (m_nextdex + 1) % TRACE_LOOPS;
	m_history[m_nextdex] = pc;
}


//-------------------------------------------------
//  vprintf - generic print to the trace file
//-------------------------------------------------

void device_debug::tracer::vprintf(const char *format, va_list va)
{
	// pass through to the file
	vfprintf(&m_file, format, va);
}


//-------------------------------------------------
//  flush - flush any pending changes to the trace
//  file
//-------------------------------------------------

void device_debug::tracer::flush()
{
	fflush(&m_file);
}


//-------------------------------------------------
//  dasm_pc_tag - constructor
//-------------------------------------------------

device_debug::dasm_pc_tag::dasm_pc_tag(const offs_t& address, const UINT32& crc)
	: m_address(address),
		m_crc(crc)
{
}

//-------------------------------------------------
//  dasm_memory_access - constructor
//-------------------------------------------------

device_debug::dasm_memory_access::dasm_memory_access(const address_spacenum& address_space,
														const offs_t& address,
														const UINT64& data,
														const offs_t& pc)
	: m_address_space(address_space),
		m_address(address),
		m_data(data),
		m_pc(pc)
{
}

//-------------------------------------------------
//  dasm_comment - constructor
//-------------------------------------------------

device_debug::dasm_comment::dasm_comment(offs_t address, UINT32 crc, const char *text, rgb_t color)
	: dasm_pc_tag(address, crc),
		m_text(text),
		m_color(std::move(color))
{
}
