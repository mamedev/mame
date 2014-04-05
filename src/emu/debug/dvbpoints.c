// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    dvbpoints.c

    Breakpoint debugger view.

***************************************************************************/

#include "emu.h"
#include "dvbpoints.h"



//**************************************************************************
//  DEBUG VIEW BREAK POINTS
//**************************************************************************

static const int tableBreaks[] = { 5, 9, 31, 45, 63, 80 };


//-------------------------------------------------
//  debug_view_breakpoints - constructor
//-------------------------------------------------

debug_view_breakpoints::debug_view_breakpoints(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate)
	: debug_view(machine, DVT_BREAK_POINTS, osdupdate, osdprivate),
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

void debug_view_breakpoints::view_notify(debug_view_notification type)
{
}


//-------------------------------------------------
//  view_char - handle a character typed within
//  the current view
//-------------------------------------------------

void debug_view_breakpoints::view_char(int chval)
{
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
		else if (pos.x < tableBreaks[3] && m_sortType == SORT_ADDRESS_ASCENDING)
			m_sortType = SORT_ADDRESS_DESCENDING;
		else if (pos.x < tableBreaks[3])
			m_sortType = SORT_ADDRESS_ASCENDING;
		else if (pos.x < tableBreaks[4] && m_sortType == SORT_CONDITION_ASCENDING)
			m_sortType = SORT_CONDITION_DESCENDING;
		else if (pos.x < tableBreaks[4])
			m_sortType = SORT_CONDITION_ASCENDING;
		else if (pos.x < tableBreaks[5] && m_sortType == SORT_ACTION_ASCENDING)
			m_sortType = SORT_ACTION_DESCENDING;
		else if (pos.x < tableBreaks[5])
			m_sortType = SORT_ACTION_ASCENDING;
	}
	else
	{
		// Gather a sorted list of all the breakpoints for all the CPUs
		device_debug::breakpoint** bpList = NULL;
		const int numBPs = breakpoints(SORT_NONE, bpList);

		const int bpIndex = pos.y-1;
		if (bpIndex > numBPs || bpIndex < 0)
			return;

		// Enable / disable
		if (bpList[bpIndex]->enabled())
			bpList[bpIndex]->setEnabled(false);
		else
			bpList[bpIndex]->setEnabled(true);

		delete[] bpList;
	}

	view_update();
}


void debug_view_breakpoints::pad_astring_to_length(astring& str, int len)
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
	const device_debug::breakpoint* left = *(device_debug::breakpoint**)a;
	const device_debug::breakpoint* right = *(device_debug::breakpoint**)b;
	return left->index() > right->index();
}

static int cIndexDescending(const void* a, const void* b)
{
	const device_debug::breakpoint* left = *(device_debug::breakpoint**)a;
	const device_debug::breakpoint* right = *(device_debug::breakpoint**)b;
	return left->index() < right->index();
}

static int cEnabledAscending(const void* a, const void* b)
{
	const device_debug::breakpoint* left = *(device_debug::breakpoint**)a;
	const device_debug::breakpoint* right = *(device_debug::breakpoint**)b;
	return left->enabled() < right->enabled();
}

static int cEnabledDescending(const void* a, const void* b)
{
	const device_debug::breakpoint* left = *(device_debug::breakpoint**)a;
	const device_debug::breakpoint* right = *(device_debug::breakpoint**)b;
	return left->enabled() > right->enabled();
}

static int cCpuAscending(const void* a, const void* b)
{
	const device_debug::breakpoint* left = *(device_debug::breakpoint**)a;
	const device_debug::breakpoint* right = *(device_debug::breakpoint**)b;
	const int result = strcmp(left->debugInterface()->device().tag(), right->debugInterface()->device().tag());
	return result >= 0;
}

