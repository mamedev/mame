// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*********************************************************************

    dvrpoints.cpp

    Registerpoint debugger view.

***************************************************************************/

#include "emu.h"
#include "dvrpoints.h"

#include "debugcpu.h"
#include "points.h"

#include <algorithm>
#include <iomanip>


namespace {

bool cIndexAscending(std::pair<device_t *, debug_registerpoint const *> const &a, std::pair<device_t *, debug_registerpoint const *> const &b)
{
	return a.second->index() < b.second->index();
}

bool cIndexDescending(std::pair<device_t *, debug_registerpoint const *> const &a, std::pair<device_t *, debug_registerpoint const *> const &b)
{
	return a.second->index() > b.second->index();
}

bool cEnabledAscending(std::pair<device_t *, debug_registerpoint const *> const &a, std::pair<device_t *, debug_registerpoint const *> const &b)
{
	return !a.second->enabled() && b.second->enabled();
}

bool cEnabledDescending(std::pair<device_t *, debug_registerpoint const *> const &a, std::pair<device_t *, debug_registerpoint const *> const &b)
{
	return cEnabledAscending(b, a);
}

bool cCpuAscending(std::pair<device_t *, debug_registerpoint const *> const &a, std::pair<device_t *, debug_registerpoint const *> const &b)
{
	return strcmp(a.first->tag(), b.first->tag()) < 0;
}

bool cCpuDescending(std::pair<device_t *, debug_registerpoint const *> const &a, std::pair<device_t *, debug_registerpoint const *> const &b)
{
	return cCpuAscending(b, a);
}

bool cConditionAscending(std::pair<device_t *, debug_registerpoint const *> const &a, std::pair<device_t *, debug_registerpoint const *> const &b)
{
	return strcmp(a.second->condition(), b.second->condition()) < 0;
}

bool cConditionDescending(std::pair<device_t *, debug_registerpoint const *> const &a, std::pair<device_t *, debug_registerpoint const *> const &b)
{
	return cConditionAscending(b, a);
}

bool cActionAscending(std::pair<device_t *, debug_registerpoint const *> const &a, std::pair<device_t *, debug_registerpoint const *> const &b)
{
	return a.second->action() < b.second->action();
}

bool cActionDescending(std::pair<device_t *, debug_registerpoint const *> const &a, std::pair<device_t *, debug_registerpoint const *> const &b)
{
	return cActionAscending(b, a);
}


constexpr int TABLE_BREAKS[] = { 5, 9, 31, 49, 66 };

} // anonymous namespace


//**************************************************************************
//  DEBUG VIEW REGISTER POINTS
//**************************************************************************


//-------------------------------------------------
//  debug_view_registerpoints - constructor
//-------------------------------------------------

debug_view_registerpoints::debug_view_registerpoints(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate)
	: debug_view(machine, DVT_REGISTER_POINTS, osdupdate, osdprivate)
	, m_sort_type(cIndexAscending)
{
	// fail if no available sources
	enumerate_sources();
	if (m_source_list.empty())
		throw std::bad_alloc();
}


//-------------------------------------------------
//  ~debug_view_registerpoints - destructor
//-------------------------------------------------

debug_view_registerpoints::~debug_view_registerpoints()
{
}


//-------------------------------------------------
//  enumerate_sources - enumerate all possible
//  sources for a disassembly view
//-------------------------------------------------

void debug_view_registerpoints::enumerate_sources()
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

void debug_view_registerpoints::view_click(const int button, const debug_view_xy& pos)
{
	bool clickedTopRow = (m_topleft.y == pos.y);

	if (clickedTopRow)
	{
		if (pos.x < TABLE_BREAKS[0])
			m_sort_type = (m_sort_type == &cIndexAscending) ? &cIndexDescending : &cIndexAscending;
		else if (pos.x < TABLE_BREAKS[1])
			m_sort_type = (m_sort_type == &cEnabledAscending) ? &cEnabledDescending : &cEnabledAscending;
		else if (pos.x < TABLE_BREAKS[2])
			m_sort_type = (m_sort_type == &cCpuAscending) ? &cCpuDescending : &cCpuAscending;
		else if (pos.x < TABLE_BREAKS[3])
			m_sort_type = (m_sort_type == &cConditionAscending) ? &cConditionDescending : &cConditionAscending;
		else if (pos.x < TABLE_BREAKS[4])
			m_sort_type = (m_sort_type == &cActionAscending) ? &cActionDescending : &cActionAscending;
	}
	else
	{
		// Gather a sorted list of all the breakpoints for all the CPUs
		gather_registerpoints();

		int rpIndex = pos.y - 1;
		if ((rpIndex >= m_buffer.size()) || (rpIndex < 0))
			return;

		// Enable / disable
		m_buffer[rpIndex].first->debug()->registerpoint_enable(
				m_buffer[rpIndex].second->index(),
				!m_buffer[rpIndex].second->enabled());
	}

	begin_update();
	m_update_pending = true;
	end_update();
}


