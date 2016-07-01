// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    dvstate.c

    State debugger view.

***************************************************************************/

#include "emu.h"
#include "debugvw.h"
#include "dvstate.h"

const int debug_view_state::REG_DIVIDER;
const int debug_view_state::REG_CYCLES;
const int debug_view_state::REG_BEAMX;
const int debug_view_state::REG_BEAMY;
const int debug_view_state::REG_FRAME;


//**************************************************************************
//  DEBUG VIEW STATE SOURCE
//**************************************************************************

//-------------------------------------------------
//  debug_view_state_source - constructor
//-------------------------------------------------

debug_view_state_source::debug_view_state_source(const char *name, device_t &device)
	: debug_view_source(name, &device),
		m_stateintf(dynamic_cast<device_state_interface *>(&device)),
		m_execintf(dynamic_cast<device_execute_interface *>(&device))
{
}



//**************************************************************************
//  DEBUG VIEW STATE
//**************************************************************************

//-------------------------------------------------
//  debug_view_state - constructor
//-------------------------------------------------

debug_view_state::debug_view_state(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate)
	: debug_view(machine, DVT_STATE, osdupdate, osdprivate),
		m_divider(0),
		m_last_update(0)
{
	// fail if no available sources
	enumerate_sources();
	if (m_source_list.count() == 0)
		throw std::bad_alloc();
}


//-------------------------------------------------
//  ~debug_view_state - destructor
//-------------------------------------------------

debug_view_state::~debug_view_state()
{
	reset();
}


//-------------------------------------------------
//  enumerate_sources - enumerate all possible
//  sources for a registers view
//-------------------------------------------------

void debug_view_state::enumerate_sources()
{
	// start with an empty list
	m_source_list.reset();

	// iterate over devices that have state interfaces
	std::string name;
	for (device_state_interface &state : state_interface_iterator(machine().root_device()))
	{
		name = string_format("%s '%s'", state.device().name(), state.device().tag());
		m_source_list.append(*global_alloc(debug_view_state_source(name.c_str(), state.device())));
	}

	// reset the source to a known good entry
	set_source(*m_source_list.first());
}


//-------------------------------------------------
//  reset - delete all of our state items
//-------------------------------------------------

void debug_view_state::reset()
{
	// free all items in the state list
	m_state_list.clear();
}


//-------------------------------------------------
//  recompute - recompute all info for the
//  registers view
//-------------------------------------------------

void debug_view_state::recompute()
{
	const debug_view_state_source &source = downcast<const debug_view_state_source &>(*m_source);

	// start with a blank list
	reset();

	// add a cycles entry: cycles:99999999
	m_state_list.push_back(std::make_unique<state_item>(REG_CYCLES, "cycles", 8));

	// add a beam entry: beamx:1234
	m_state_list.push_back(std::make_unique<state_item>(REG_BEAMX, "beamx", 4));

	// add a beam entry: beamy:5678
	m_state_list.push_back(std::make_unique<state_item>(REG_BEAMY, "beamy", 4));

	// add a beam entry: frame:123456
	m_state_list.push_back(std::make_unique<state_item>(REG_FRAME, "frame", 6));

	// add a flags entry: flags:xxxxxxxx
	m_state_list.push_back(std::make_unique<state_item>(STATE_GENFLAGS, "flags", source.m_stateintf->state_string_max_length(STATE_GENFLAGS)));

	// add a divider entry
	m_state_list.push_back(std::make_unique<state_item>(REG_DIVIDER, "", 0));

	// add all registers into it
	for (auto &entry : source.m_stateintf->state_entries())
		if (entry->divider())
		{
			m_state_list.push_back(std::make_unique<state_item>(REG_DIVIDER, "", 0));
		}
		else if (entry->visible())
		{
			m_state_list.push_back(std::make_unique<state_item>(entry->index(), entry->symbol(), source.m_stateintf->state_string_max_length(entry->index())));
		}

	// count the entries and determine the maximum tag and value sizes
	int count = 0;
	int maxtaglen = 0;
	int maxvallen = 0;
	for (auto &item : m_state_list)
	{
		count++;
		maxtaglen = MAX(maxtaglen, item->m_symbol.length());
		maxvallen = MAX(maxvallen, item->m_vallen);
	}

	// set the current divider and total cols
	m_divider = 1 + maxtaglen + 1;
	m_total.x = 1 + maxtaglen + 2 + maxvallen + 1;
	m_total.y = count;
	m_topleft.x = 0;
	m_topleft.y = 0;

	// no longer need to recompute
	m_recompute = false;
}