static int cCpuDescending(const void* a, const void* b)
{
	const device_debug::breakpoint* left = *(device_debug::breakpoint**)a;
	const device_debug::breakpoint* right = *(device_debug::breakpoint**)b;
	const int result = strcmp(left->debugInterface()->device().tag(), right->debugInterface()->device().tag());
	return result < 0;
}

static int cAddressAscending(const void* a, const void* b)
{
	const device_debug::breakpoint* left = *(device_debug::breakpoint**)a;
	const device_debug::breakpoint* right = *(device_debug::breakpoint**)b;
	return left->address() > right->address();
}

static int cAddressDescending(const void* a, const void* b)
{
	const device_debug::breakpoint* left = *(device_debug::breakpoint**)a;
	const device_debug::breakpoint* right = *(device_debug::breakpoint**)b;
	return left->address() < right->address();
}

static int cConditionAscending(const void* a, const void* b)
{
	const device_debug::breakpoint* left = *(device_debug::breakpoint**)a;
	const device_debug::breakpoint* right = *(device_debug::breakpoint**)b;
	const int result = strcmp(left->condition(), right->condition());
	return result >= 0;
}

static int cConditionDescending(const void* a, const void* b)
{
	const device_debug::breakpoint* left = *(device_debug::breakpoint**)a;
	const device_debug::breakpoint* right = *(device_debug::breakpoint**)b;
	const int result = strcmp(left->condition(), right->condition());
	return result < 0;
}

static int cActionAscending(const void* a, const void* b)
{
	const device_debug::breakpoint* left = *(device_debug::breakpoint**)a;
	const device_debug::breakpoint* right = *(device_debug::breakpoint**)b;
	const int result = strcmp(left->action(), right->action());
	return result >= 0;
}

static int cActionDescending(const void* a, const void* b)
{
	const device_debug::breakpoint* left = *(device_debug::breakpoint**)a;
	const device_debug::breakpoint* right = *(device_debug::breakpoint**)b;
	const int result = strcmp(left->action(), right->action());
	return result < 0;
}


int debug_view_breakpoints::breakpoints(SortMode sort, device_debug::breakpoint**& bpList)
{
	// Alloc
	int numBPs = 0;
	bpList = NULL;
	for (const debug_view_source *source = m_source_list.first(); source != NULL; source = source->next())
	{
		const device_debug& debugInterface = *source->device()->debug();
		for (device_debug::breakpoint *bp = debugInterface.breakpoint_first(); bp != NULL; bp = bp->next())
			numBPs++;
	}
	bpList = new device_debug::breakpoint*[numBPs];

	int bpAddIndex = 0;
	for (const debug_view_source *source = m_source_list.first(); source != NULL; source = source->next())
	{
		// Collect
		device_debug& debugInterface = *source->device()->debug();
		for (device_debug::breakpoint *bp = debugInterface.breakpoint_first(); bp != NULL; bp = bp->next())
		{
			bpList[bpAddIndex] = bp;
			bpAddIndex++;
		}
	}

	// And now for the sort
	switch (m_sortType)
	{
		case SORT_NONE:
			break;
		case SORT_INDEX_ASCENDING:
			qsort(bpList, numBPs, sizeof(device_debug::breakpoint*), cIndexAscending);
			break;
		case SORT_INDEX_DESCENDING:
			qsort(bpList, numBPs, sizeof(device_debug::breakpoint*), cIndexDescending);
			break;
		case SORT_ENABLED_ASCENDING:
			qsort(bpList, numBPs, sizeof(device_debug::breakpoint*), cEnabledAscending);
			break;
		case SORT_ENABLED_DESCENDING:
			qsort(bpList, numBPs, sizeof(device_debug::breakpoint*), cEnabledDescending);
			break;
		case SORT_CPU_ASCENDING:
			qsort(bpList, numBPs, sizeof(device_debug::breakpoint*), cCpuAscending);
			break;
		case SORT_CPU_DESCENDING:
			qsort(bpList, numBPs, sizeof(device_debug::breakpoint*), cCpuDescending);
			break;
		case SORT_ADDRESS_ASCENDING:
			qsort(bpList, numBPs, sizeof(device_debug::breakpoint*), cAddressAscending);
			break;
		case SORT_ADDRESS_DESCENDING:
			qsort(bpList, numBPs, sizeof(device_debug::breakpoint*), cAddressDescending);
			break;
		case SORT_CONDITION_ASCENDING:
			qsort(bpList, numBPs, sizeof(device_debug::breakpoint*), cConditionAscending);
			break;
		case SORT_CONDITION_DESCENDING:
			qsort(bpList, numBPs, sizeof(device_debug::breakpoint*), cConditionDescending);
			break;
		case SORT_ACTION_ASCENDING:
			qsort(bpList, numBPs, sizeof(device_debug::breakpoint*), cActionAscending);
			break;
		case SORT_ACTION_DESCENDING:
			qsort(bpList, numBPs, sizeof(device_debug::breakpoint*), cActionDescending);
			break;
	}

	return numBPs;
}


