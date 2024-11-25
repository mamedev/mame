// license:BSD-3-Clause
// copyright-holders:Andrew Gardner, Vas Crabb
/*********************************************************************

    dvbpoints.cpp

    Breakpoint debugger view.

***************************************************************************/

#include "emu.h"
#include "dvbpoints.h"

#include "debugcpu.h"
#include "points.h"

#include <algorithm>
#include <iomanip>



// Sorting functors for the qsort function
static bool cIndexAscending(const debug_breakpoint *a, const debug_breakpoint *b)
{
	return a->index() < b->index();
}

static bool cIndexDescending(const debug_breakpoint *a, const debug_breakpoint *b)
{
	return cIndexAscending(b, a);
}

static bool cEnabledAscending(const debug_breakpoint *a, const debug_breakpoint *b)
{
	return !a->enabled() && b->enabled();
}

static bool cEnabledDescending(const debug_breakpoint *a, const debug_breakpoint *b)
{
	return cEnabledAscending(b, a);
}

static bool cCpuAscending(const debug_breakpoint *a, const debug_breakpoint *b)
{
	return strcmp(a->debugInterface()->device().tag(), b->debugInterface()->device().tag()) < 0;
}

static bool cCpuDescending(const debug_breakpoint *a, const debug_breakpoint *b)
{
	return cCpuAscending(b, a);
}

static bool cAddressAscending(const debug_breakpoint *a, const debug_breakpoint *b)
{
	return a->address() < b->address();
}

static bool cAddressDescending(const debug_breakpoint *a, const debug_breakpoint *b)
{
	return cAddressAscending(b, a);
}

static bool cConditionAscending(const debug_breakpoint *a, const debug_breakpoint *b)
{
	return strcmp(a->condition(), b->condition()) < 0;
}

static bool cConditionDescending(const debug_breakpoint *a, const debug_breakpoint *b)
{
	return cConditionAscending(b, a);
}

static bool cActionAscending(const debug_breakpoint *a, const debug_breakpoint *b)
{
	return a->action() < b->action();
}

static bool cActionDescending(const debug_breakpoint *a, const debug_breakpoint *b)
{
	return cActionAscending(b, a);
}


//**************************************************************************
//  DEBUG VIEW BREAK POINTS
//**************************************************************************

static const int tableBreaks[] = { 5, 9, 31, 45, 63, 80 };


//-------------------------------------------------
//  debug_view_breakpoints - constructor
//-------------------------------------------------

debug_view_breakpoints::debug_view_breakpoints(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate)
	: debug_view(machine, DVT_BREAK_POINTS, osdupdate, osdprivate)
	, m_sortType(cIndexAscending)
{
	// fail if no available sources
	enumerate_sources();
	if (m_source_list.empty())
		throw std::bad_alloc();
}


//-------------------------------------------------
//  ~debug_view_breakpoints - destructor
//-------------------------------------------------

debug_view_breakpoints::~debug_view_breakpoints()
{
}


//-------------------------------------------------
//  enumerate_sources - enumerate all possible
//  sources for a disassembly view
//-------------------------------------------------

void debug_view_breakpoints::enumerate_sources()
{
	// start with an empty list
	m_source_list.clear();

	// iterate over devices with disassembly interfaces
	for (device_disasm_interface &dasm : disasm_interface_enumerator(machine().root_device()))
	{
		m_source_list.emplace_back(
				std::make_unique<debug_view_source>(
					util::string_format("%s '%s'", dasm.device().name(), dasm.device().tag()),
					&dasm.device()));
	}

	// reset the source to a known good entry
	if (!m_source_list.empty())
		set_source(*m_source_list[0]);
}


//-------------------------------------------------
//  view_click - handle a mouse click within the
//  current view
//-------------------------------------------------

void debug_view_breakpoints::view_click(const int button, const debug_view_xy& pos)
{
	bool clickedTopRow = (m_topleft.y == pos.y);

	if (clickedTopRow)
	{
		if (pos.x < tableBreaks[0])
			m_sortType = (m_sortType == &cIndexAscending) ? &cIndexDescending : &cIndexAscending;
		else if (pos.x < tableBreaks[1])
			m_sortType = (m_sortType == &cEnabledAscending) ? &cEnabledDescending : &cEnabledAscending;
		else if (pos.x < tableBreaks[2])
			m_sortType = (m_sortType == &cCpuAscending) ? &cCpuDescending : &cCpuAscending;
		else if (pos.x < tableBreaks[3])
			m_sortType = (m_sortType == &cAddressAscending) ? &cAddressDescending : &cAddressAscending;
		else if (pos.x < tableBreaks[4])
			m_sortType = (m_sortType == &cConditionAscending) ? &cConditionDescending : &cConditionAscending;
		else if (pos.x < tableBreaks[5])
			m_sortType = (m_sortType == &cActionAscending) ? &cActionDescending : &cActionAscending;
	}
	else
	{
		// Gather a sorted list of all the breakpoints for all the CPUs
		gather_breakpoints();

		int bpIndex = pos.y - 1;
		if ((bpIndex >= m_buffer.size()) || (bpIndex < 0))
			return;

		// Enable / disable
		const_cast<debug_breakpoint &>(*m_buffer[bpIndex]).setEnabled(!m_buffer[bpIndex]->enabled());

		machine().debug_view().update_all(DVT_DISASSEMBLY);
	}

	begin_update();
	m_update_pending = true;
	end_update();
}


