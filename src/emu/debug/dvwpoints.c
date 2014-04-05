// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    dvwpoints.c

    Watchpoint debugger view.

***************************************************************************/

#include "emu.h"
#include "dvwpoints.h"



//**************************************************************************
//  DEBUG VIEW WATCH POINTS
//**************************************************************************

static const int tableBreaks[] = { 5, 9, 31, 42, 60, 67, 86, 100 };

//-------------------------------------------------
//  debug_view_watchpoints - constructor
//-------------------------------------------------

debug_view_watchpoints::debug_view_watchpoints(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate)
	: debug_view(machine, DVT_WATCH_POINTS, osdupdate, osdprivate),
		m_sortType(SORT_INDEX_ASCENDING)
{
	// fail if no available sources
	enumerate_sources();
	if (m_source_list.count() == 0)
		throw std::bad_alloc();

	// configure the view
	m_total.y = 10;
	m_supports_cursor = false;
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
		astring name;
		name.printf("%s '%s'", dasm->device().name(), dasm->device().tag());
		m_source_list.append(*global_alloc(debug_view_source(name.cstr(), &dasm->device())));
	}

	// reset the source to a known good entry
	set_source(*m_source_list.first());
}


//-------------------------------------------------
//  view_notify - handle notification of updates
//  to cursor changes
//-------------------------------------------------

void debug_view_watchpoints::view_notify(debug_view_notification type)
{
}


//-------------------------------------------------
//  view_char - handle a character typed within
//  the current view
//-------------------------------------------------

void debug_view_watchpoints::view_char(int chval)
{
}


//-------------------------------------------------
//  view_click - handle a mouse click within the
//  current view
//-------------------------------------------------

void debug_view_watchpoints::view_click(const int button, const debug_view_xy& pos)
{
	bool clickedTopRow = (m_topleft.y == pos.y);

	if (clickedTopRow)
	{
		if (pos.x < tableBreaks[0] && m_sortType == SORT_INDEX_ASCENDING)
			m_sortType = SORT_INDEX_DESCENDING;
		else if (pos.x < tableBreaks[0])
			m_sortType = SORT_INDEX_ASCENDING;
		else if (pos.x < tableBreaks[1] && m_sortType == SORT_ENABLED_ASCENDING)
			m_sortType = SORT_ENABLED_DESCENDING;
		else if (pos.x < tableBreaks[1])
			m_sortType = SORT_ENABLED_ASCENDING;
		else if (pos.x < tableBreaks[2] && m_sortType == SORT_CPU_ASCENDING)
			m_sortType = SORT_CPU_DESCENDING;
		else if (pos.x < tableBreaks[2])
			m_sortType = SORT_CPU_ASCENDING;
		else if (pos.x < tableBreaks[3] && m_sortType == SORT_SPACE_ASCENDING)
			m_sortType = SORT_SPACE_DESCENDING;
		else if (pos.x < tableBreaks[3])
			m_sortType = SORT_SPACE_ASCENDING;
		else if (pos.x < tableBreaks[4] && m_sortType == SORT_ADDRESS_ASCENDING)
			m_sortType = SORT_ADDRESS_DESCENDING;
		else if (pos.x < tableBreaks[4])
			m_sortType = SORT_ADDRESS_ASCENDING;
		else if (pos.x < tableBreaks[5] && m_sortType == SORT_TYPE_ASCENDING)
			m_sortType = SORT_TYPE_DESCENDING;
		else if (pos.x < tableBreaks[5])
			m_sortType = SORT_TYPE_ASCENDING;
		else if (pos.x < tableBreaks[6] && m_sortType == SORT_CONDITION_ASCENDING)
			m_sortType = SORT_CONDITION_DESCENDING;
		else if (pos.x < tableBreaks[6])
			m_sortType = SORT_CONDITION_ASCENDING;
		else if (pos.x < tableBreaks[7] && m_sortType == SORT_ACTION_ASCENDING)
			m_sortType = SORT_ACTION_DESCENDING;
		else if (pos.x < tableBreaks[7])
			m_sortType = SORT_ACTION_ASCENDING;
	}
	else
	{
		// Gather a sorted list of all the watchpoints for all the CPUs
		device_debug::watchpoint** wpList = NULL;
		const int numWPs = watchpoints(SORT_NONE, wpList);

		const int wpIndex = pos.y-1;
		if (wpIndex > numWPs || wpIndex < 0)
			return;

		// Enable / disable
		if (wpList[wpIndex]->enabled())
			wpList[wpIndex]->setEnabled(false);
		else
			wpList[wpIndex]->setEnabled(true);

		delete[] wpList;
	}

	view_update();
}