//-------------------------------------------------
//  view_notify - handle notification of updates
//  to cursor changes
//-------------------------------------------------

void debug_view_state::view_notify(debug_view_notification type)
{
	if (type == VIEW_NOTIFY_SOURCE_CHANGED)
		m_recompute = true;
}


//-------------------------------------------------
//  view_update - update the contents of the
//  register view
//-------------------------------------------------

void debug_view_state::view_update()
{
	// if our assumptions changed, revisit them
	if (m_recompute)
		recompute();

	// get cycle count if we have an execute interface
	const debug_view_state_source &source = downcast<const debug_view_state_source &>(*m_source);
	UINT64 total_cycles = 0;
	if (source.m_execintf != nullptr)
		total_cycles = source.m_execintf->total_cycles();

	// find the first entry
	auto it = m_state_list.begin();

	// loop over visible rows
	screen_device *screen = machine().first_screen();
	debug_view_char *dest = &m_viewdata[0];
	for (UINT32 row = 0; row < m_visible.y; row++)
	{
		UINT32 col = 0;
		
		// if this visible row is valid, add it to the buffer
		if (it != m_state_list.end())
		{
			state_item *curitem = it->get();

			UINT32 effcol = m_topleft.x;
			UINT8 attrib = DCA_NORMAL;
			UINT32 len = 0;
			std::string valstr;

			// get the effective string
			if (curitem->m_index >= REG_FRAME && curitem->m_index <= REG_DIVIDER)
			{
				curitem->m_lastval = curitem->m_currval;
				switch (curitem->m_index)
				{
					case REG_DIVIDER:
						curitem->m_vallen = 0;
						curitem->m_symbol.clear();
						for (int i = 0; i < m_total.x; i++)
							curitem->m_symbol.append("-");
						break;

					case REG_CYCLES:
						if (source.m_execintf != nullptr)
						{
							curitem->m_currval = source.m_execintf->cycles_remaining();
							valstr = string_format("%-8d", (UINT32)curitem->m_currval);
						}
						break;

					case REG_BEAMX:
						if (screen != nullptr)
						{
							curitem->m_currval = screen->hpos();
							valstr = string_format("%4d", (UINT32)curitem->m_currval);
						}
						break;

					case REG_BEAMY:
						if (screen != nullptr)
						{
							curitem->m_currval = screen->vpos();
							valstr = string_format("%4d", (UINT32)curitem->m_currval);
						}
						break;

					case REG_FRAME:
						if (screen != nullptr)
						{
							curitem->m_currval = screen->frame_number();
							valstr = string_format("%6d", (UINT32)curitem->m_currval);
						}
						break;
				}
			}
			else
			{
				if (m_last_update != total_cycles)
					curitem->m_lastval = curitem->m_currval;
				curitem->m_currval = source.m_stateintf->state_int(curitem->m_index);
				valstr = source.m_stateintf->state_string(curitem->m_index);
			}

			// see if we changed
			if (curitem->m_lastval != curitem->m_currval)
				attrib = DCA_CHANGED;

			// build up a string
			char temp[256];
			if (curitem->m_symbol.length() < m_divider - 1)
			{
				memset(&temp[len], ' ', m_divider - 1 - curitem->m_symbol.length());
				len += m_divider - 1 - curitem->m_symbol.length();
			}

			memcpy(&temp[len], curitem->m_symbol.c_str(), curitem->m_symbol.length());
			len += curitem->m_symbol.length();

			temp[len++] = ' ';
			temp[len++] = ' ';

			memcpy(&temp[len], valstr.c_str(), curitem->m_vallen);
			len += curitem->m_vallen;

			temp[len++] = ' ';
			temp[len] = 0;

			// copy data
			while (col < m_visible.x && effcol < len)
			{
				dest->byte = temp[effcol++];
				dest->attrib = attrib | ((effcol <= m_divider) ? DCA_ANCILLARY : DCA_NORMAL);
				dest++;
				col++;
			}

			// advance to the next item
			++it;
		}

		// fill the rest with blanks
		while (col < m_visible.x)
		{
			dest->byte = ' ';
			dest->attrib = DCA_NORMAL;
			dest++;
			col++;
		}
	}

	// remember the last update
	m_last_update = total_cycles;
}


//-------------------------------------------------
//  state_item - constructor
//-------------------------------------------------

debug_view_state::state_item::state_item(int index, const char *name, UINT8 valuechars)
	: m_lastval(0),
		m_currval(0),
		m_index(index),
		m_vallen(valuechars),
		m_symbol(name)
{
}