void debug_view_breakpoints::pad_ostream_to_length(std::ostream& str, int len)
{
	auto const current = str.tellp();
	if (current < decltype(current)(len))
		str << std::setw(decltype(current)(len) - current) << "";
}


void debug_view_breakpoints::gather_breakpoints()
{
	m_buffer.resize(0);
	for (auto &source : m_source_list)
	{
		// Collect
		device_debug &debugInterface = *source->device()->debug();
		for (const auto &bpp : debugInterface.breakpoint_list())
			m_buffer.push_back(bpp.second.get());
	}

	// And now for the sort
	if (!m_buffer.empty())
		std::stable_sort(m_buffer.begin(), m_buffer.end(), m_sortType);
}


//-------------------------------------------------
//  view_update - update the contents of the
//  breakpoints view
//-------------------------------------------------

void debug_view_breakpoints::view_update()
{
	// Gather a list of all the breakpoints for all the CPUs
	gather_breakpoints();

	// Set the view region so the scroll bars update
	m_total.x = tableBreaks[std::size(tableBreaks) - 1];
	m_total.y = m_buffer.size() + 1;
	if (m_total.y < 10)
		m_total.y = 10;

	// Draw
	debug_view_char     *dest = &m_viewdata[0];
	util::ovectorstream linebuf;
	linebuf.reserve(std::size(tableBreaks) - 1);

	// Header
	if (m_visible.y > 0)
	{
		linebuf.clear();
		linebuf.rdbuf()->clear();
		linebuf << "ID";
		if (m_sortType == &cIndexAscending) linebuf.put('\\');
		else if (m_sortType == &cIndexDescending) linebuf.put('/');
		pad_ostream_to_length(linebuf, tableBreaks[0]);
		linebuf << "En";
		if (m_sortType == &cEnabledAscending) linebuf.put('\\');
		else if (m_sortType == &cEnabledDescending) linebuf.put('/');
		pad_ostream_to_length(linebuf, tableBreaks[1]);
		linebuf << "CPU";
		if (m_sortType == &cCpuAscending) linebuf.put('\\');
		else if (m_sortType == &cCpuDescending) linebuf.put('/');
		pad_ostream_to_length(linebuf, tableBreaks[2]);
		linebuf << "Address";
		if (m_sortType == &cAddressAscending) linebuf.put('\\');
		else if (m_sortType == &cAddressDescending) linebuf.put('/');
		pad_ostream_to_length(linebuf, tableBreaks[3]);
		linebuf << "Condition";
		if (m_sortType == &cConditionAscending) linebuf.put('\\');
		else if (m_sortType == &cConditionDescending) linebuf.put('/');
		pad_ostream_to_length(linebuf, tableBreaks[4]);
		linebuf << "Action";
		if (m_sortType == &cActionAscending) linebuf.put('\\');
		else if (m_sortType == &cActionDescending) linebuf.put('/');
		pad_ostream_to_length(linebuf, tableBreaks[5]);

		auto const &text(linebuf.vec());
		for (u32 i = m_topleft.x; i < (m_topleft.x + m_visible.x); i++, dest++)
		{
			dest->byte = (i < text.size()) ? text[i] : ' ';
			dest->attrib = DCA_ANCILLARY;
		}
	}

	for (int row = 1; row < m_visible.y; row++)
	{
		// Breakpoints
		int bpi = row + m_topleft.y - 1;
		if ((bpi < m_buffer.size()) && (bpi >= 0))
		{
			const debug_breakpoint *const bp = m_buffer[bpi];

			linebuf.clear();
			linebuf.rdbuf()->clear();
			util::stream_format(linebuf, "%2X", bp->index());
			pad_ostream_to_length(linebuf, tableBreaks[0]);
			linebuf.put(bp->enabled() ? 'X' : 'O');
			pad_ostream_to_length(linebuf, tableBreaks[1]);
			linebuf << bp->debugInterface()->device().tag();
			pad_ostream_to_length(linebuf, tableBreaks[2]);
			util::stream_format(linebuf, "%0*X", bp->debugInterface()->logaddrchars(), bp->address());
			pad_ostream_to_length(linebuf, tableBreaks[3]);
			if (strcmp(bp->condition(), "1"))
				linebuf << bp->condition();
			pad_ostream_to_length(linebuf, tableBreaks[4]);
			linebuf << bp->action();
			pad_ostream_to_length(linebuf, tableBreaks[5]);

			auto const &text(linebuf.vec());
			for (u32 i = m_topleft.x; i < (m_topleft.x + m_visible.x); i++, dest++)
			{
				dest->byte = (i < text.size()) ? text[i] : ' ';
				dest->attrib = DCA_NORMAL;

				// Color disabled breakpoints red
				if ((i >= tableBreaks[0]) && (i < tableBreaks[1]) && !bp->enabled())
					dest->attrib |= DCA_CHANGED;
			}
		}
		else
		{
			// Fill the remaining vertical space
			for (u32 i = m_topleft.x; i < (m_topleft.x + m_visible.x); i++, dest++)
			{
				dest->byte = ' ';
				dest->attrib = DCA_NORMAL;
			}
		}
	}
}
