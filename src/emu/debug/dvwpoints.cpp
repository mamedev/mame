// license:BSD-3-Clause
// copyright-holders:Andrew Gardner, Vas Crabb
/*********************************************************************

    dvwpoints.cpp

    Watchpoint debugger view.

***************************************************************************/

#include "emu.h"
#include "dvwpoints.h"

#include "debugcpu.h"
#include "points.h"

#include <algorithm>
#include <iomanip>



static bool cIndexAscending(const debug_watchpoint *a, const debug_watchpoint *b)
{
	return a->index() < b->index();
}

static bool cIndexDescending(const debug_watchpoint *a, const debug_watchpoint *b)
{
	return cIndexAscending(b, a);
}

static bool cEnabledAscending(const debug_watchpoint *a, const debug_watchpoint *b)
{
	return !a->enabled() && b->enabled();
}

static bool cEnabledDescending(const debug_watchpoint *a, const debug_watchpoint *b)
{
	return cEnabledAscending(b, a);
}

static bool cCpuAscending(const debug_watchpoint *a, const debug_watchpoint *b)
{
	return strcmp(a->debugInterface()->device().tag(), b->debugInterface()->device().tag()) < 0;
}

static bool cCpuDescending(const debug_watchpoint *a, const debug_watchpoint *b)
{
	return cCpuAscending(b, a);
}

static bool cSpaceAscending(const debug_watchpoint *a, const debug_watchpoint *b)
{
	return strcmp(a->space().name(), b->space().name()) < 0;
}

static bool cSpaceDescending(const debug_watchpoint *a, const debug_watchpoint *b)
{
	return cSpaceAscending(b, a);
}

static bool cAddressAscending(const debug_watchpoint *a, const debug_watchpoint *b)
{
	return a->address() < b->address();
}

static bool cAddressDescending(const debug_watchpoint *a, const debug_watchpoint *b)
{
	return cAddressAscending(b, a);
}

static bool cTypeAscending(const debug_watchpoint *a, const debug_watchpoint *b)
{
	return int(a->type()) < int(b->type());
}

static bool cTypeDescending(const debug_watchpoint *a, const debug_watchpoint *b)
{
	return cTypeAscending(b, a);
}

static bool cConditionAscending(const debug_watchpoint *a, const debug_watchpoint *b)
{
	return strcmp(a->condition(), b->condition()) < 0;
}

static bool cConditionDescending(const debug_watchpoint *a, const debug_watchpoint *b)
{
	return cConditionAscending(b, a);
}

static bool cActionAscending(const debug_watchpoint *a, const debug_watchpoint *b)
{
	return a->action() < b->action();
}

static bool cActionDescending(const debug_watchpoint *a, const debug_watchpoint *b)
{
	return cActionAscending(b, a);
}


//**************************************************************************
//  DEBUG VIEW WATCH POINTS
//**************************************************************************

static const int tableBreaks[] = { 5, 9, 31, 42, 60, 67, 86, 100 };

//-------------------------------------------------
//  debug_view_watchpoints - constructor
//-------------------------------------------------

debug_view_watchpoints::debug_view_watchpoints(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate)
	: debug_view(machine, DVT_WATCH_POINTS, osdupdate, osdprivate),
		m_sortType(&cIndexAscending)
{
	// fail if no available sources
	enumerate_sources();
	if (m_source_list.empty())
		throw std::bad_alloc();
}


//-------------------------------------------------
//  ~debug_view_watchpoints - destructor
//-------------------------------------------------

debug_view_watchpoints::~debug_view_watchpoints()
{
}


//-------------------------------------------------
//  enumerate_sources - enumerate all possible
//  sources for a disassembly view
//-------------------------------------------------

void debug_view_watchpoints::enumerate_sources()
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

void debug_view_watchpoints::view_click(const int button, const debug_view_xy& pos)
{
	bool const clickedTopRow = (m_topleft.y == pos.y);

	if (clickedTopRow)
	{
		if (pos.x < tableBreaks[0])
			m_sortType = (m_sortType == &cIndexAscending) ? &cIndexDescending : &cIndexAscending;
		else if (pos.x < tableBreaks[1])
			m_sortType = (m_sortType == &cEnabledAscending) ? &cEnabledDescending : &cEnabledAscending;
		else if (pos.x < tableBreaks[2])
			m_sortType = (m_sortType == &cCpuAscending) ? &cCpuDescending : &cCpuAscending;
		else if (pos.x < tableBreaks[3])
			m_sortType = (m_sortType == &cSpaceAscending) ? &cSpaceDescending : &cSpaceAscending;
		else if (pos.x < tableBreaks[4])
			m_sortType = (m_sortType == &cAddressAscending) ? &cAddressDescending : &cAddressAscending;
		else if (pos.x < tableBreaks[5])
			m_sortType = (m_sortType == &cTypeAscending) ? &cTypeDescending : &cTypeAscending;
		else if (pos.x < tableBreaks[6])
			m_sortType = (m_sortType == &cConditionAscending) ? &cConditionDescending : &cConditionAscending;
		else if (pos.x < tableBreaks[7])
			m_sortType = (m_sortType == &cActionAscending) ? &cActionDescending : &cActionAscending;
	}
	else
	{
		// Gather a sorted list of all the watchpoints for all the CPUs
		gather_watchpoints();

		int const wpIndex = pos.y - 1;
		if ((wpIndex >= m_buffer.size()) || (wpIndex < 0))
			return;

		// Enable / disable
		m_buffer[wpIndex]->setEnabled(!m_buffer[wpIndex]->enabled());
	}

	begin_update();
	m_update_pending = true;
	end_update();
}


