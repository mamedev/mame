// license:BSD-3-Clause
// copyright-holders:Andrew Gardner, Vas Crabb
/*********************************************************************

    dvepoints.cpp

    Exceptionpoint debugger view.

***************************************************************************/

#include "emu.h"
#include "dvepoints.h"

#include "debugcpu.h"
#include "points.h"

#include <algorithm>
#include <iomanip>
#include <locale>



// Sorting functors for the qsort function
static bool cIndexAscending(const debug_exceptionpoint *a, const debug_exceptionpoint *b)
{
	return a->index() < b->index();
}

static bool cIndexDescending(const debug_exceptionpoint *a, const debug_exceptionpoint *b)
{
	return cIndexAscending(b, a);
}

static bool cEnabledAscending(const debug_exceptionpoint *a, const debug_exceptionpoint *b)
{
	return !a->enabled() && b->enabled();
}

static bool cEnabledDescending(const debug_exceptionpoint *a, const debug_exceptionpoint *b)
{
	return cEnabledAscending(b, a);
}

static bool cCpuAscending(const debug_exceptionpoint *a, const debug_exceptionpoint *b)
{
	return strcmp(a->debugInterface()->device().tag(), b->debugInterface()->device().tag()) < 0;
}

static bool cCpuDescending(const debug_exceptionpoint *a, const debug_exceptionpoint *b)
{
	return cCpuAscending(b, a);
}

static bool cTypeAscending(const debug_exceptionpoint *a, const debug_exceptionpoint *b)
{
	return a->type() < b->type();
}

static bool cTypeDescending(const debug_exceptionpoint *a, const debug_exceptionpoint *b)
{
	return cTypeAscending(b, a);
}

static bool cConditionAscending(const debug_exceptionpoint *a, const debug_exceptionpoint *b)
{
	return strcmp(a->condition(), b->condition()) < 0;
}

static bool cConditionDescending(const debug_exceptionpoint *a, const debug_exceptionpoint *b)
{
	return cConditionAscending(b, a);
}

static bool cActionAscending(const debug_exceptionpoint *a, const debug_exceptionpoint *b)
{
	return a->action() < b->action();
}

static bool cActionDescending(const debug_exceptionpoint *a, const debug_exceptionpoint *b)
{
	return cActionAscending(b, a);
}


//**************************************************************************
//  DEBUG VIEW BREAK POINTS
//**************************************************************************

static const int tableBreaks[] = { 5, 9, 31, 45, 63, 80 };


//-------------------------------------------------
//  debug_view_exceptionpoints - constructor
//-------------------------------------------------

debug_view_exceptionpoints::debug_view_exceptionpoints(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate)
	: debug_view(machine, DVT_EXCEPTION_POINTS, osdupdate, osdprivate)
	, m_sortType(cIndexAscending)
{
	// fail if no available sources
	enumerate_sources();
	if (m_source_list.empty())
		throw std::bad_alloc();
}


//-------------------------------------------------
//  ~debug_view_exceptionpoints - destructor
//-------------------------------------------------

debug_view_exceptionpoints::~debug_view_exceptionpoints()
{
}


//-------------------------------------------------
//  enumerate_sources - enumerate all possible
//  sources for a disassembly view
//-------------------------------------------------