//-------------------------------------------------
//  view_update - update the contents of the
//  disassembly view
//-------------------------------------------------

void debug_view_breakpoints::view_update()
{
	// Gather a list of all the breakpoints for all the CPUs
	device_debug::breakpoint** bpList = NULL;
	const int numBPs = breakpoints(SORT_NONE, bpList);

	// Set the view region so the scroll bars update
	m_total.y = numBPs+1;

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
			header.catprintf("Address");
			if (m_sortType == SORT_ADDRESS_ASCENDING) header.catprintf("\\");
			else if (m_sortType == SORT_ADDRESS_DESCENDING) header.catprintf("/");
			pad_astring_to_length(header, tableBreaks[3]);
			header.catprintf("Condition");
			if (m_sortType == SORT_CONDITION_ASCENDING) header.catprintf("\\");
			else if (m_sortType == SORT_CONDITION_DESCENDING) header.catprintf("/");
			pad_astring_to_length(header, tableBreaks[4]);
			header.catprintf("Action");
			if (m_sortType == SORT_ACTION_ASCENDING) header.catprintf("\\");
			else if (m_sortType == SORT_ACTION_DESCENDING) header.catprintf("/");
			pad_astring_to_length(header, tableBreaks[5]);

			for (int i = 0; i < m_visible.x; i++)
			{
				dest->byte = (i < header.len()) ? header[i] : ' ';
				dest->attrib = DCA_ANCILLARY;
				dest++;
			}
			continue;
		}

		// Breakpoints
		int bpi = effrow-1;
		if (bpi < numBPs && bpi >= 0)
		{
			device_debug::breakpoint* bp = bpList[bpi];

			astring buffer;
			buffer.printf("%x", bp->index());
			pad_astring_to_length(buffer, tableBreaks[0]);
			buffer.catprintf("%c", bp->enabled() ? 'X' : 'O');
			pad_astring_to_length(buffer, tableBreaks[1]);
			buffer.catprintf("%s", bp->debugInterface()->device().tag());
			pad_astring_to_length(buffer, tableBreaks[2]);
			buffer.catprintf("%s", core_i64_hex_format(bp->address(), bp->debugInterface()->logaddrchars()));
			pad_astring_to_length(buffer, tableBreaks[3]);
			if (astring(bp->condition()) != astring("1"))
			{
				buffer.catprintf("%s", bp->condition());
				pad_astring_to_length(buffer, tableBreaks[4]);
			}
			if (astring(bp->action()) != astring(""))
			{
				buffer.catprintf("%s", bp->action());
				pad_astring_to_length(buffer, tableBreaks[5]);
			}

			for (int i = 0; i < m_visible.x; i++)
			{
				dest->byte = (i < buffer.len()) ? buffer[i] : ' ';
				dest->attrib = DCA_NORMAL;

				// Color disabled breakpoints red
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

	delete[] bpList;
}