void debug_view_registerpoints::pad_ostream_to_length(std::ostream& str, int len)
{
	auto const current = str.tellp();
	if (current < decltype(current)(len))
		str << std::setw(decltype(current)(len) - current) << "";
}


void debug_view_registerpoints::gather_registerpoints()
{
	m_buffer.resize(0);
	for (auto &source : m_source_list)
	{
		// Collect
		device_debug &debugInterface = *source->device()->debug();
		for (const auto &rp : debugInterface.registerpoint_list())
			m_buffer.emplace_back(source->device(), &rp);
	}

	// And now for the sort
	if (!m_buffer.empty())
		std::stable_sort(m_buffer.begin(), m_buffer.end(), m_sort_type);
}


//-------------------------------------------------
//  view_update - update the contents of the
//  registerpoints view
//-------------------------------------------------

void debug_view_registerpoints::view_update()
{
	// Gather a list of all the registerpoints for all the CPUs
	gather_registerpoints();

	// Set the view region so the scroll bars update
	m_total.x = TABLE_BREAKS[std::size(TABLE_BREAKS) - 1];
	m_total.y = m_buffer.size() + 1;
	if (m_total.y < 10)
		m_total.y = 10;

	// Draw
	debug_view_char     *dest = &m_viewdata[0];
	util::ovectorstream linebuf;
	linebuf.reserve(std::size(TABLE_BREAKS) - 1);

	// Header
	if (m_visible.y > 0)
	{
		linebuf.clear();
		linebuf.rdbuf()->clear();
		linebuf << "ID";
		if (m_sort_type == &cIndexAscending) linebuf.put('\\');
		else if (m_sort_type == &cIndexDescending) linebuf.put('/');
		pad_ostream_to_length(linebuf, TABLE_BREAKS[0]);
		linebuf << "En";
		if (m_sort_type == &cEnabledAscending) linebuf.put('\\');
		else if (m_sort_type == &cEnabledDescending) linebuf.put('/');
		pad_ostream_to_length(linebuf, TABLE_BREAKS[1]);
		linebuf << "CPU";
		if (m_sort_type == &cCpuAscending) linebuf.put('\\');
		else if (m_sort_type == &cCpuDescending) linebuf.put('/');
		pad_ostream_to_length(linebuf, TABLE_BREAKS[2]);
		linebuf << "Condition";
		if (m_sort_type == &cConditionAscending) linebuf.put('\\');
		else if (m_sort_type == &cConditionDescending) linebuf.put('/');
		pad_ostream_to_length(linebuf, TABLE_BREAKS[3]);
		linebuf << "Action";
		if (m_sort_type == &cActionAscending) linebuf.put('\\');
		else if (m_sort_type == &cActionDescending) linebuf.put('/');
		pad_ostream_to_length(linebuf, TABLE_BREAKS[4]);

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
		int rpi = row + m_topleft.y - 1;
		if ((rpi < m_buffer.size()) && (rpi >= 0))
		{
			point_pair const &rpp = m_buffer[rpi];

			linebuf.clear();
			linebuf.rdbuf()->clear();
			util::stream_format(linebuf, "%2X", rpp.second->index());
			pad_ostream_to_length(linebuf, TABLE_BREAKS[0]);
			linebuf.put(rpp.second->enabled() ? 'X' : 'O');
			pad_ostream_to_length(linebuf, TABLE_BREAKS[1]);
			linebuf << rpp.first->tag();
			pad_ostream_to_length(linebuf, TABLE_BREAKS[2]);
			linebuf << rpp.second->condition();
			pad_ostream_to_length(linebuf, TABLE_BREAKS[3]);
			linebuf << rpp.second->action();
			pad_ostream_to_length(linebuf, TABLE_BREAKS[4]);

			auto const &text(linebuf.vec());
			for (u32 i = m_topleft.x; i < (m_topleft.x + m_visible.x); i++, dest++)
			{
				dest->byte = (i < text.size()) ? text[i] : ' ';
				dest->attrib = DCA_NORMAL;

				// Color disabled breakpoints red
				if ((i >= TABLE_BREAKS[0]) && (i < TABLE_BREAKS[1]) && !rpp.second->enabled())
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