void debug_view_watchpoints::pad_astring_to_length(astring& str, int len)
{
	int diff = len - str.len();
	if (diff > 0)
	{
		astring buffer;
		buffer.expand(diff);
		for (int i = 0; i < diff; i++)
			buffer.catprintf(" ");
		str.catprintf("%s", buffer.cstr());
	}
}


// Sorting functors for the qsort function
static int cIndexAscending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	return left->index() > right->index();
}

static int cIndexDescending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	return left->index() < right->index();
}

static int cEnabledAscending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	return left->enabled() < right->enabled();
}

static int cEnabledDescending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	return left->enabled() > right->enabled();
}

static int cCpuAscending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	const int result = strcmp(left->debugInterface()->device().tag(), right->debugInterface()->device().tag());
	return result >= 0;
}

static int cCpuDescending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	const int result = strcmp(left->debugInterface()->device().tag(), right->debugInterface()->device().tag());
	return result < 0;
}

static int cSpaceAscending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	const int result = strcmp(left->space().name(), right->space().name());
	return result >= 0;
}

static int cSpaceDescending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	const int result = strcmp(left->space().name(), right->space().name());
	return result < 0;
}

static int cAddressAscending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	return left->address() > right->address();
}

static int cAddressDescending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	return left->address() < right->address();
}

static int cTypeAscending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	return left->type() > right->type();
}

static int cTypeDescending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	return left->type() < right->type();
}

static int cConditionAscending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	const int result = strcmp(left->condition(), right->condition());
	return result >= 0;
}

static int cConditionDescending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	const int result = strcmp(left->condition(), right->condition());
	return result < 0;
}

static int cActionAscending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	const int result = strcmp(left->action(), right->action());
	return result >= 0;
}

static int cActionDescending(const void* a, const void* b)
{
	const device_debug::watchpoint* left = *(device_debug::watchpoint**)a;
	const device_debug::watchpoint* right = *(device_debug::watchpoint**)b;
	const int result = strcmp(left->action(), right->action());
	return result < 0;
}


