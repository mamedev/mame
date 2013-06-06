/*********************************************************************

    dvpoints.c

	Breakpoint debugger view.

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
#include "dvbpoints.h"
#include "debugcpu.h"



//**************************************************************************
//  DEBUG VIEW BREAK POINTS
//**************************************************************************

//-------------------------------------------------
//  debug_view_breakpoints - constructor
//-------------------------------------------------

debug_view_breakpoints::debug_view_breakpoints(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate)
	: debug_view(machine, DVT_BREAK_POINTS, osdupdate, osdprivate)
{
	// fail if no available sources
	enumerate_sources();
	if (m_source_list.count() == 0)
		throw std::bad_alloc();

	// configure the view
	m_total.y = 50;             // TODO
	m_supports_cursor = true;
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
		m_source_list.append(*auto_alloc(machine(), debug_view_source(name.cstr(), &dasm->device())));
	}

	// reset the source to a known good entry
	set_source(*m_source_list.head());
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

//-------------------------------------------------
//  view_update - update the contents of the
//  disassembly view
//-------------------------------------------------

void debug_view_breakpoints::view_update()
{
	const debug_view_source& source = *m_source;
	const device_debug& debugInterface = *source.device()->debug();
	debug_view_char *dest = m_viewdata;

	// Gather a list of all the breakpoints in reverse order
	int numBPs = 0;
	device_debug::breakpoint** bpList = NULL;
	if (debugInterface.breakpoint_first() != NULL)
	{
		// Alloc
		for (device_debug::breakpoint *bp = debugInterface.breakpoint_first(); bp != NULL; bp = bp->next())
			numBPs++;
		bpList = new device_debug::breakpoint*[numBPs];
		
		// Collect
		int i = 1;
		for (device_debug::breakpoint *bp = debugInterface.breakpoint_first(); bp != NULL; bp = bp->next())
		{
			bpList[numBPs-i] = bp;
			i++;
		}
	}

	// Draw
	for (int row = 0; row < m_visible.y; row++)
	{
		UINT32 effrow = m_topleft.y + row;
		
		// Header
		if (effrow == 0)
		{
			astring header = "ID   En  Address          Condition         Action";
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
			pad_astring_to_length(buffer, 5);
			buffer.catprintf("%c", bp->enabled() ? 'X' : 'O');
			pad_astring_to_length(buffer, 9);
			buffer.catprintf("%s", core_i64_hex_format(bp->address(), debugInterface.logaddrchars()));
			pad_astring_to_length(buffer, 26);
			if (astring(bp->condition()) != astring("1"))
			{
				buffer.catprintf("%s", bp->condition());
				pad_astring_to_length(buffer, 44);
			}
			if (astring(bp->action()) != astring(""))
			{
				buffer.catprintf("%s", bp->action());
				pad_astring_to_length(buffer, 60);
			}
			
			for (int i = 0; i < m_visible.x; i++)
			{
				dest->byte = (i < buffer.len()) ? buffer[i] : ' ';
				dest->attrib = DCA_NORMAL;
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
	
	delete bpList;
}