void debug_view_exceptionpoints::enumerate_sources()
{
	// start with an empty list
	m_source_list.clear();

	// iterate over devices with disassembly interfaces
	for (device_disasm_interface &dasm : disasm_interface_enumerator(machine().root_device()))
	{
		m_source_list.emplace_back(
				std::make_unique<debug_view_source>(
					util::string_format(std::locale::classic(), "%s '%s'", dasm.device().name(), dasm.device().tag()),
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

void debug_view_exceptionpoints::view_click(const int button, const debug_view_xy& pos)
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
			m_sortType = (m_sortType == &cTypeAscending) ? &cTypeDescending : &cTypeAscending;
		else if (pos.x < tableBreaks[4])
			m_sortType = (m_sortType == &cConditionAscending) ? &cConditionDescending : &cConditionAscending;
		else if (pos.x < tableBreaks[5])
			m_sortType = (m_sortType == &cActionAscending) ? &cActionDescending : &cActionAscending;
	}
	else
	{
		// Gather a sorted list of all the exceptionpoints for all the CPUs
		gather_exceptionpoints();

		int epIndex = pos.y - 1;
		if ((epIndex >= m_buffer.size()) || (epIndex < 0))
			return;

		// Enable / disable
		const_cast<debug_exceptionpoint &>(*m_buffer[epIndex]).setEnabled(!m_buffer[epIndex]->enabled());

		machine().debug_view().update_all(DVT_DISASSEMBLY);
	}

	begin_update();
	m_update_pending = true;
	end_update();
}


void debug_view_exceptionpoints::pad_ostream_to_length(std::ostream& str, int len)
{
	auto const current = str.tellp();
	if (current < decltype(current)(len))
		str << std::setw(decltype(current)(len) - current) << "";
}


void debug_view_exceptionpoints::gather_exceptionpoints()
{
	m_buffer.resize(0);
	for (auto &source : m_source_list)
	{
		// Collect
		device_debug &debugInterface = *source->device()->debug();
		for (const auto &epp : debugInterface.exceptionpoint_list())
			m_buffer.push_back(epp.second.get());
	}

	// And now for the sort
	if (!m_buffer.empty())
		std::stable_sort(m_buffer.begin(), m_buffer.end(), m_sortType);
}


//-------------------------------------------------
//  view_update - update the contents of the
//  exceptionpoints view
//-------------------------------------------------

void debug_view_exceptionpoints::view_update()
{
	// Gather a list of all the exceptionpoints for all the CPUs
	gather_exceptionpoints();

	// Set the view region so the scroll bars update
	m_total.x = tableBreaks[std::size(tableBreaks) - 1];
	m_total.y = m_buffer.size() + 1;
	if (m_total.y < 10)
		m_total.y = 10;

	// Draw
	debug_view_char     *dest = &m_viewdata[0];
	util::ovectorstream linebuf;
	linebuf.imbue(std::locale::classic());
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
		linebuf << "Type";
		if (m_sortType == &cTypeAscending) linebuf.put('\\');
		else if (m_sortType == &cTypeDescending) linebuf.put('/');
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
		int epi = row + m_topleft.y - 1;
		if ((epi < m_buffer.size()) && (epi >= 0))
		{
			const debug_exceptionpoint *const ep = m_buffer[epi];

			linebuf.clear();
			linebuf.rdbuf()->clear();
			util::stream_format(linebuf, "%2X", ep->index());
			pad_ostream_to_length(linebuf, tableBreaks[0]);
			linebuf.put(ep->enabled() ? 'X' : 'O');
			pad_ostream_to_length(linebuf, tableBreaks[1]);
			linebuf << ep->debugInterface()->device().tag();
			pad_ostream_to_length(linebuf, tableBreaks[2]);
			util::stream_format(linebuf, "%X", ep->type());
			pad_ostream_to_length(linebuf, tableBreaks[3]);
			if (strcmp(ep->condition(), "1"))
				linebuf << ep->condition();
			pad_ostream_to_length(linebuf, tableBreaks[4]);
			linebuf << ep->action();
			pad_ostream_to_length(linebuf, tableBreaks[5]);

			auto const &text(linebuf.vec());
			for (u32 i = m_topleft.x; i < (m_topleft.x + m_visible.x); i++, dest++)
			{
				dest->byte = (i < text.size()) ? text[i] : ' ';
				dest->attrib = DCA_NORMAL;

				// Color disabled exceptionpoints red
				if ((i >= tableBreaks[0]) && (i < tableBreaks[1]) && !ep->enabled())
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