int debug_view_watchpoints::watchpoints(SortMode sort, device_debug::watchpoint**& wpList)
{
	// Alloc
	int numWPs = 0;
	wpList = NULL;
	for (const debug_view_source *source = m_source_list.first(); source != NULL; source = source->next())
	{
		for (address_spacenum spacenum = AS_0; spacenum < ADDRESS_SPACES; spacenum++)
		{
			/* loop over the watchpoints */
			const device_debug& debugInterface = *source->device()->debug();
			for (device_debug::watchpoint *wp = debugInterface.watchpoint_first(spacenum); wp != NULL; wp = wp->next())
				numWPs++;
		}
	}
	wpList = new device_debug::watchpoint*[numWPs];

	int wpAddIndex = 0;
	for (const debug_view_source *source = m_source_list.first(); source != NULL; source = source->next())
	{
		// Collect
		for (address_spacenum spacenum = AS_0; spacenum < ADDRESS_SPACES; spacenum++)
		{
			device_debug& debugInterface = *source->device()->debug();
			for (device_debug::watchpoint *wp = debugInterface.watchpoint_first(spacenum); wp != NULL; wp = wp->next())
			{
				wpList[wpAddIndex] = wp;
				wpAddIndex++;
			}
		}
	}

	// And now for the sort
	switch (m_sortType)
	{
		case SORT_NONE:
			break;
		case SORT_INDEX_ASCENDING:
			qsort(wpList, numWPs, sizeof(device_debug::watchpoint*), cIndexAscending);
			break;
		case SORT_INDEX_DESCENDING:
			qsort(wpList, numWPs, sizeof(device_debug::watchpoint*), cIndexDescending);
			break;
		case SORT_ENABLED_ASCENDING:
			qsort(wpList, numWPs, sizeof(device_debug::watchpoint*), cEnabledAscending);
			break;
		case SORT_ENABLED_DESCENDING:
			qsort(wpList, numWPs, sizeof(device_debug::watchpoint*), cEnabledDescending);
			break;
		case SORT_CPU_ASCENDING:
			qsort(wpList, numWPs, sizeof(device_debug::watchpoint*), cCpuAscending);
			break;
		case SORT_CPU_DESCENDING:
			qsort(wpList, numWPs, sizeof(device_debug::watchpoint*), cCpuDescending);
			break;
		case SORT_SPACE_ASCENDING:
			qsort(wpList, numWPs, sizeof(device_debug::watchpoint*), cSpaceAscending);
			break;
		case SORT_SPACE_DESCENDING:
			qsort(wpList, numWPs, sizeof(device_debug::watchpoint*), cSpaceDescending);
			break;
		case SORT_ADDRESS_ASCENDING:
			qsort(wpList, numWPs, sizeof(device_debug::watchpoint*), cAddressAscending);
			break;
		case SORT_ADDRESS_DESCENDING:
			qsort(wpList, numWPs, sizeof(device_debug::watchpoint*), cAddressDescending);
			break;
		case SORT_TYPE_ASCENDING:
			qsort(wpList, numWPs, sizeof(device_debug::watchpoint*), cTypeAscending);
			break;
		case SORT_TYPE_DESCENDING:
			qsort(wpList, numWPs, sizeof(device_debug::watchpoint*), cTypeDescending);
			break;
		case SORT_CONDITION_ASCENDING:
			qsort(wpList, numWPs, sizeof(device_debug::watchpoint*), cConditionAscending);
			break;
		case SORT_CONDITION_DESCENDING:
			qsort(wpList, numWPs, sizeof(device_debug::watchpoint*), cConditionDescending);
			break;
		case SORT_ACTION_ASCENDING:
			qsort(wpList, numWPs, sizeof(device_debug::watchpoint*), cActionAscending);
			break;
		case SORT_ACTION_DESCENDING:
			qsort(wpList, numWPs, sizeof(device_debug::watchpoint*), cActionDescending);
			break;
	}

	return numWPs;
}


//-------------------------------------------------
//  view_update - update the contents of the
//  disassembly view
//-------------------------------------------------

