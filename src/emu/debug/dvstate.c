/*********************************************************************

    dvstate.c

    State debugger view.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "debugvw.h"
#include "dvstate.h"



//**************************************************************************
//  DEBUG VIEW STATE SOURCE
//**************************************************************************

//-------------------------------------------------
//  debug_view_state_source - constructor
//-------------------------------------------------

debug_view_state_source::debug_view_state_source(const char *name, device_t &device)
	: debug_view_source(name, &device),
	  m_device(device),
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
	  m_last_update(0),
	  m_state_list(NULL)
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
	state_interface_iterator iter(machine().root_device());
	astring name;
	for (device_state_interface *state = iter.first(); state != NULL; state = iter.next())
	{
		name.printf("%s '%s'", state->device().name(), state->device().tag());
		m_source_list.append(*auto_alloc(machine(), debug_view_state_source(name, state->device())));
	}

	// reset the source to a known good entry
	set_source(*m_source_list.head());
}


//-------------------------------------------------
//  reset - delete all of our state items
//-------------------------------------------------

void debug_view_state::reset()
{
	// free all items in the state list
	while (m_state_list != NULL)
	{
		state_item *oldhead = m_state_list;
		m_state_list = oldhead->m_next;
		auto_free(machine(), oldhead);
	}
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
	state_item **tailptr = &m_state_list;
	*tailptr = auto_alloc(machine(), state_item(REG_CYCLES, "cycles", 8));
	tailptr = &(*tailptr)->m_next;

	// add a beam entry: beamx:1234
	*tailptr = auto_alloc(machine(), state_item(REG_BEAMX, "beamx", 4));
	tailptr = &(*tailptr)->m_next;

	// add a beam entry: beamy:5678
	*tailptr = auto_alloc(machine(), state_item(REG_BEAMY, "beamy", 4));
	tailptr = &(*tailptr)->m_next;

	// add a beam entry: frame:123456
	*tailptr = auto_alloc(machine(), state_item(REG_FRAME, "frame", 6));
	tailptr = &(*tailptr)->m_next;

	// add a flags entry: flags:xxxxxxxx
	*tailptr = auto_alloc(machine(), state_item(STATE_GENFLAGS, "flags", source.m_stateintf->state_string_max_length(STATE_GENFLAGS)));
	tailptr = &(*tailptr)->m_next;

	// add a divider entry
	*tailptr = auto_alloc(machine(), state_item(REG_DIVIDER, "", 0));
	tailptr = &(*tailptr)->m_next;

	// add all registers into it
	for (const device_state_entry *entry = source.m_stateintf->state_first(); entry != NULL; entry = entry->next())
		if (entry->visible())
		{
			*tailptr = auto_alloc(machine(), state_item(entry->index(), entry->symbol(), source.m_stateintf->state_string_max_length(entry->index())));
			tailptr = &(*tailptr)->m_next;
		}

	// count the entries and determine the maximum tag and value sizes
	int count = 0;
	int maxtaglen = 0;
	int maxvallen = 0;
	for (state_item *item = m_state_list; item != NULL; item = item->m_next)
	{
		count++;
		maxtaglen = MAX(maxtaglen, item->m_symbol.len());
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
	if (source.m_execintf != NULL)
		total_cycles = source.m_execintf->total_cycles();

	// find the first entry
	state_item *curitem = m_state_list;
	for (int index = 0; curitem != NULL && index < m_topleft.y; index++)
		curitem = curitem->m_next;

	// loop over visible rows
	screen_device *screen = machine().primary_screen;
	debug_view_char *dest = m_viewdata;
	for (UINT32 row = 0; row < m_visible.y; row++)
	{
		UINT32 col = 0;

		// if this visible row is valid, add it to the buffer
		if (curitem != NULL)
		{
			UINT32 effcol = m_topleft.x;
			UINT8 attrib = DCA_NORMAL;
			UINT32 len = 0;
			astring valstr;

			// get the effective string
			if (curitem->m_index >= REG_FRAME && curitem->m_index <= REG_DIVIDER)
			{
				curitem->m_lastval = curitem->m_currval;
				switch (curitem->m_index)
				{
					case REG_DIVIDER:
						curitem->m_vallen = 0;
						curitem->m_symbol.reset();
						for (int i = 0; i < m_total.x; i++)
							curitem->m_symbol.cat("-");
						break;

					case REG_CYCLES:
						if (source.m_execintf != NULL)
						{
							curitem->m_currval = source.m_execintf->cycles_remaining();
							valstr.printf("%-8d", (UINT32)curitem->m_currval);
						}
						break;

					case REG_BEAMX:
						if (screen != NULL)
						{
							curitem->m_currval = screen->hpos();
							valstr.printf("%4d", (UINT32)curitem->m_currval);
						}
						break;

					case REG_BEAMY:
						if (screen != NULL)
						{
							curitem->m_currval = screen->vpos();
							valstr.printf("%4d", (UINT32)curitem->m_currval);
						}
						break;

					case REG_FRAME:
						if (screen != NULL)
						{
							curitem->m_currval = screen->frame_number();
							valstr.printf("%6d", (UINT32)curitem->m_currval);
						}
						break;
				}
			}
			else
			{
				if (m_last_update != total_cycles)
					curitem->m_lastval = curitem->m_currval;
				curitem->m_currval = source.m_stateintf->state_int(curitem->m_index);
				source.m_stateintf->state_string(curitem->m_index, valstr);
			}

			// see if we changed
			if (curitem->m_lastval != curitem->m_currval)
				attrib = DCA_CHANGED;

			// build up a string
			char temp[256];
			if (curitem->m_symbol.len() < m_divider - 1)
			{
				memset(&temp[len], ' ', m_divider - 1 - curitem->m_symbol.len());
				len += m_divider - 1 - curitem->m_symbol.len();
			}

			memcpy(&temp[len], curitem->m_symbol.cstr(), curitem->m_symbol.len());
			len += curitem->m_symbol.len();

			temp[len++] = ' ';
			temp[len++] = ' ';

			memcpy(&temp[len], valstr.cstr(), curitem->m_vallen);
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
			curitem = curitem->m_next;
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
	: m_next(NULL),
	  m_lastval(0),
	  m_currval(0),
	  m_index(index),
	  m_vallen(valuechars),
	  m_symbol(name)
{
}
