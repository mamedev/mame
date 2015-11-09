// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    dvwpoints.c

    Watchpoint debugger view.

***************************************************************************/

#include "emu.h"
#include "dvwpoints.h"



static int cIndexAscending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	return left->index() - right->index();
}

static int cIndexDescending(const void* a, const void* b)
{
	return cIndexAscending(b, a);
}

static int cEnabledAscending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	return (left->enabled() ? 1 : 0) - (right->enabled() ? 1 : 0);
}

static int cEnabledDescending(const void* a, const void* b)
{
	return cEnabledAscending(b, a);
}

static int cCpuAscending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	return strcmp(left->debugInterface()->device().tag(), right->debugInterface()->device().tag());
}

static int cCpuDescending(const void* a, const void* b)
{
	return cCpuAscending(b, a);
}

static int cSpaceAscending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	return strcmp(left->space().name(), right->space().name());
}

static int cSpaceDescending(const void* a, const void* b)
{
	return cSpaceAscending(b, a);
}

static int cAddressAscending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	return (left->address() > right->address()) ? 1 : (left->address() < right->address()) ? -1 : 0;
}

static int cAddressDescending(const void* a, const void* b)
{
	return cAddressAscending(b, a);
}

static int cTypeAscending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	return left->type() - right->type();
}

static int cTypeDescending(const void* a, const void* b)
{
	return cTypeAscending(b, a);
}

static int cConditionAscending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	return strcmp(left->condition(), right->condition());
}

static int cConditionDescending(const void* a, const void* b)
{
	return cConditionAscending(b, a);
}

static int cActionAscending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	return strcmp(left->action(), right->action());
}

static int cActionDescending(const void* a, const void* b)
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
	if (m_source_list.count() == 0)
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
	m_source_list.reset();

	// iterate over devices with disassembly interfaces
	disasm_interface_iterator iter(machine().root_device());
	for (device_disasm_interface *dasm = iter.first(); dasm != NULL; dasm = iter.next())
	{
		std::string name;
		strprintf(name, "%s '%s'", dasm->device().name(), dasm->device().tag());
		m_source_list.append(*global_alloc(debug_view_source(name.c_str(), &dasm->device())));
	}

	// reset the source to a known good entry
	set_source(*m_source_list.first());
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


void debug_view_watchpoints::pad_astring_to_length(std::string& str, int len)
{
	int diff = len - str.length();
	if (diff > 0)
	{
		std::string buffer;
		for (int i = 0; i < diff; i++)
			buffer.append(" ");
		strcatprintf(str, "%s", buffer.c_str());
	}
}


void debug_view_watchpoints::gather_watchpoints()
{
	m_buffer.resize(0);
	for (const debug_view_source *source = m_source_list.first(); source != NULL; source = source->next())
	{
		// Collect
		device_debug &debugInterface = *source->device()->debug();
		for (address_spacenum spacenum = AS_0; spacenum < ADDRESS_SPACES; spacenum++)
		{
			for (device_debug::watchpoint *wp = debugInterface.watchpoint_first(spacenum); wp != NULL; wp = wp->next())
				m_buffer.push_back(wp);
		}
	}

	// And now for the sort
	if (!m_buffer.empty())
		qsort(&m_buffer[0], m_buffer.size(), sizeof(device_debug::watchpoint *), m_sortType);
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
	m_total.x = tableBreaks[ARRAY_LENGTH(tableBreaks) - 1];
	m_total.y = m_buffer.size() + 1;
	if (m_total.y < 10)
		m_total.y = 10;

	// Draw
	debug_view_char *dest = &m_viewdata[0];
	std::string         linebuf;

	// Header
	if (m_visible.y > 0)
	{
		linebuf.clear();
		linebuf.append("ID");
		if (m_sortType == &cIndexAscending) linebuf.push_back('\\');
		else if (m_sortType == &cIndexDescending) linebuf.push_back('/');
		pad_astring_to_length(linebuf, tableBreaks[0]);
		linebuf.append("En");
		if (m_sortType == &cEnabledAscending) linebuf.push_back('\\');
		else if (m_sortType == &cEnabledDescending) linebuf.push_back('/');
		pad_astring_to_length(linebuf, tableBreaks[1]);
		linebuf.append("CPU");
		if (m_sortType == &cCpuAscending) linebuf.push_back('\\');
		else if (m_sortType == &cCpuDescending) linebuf.push_back('/');
		pad_astring_to_length(linebuf, tableBreaks[2]);
		linebuf.append("Space");
		if (m_sortType == &cSpaceAscending) linebuf.push_back('\\');
		else if (m_sortType == &cSpaceDescending) linebuf.push_back('/');
		pad_astring_to_length(linebuf, tableBreaks[3]);
		linebuf.append("Addresses");
		if (m_sortType == &cAddressAscending) linebuf.push_back('\\');
		else if (m_sortType == &cAddressDescending) linebuf.push_back('/');
		pad_astring_to_length(linebuf, tableBreaks[4]);
		linebuf.append("Type");
		if (m_sortType == &cTypeAscending) linebuf.push_back('\\');
		else if (m_sortType == &cTypeDescending) linebuf.push_back('/');
		pad_astring_to_length(linebuf, tableBreaks[5]);
		linebuf.append("Condition");
		if (m_sortType == &cConditionAscending) linebuf.push_back('\\');
		else if (m_sortType == &cConditionDescending) linebuf.push_back('/');
		pad_astring_to_length(linebuf, tableBreaks[6]);
		linebuf.append("Action");
		if (m_sortType == &cActionAscending) linebuf.push_back('\\');
		else if (m_sortType == &cActionDescending) linebuf.push_back('/');
		pad_astring_to_length(linebuf, tableBreaks[7]);

		for (UINT32 i = m_topleft.x; i < (m_topleft.x + m_visible.x); i++, dest++)
		{
			dest->byte = (i < linebuf.length()) ? linebuf[i] : ' ';
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
			device_debug::watchpoint *const wp = m_buffer[wpi];

			linebuf.clear();
			strcatprintf(linebuf, "%2X", wp->index());
			pad_astring_to_length(linebuf, tableBreaks[0]);
			linebuf.push_back(wp->enabled() ? 'X' : 'O');
			pad_astring_to_length(linebuf, tableBreaks[1]);
			linebuf.append(wp->debugInterface()->device().tag());
			pad_astring_to_length(linebuf, tableBreaks[2]);
			linebuf.append(wp->space().name());
			pad_astring_to_length(linebuf, tableBreaks[3]);
			linebuf.append(core_i64_hex_format(wp->space().byte_to_address(wp->address()), wp->space().addrchars()));
			linebuf.push_back('-');
			linebuf.append(core_i64_hex_format(wp->space().byte_to_address_end(wp->address() + wp->length()) - 1, wp->space().addrchars()));
			pad_astring_to_length(linebuf, tableBreaks[4]);
			linebuf.append(types[wp->type() & 3]);
			pad_astring_to_length(linebuf, tableBreaks[5]);
			if (strcmp(wp->condition(), "1"))
				linebuf.append(wp->condition());
			pad_astring_to_length(linebuf, tableBreaks[6]);
			linebuf.append(wp->action());
			pad_astring_to_length(linebuf, tableBreaks[7]);

			for (UINT32 i = m_topleft.x; i < (m_topleft.x + m_visible.x); i++, dest++)
			{
				dest->byte = (i < linebuf.length()) ? linebuf[i] : ' ';
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