void debug_view_watchpoints::view_update()
{
	// Gather a list of all the watchpoints for all the CPUs
	device_debug::watchpoint** wpList = NULL;
	const int numWPs = watchpoints(SORT_NONE, wpList);

	// Set the view region so the scroll bars update
	m_total.y = numWPs+1;

	// Draw
	debug_view_char *dest = m_viewdata;
	for (int row = 0; row < m_visible.y; row++)
	{
		UINT32 effrow = m_topleft.y + row;

		// Header
		if (row == 0)
		{
			astring header;
			header.printf("ID");
			if (m_sortType == SORT_INDEX_ASCENDING) header.catprintf("\\");
			else if (m_sortType == SORT_INDEX_DESCENDING) header.catprintf("/");
			pad_astring_to_length(header, tableBreaks[0]);
			header.catprintf("En");
			if (m_sortType == SORT_ENABLED_ASCENDING) header.catprintf("\\");
			else if (m_sortType == SORT_ENABLED_DESCENDING) header.catprintf("/");
			pad_astring_to_length(header, tableBreaks[1]);
			header.catprintf("CPU");
			if (m_sortType == SORT_CPU_ASCENDING) header.catprintf("\\");
			else if (m_sortType == SORT_CPU_DESCENDING) header.catprintf("/");
			pad_astring_to_length(header, tableBreaks[2]);
			header.catprintf("Space");
			if (m_sortType == SORT_SPACE_ASCENDING) header.catprintf("\\");
			else if (m_sortType == SORT_SPACE_DESCENDING) header.catprintf("/");
			pad_astring_to_length(header, tableBreaks[3]);
			header.catprintf("Addresses");
			if (m_sortType == SORT_ADDRESS_ASCENDING) header.catprintf("\\");
			else if (m_sortType == SORT_ADDRESS_DESCENDING) header.catprintf("/");
			pad_astring_to_length(header, tableBreaks[4]);
			header.catprintf("Type");
			if (m_sortType == SORT_TYPE_ASCENDING) header.catprintf("\\");
			else if (m_sortType == SORT_TYPE_DESCENDING) header.catprintf("/");
			pad_astring_to_length(header, tableBreaks[5]);
			header.catprintf("Condition");
			if (m_sortType == SORT_CONDITION_ASCENDING) header.catprintf("\\");
			else if (m_sortType == SORT_CONDITION_DESCENDING) header.catprintf("/");
			pad_astring_to_length(header, tableBreaks[6]);
			header.catprintf("Action");
			if (m_sortType == SORT_ACTION_ASCENDING) header.catprintf("\\");
			else if (m_sortType == SORT_ACTION_DESCENDING) header.catprintf("/");
			pad_astring_to_length(header, tableBreaks[7]);

			for (int i = 0; i < m_visible.x; i++)
			{
				dest->byte = (i < header.len()) ? header[i] : ' ';
				dest->attrib = DCA_ANCILLARY;
				dest++;
			}
			continue;
		}

		// watchpoints
		int wpi = effrow-1;
		if (wpi < numWPs && wpi >= 0)
		{
			static const char *const types[] = { "unkn ", "read ", "write", "r/w  " };
			device_debug::watchpoint* wp = wpList[wpi];

			astring buffer;
			buffer.printf("%x", wp->index());
			pad_astring_to_length(buffer, tableBreaks[0]);
			buffer.catprintf("%c", wp->enabled() ? 'X' : 'O');
			pad_astring_to_length(buffer, tableBreaks[1]);
			buffer.catprintf("%s", wp->debugInterface()->device().tag());
			pad_astring_to_length(buffer, tableBreaks[2]);
			buffer.catprintf("%s", wp->space().name());
			pad_astring_to_length(buffer, tableBreaks[3]);
			buffer.catprintf("%s-%s",
								core_i64_hex_format(wp->space().byte_to_address(wp->address()), wp->space().addrchars()),
								core_i64_hex_format(wp->space().byte_to_address_end(wp->address() + wp->length()) - 1, wp->space().addrchars()));
			pad_astring_to_length(buffer, tableBreaks[4]);
			buffer.catprintf("%s", types[wp->type() & 3]);
			pad_astring_to_length(buffer, tableBreaks[5]);
			if (astring(wp->condition()) != astring("1"))
			{
				buffer.catprintf("%s", wp->condition());
				pad_astring_to_length(buffer, tableBreaks[6]);
			}
			if (astring(wp->action()) != astring(""))
			{
				buffer.catprintf("%s", wp->action());
				pad_astring_to_length(buffer, tableBreaks[7]);
			}

			for (int i = 0; i < m_visible.x; i++)
			{
				dest->byte = (i < buffer.len()) ? buffer[i] : ' ';
				dest->attrib = DCA_NORMAL;

				// Color disabled watchpoints red
				if (i == 5 && dest->byte == 'O')
					dest->attrib = DCA_CHANGED;

				dest++;
			}
			continue;
		}

		// Fill the remaining vertical space
		for (int i = 0; i < m_visible.x; i++)
		{
			dest->byte = ' ';
			dest->attrib = DCA_NORMAL;
			dest++;
		}
	}

	delete[] wpList;
}