void debug_view_watchpoints::pad_ostream_to_length(std::ostream& str, int len)
{
	auto const current = str.tellp();
	if (current < decltype(current)(len))
		str << std::setw(decltype(current)(len) - current) << "";
}


void debug_view_watchpoints::gather_watchpoints()
{
	m_buffer.resize(0);
	for (auto &source : m_source_list)
	{
		// Collect
		device_debug &debugInterface = *source->device()->debug();
		for (int spacenum = 0; spacenum < debugInterface.watchpoint_space_count(); ++spacenum)
		{
			for (const auto &wp : debugInterface.watchpoint_vector(spacenum))
				m_buffer.push_back(wp.get());
		}
	}

	// And now for the sort
	if (!m_buffer.empty())
		std::stable_sort(m_buffer.begin(), m_buffer.end(), m_sortType);
}


//-------------------------------------------------
//  view_update - update the contents of the
//  watchpoints view
//-------------------------------------------------

void debug_view_watchpoints::view_update()
{
	// Gather a list of all the watchpoints for all the CPUs
	gather_watchpoints();

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
		linebuf << "Space";
		if (m_sortType == &cSpaceAscending) linebuf.put('\\');
		else if (m_sortType == &cSpaceDescending) linebuf.put('/');
		pad_ostream_to_length(linebuf, tableBreaks[3]);
		linebuf << "Addresses";
		if (m_sortType == &cAddressAscending) linebuf.put('\\');
		else if (m_sortType == &cAddressDescending) linebuf.put('/');
		pad_ostream_to_length(linebuf, tableBreaks[4]);
		linebuf << "Type";
		if (m_sortType == &cTypeAscending) linebuf.put('\\');
		else if (m_sortType == &cTypeDescending) linebuf.put('/');
		pad_ostream_to_length(linebuf, tableBreaks[5]);
		linebuf << "Condition";
		if (m_sortType == &cConditionAscending) linebuf.put('\\');
		else if (m_sortType == &cConditionDescending) linebuf.put('/');
		pad_ostream_to_length(linebuf, tableBreaks[6]);
		linebuf << "Action";
		if (m_sortType == &cActionAscending) linebuf.put('\\');
		else if (m_sortType == &cActionDescending) linebuf.put('/');
		pad_ostream_to_length(linebuf, tableBreaks[7]);

		auto const &text(linebuf.vec());
		for (u32 i = m_topleft.x; i < (m_topleft.x + m_visible.x); i++, dest++)
		{
			dest->byte = (i < text.size()) ? text[i] : ' ';
			dest->attrib = DCA_ANCILLARY;
		}
	}

	for (int row = 1; row < m_visible.y; row++)
	{
		// watchpoints
		int const wpi = row + m_topleft.y - 1;
		if ((wpi < m_buffer.size()) && wpi >= 0)
		{
			static char const *const types[] = { "unkn ", "read ", "write", "r/w  " };
			debug_watchpoint *const wp = m_buffer[wpi];

			linebuf.clear();
			linebuf.rdbuf()->clear();
			util::stream_format(linebuf, "%2X", wp->index());
			pad_ostream_to_length(linebuf, tableBreaks[0]);
			linebuf.put(wp->enabled() ? 'X' : 'O');
			pad_ostream_to_length(linebuf, tableBreaks[1]);
			linebuf << wp->debugInterface()->device().tag();
			pad_ostream_to_length(linebuf, tableBreaks[2]);
			linebuf << wp->space().name();
			pad_ostream_to_length(linebuf, tableBreaks[3]);
			util::stream_format(linebuf, "%0*X", wp->space().addrchars(), wp->address());
			linebuf.put('-');
			util::stream_format(linebuf, "%0*X", wp->space().addrchars(), wp->address() + wp->length() - 1);
			pad_ostream_to_length(linebuf, tableBreaks[4]);
			linebuf << types[int(wp->type())];
			pad_ostream_to_length(linebuf, tableBreaks[5]);
			if (strcmp(wp->condition(), "1"))
				linebuf << wp->condition();
			pad_ostream_to_length(linebuf, tableBreaks[6]);
			linebuf << wp->action();
			pad_ostream_to_length(linebuf, tableBreaks[7]);

			auto const &text(linebuf.vec());
			for (u32 i = m_topleft.x; i < (m_topleft.x + m_visible.x); i++, dest++)
			{
				dest->byte = (i < text.size()) ? text[i] : ' ';
				dest->attrib = DCA_NORMAL;

				// Color disabled watchpoints red
				if ((i >= tableBreaks[0]) && (i < tableBreaks[1]) && !wp->enabled())
					dest->attrib |= DCA_CHANGED;
			}
		}
		else
		{
			// Fill the remaining vertical space
			for (int i = 0; i < m_visible.x; i++, dest++)
			{
				dest->byte = ' ';
				dest->attrib = DCA_NORMAL;
			}
		}
	}
}
