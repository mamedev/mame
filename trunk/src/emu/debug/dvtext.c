/*********************************************************************

    dvtext.c

    Debugger simple text-based views.

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
#include "dvtext.h"
#include "debugcon.h"



//**************************************************************************
//  DEBUG VIEW TEXTBUF
//**************************************************************************

//-------------------------------------------------
//  debug_view_textbuf - constructor
//-------------------------------------------------

debug_view_textbuf::debug_view_textbuf(running_machine &machine, debug_view_type type, debug_view_osd_update_func osdupdate, void *osdprivate, text_buffer &textbuf)
	: debug_view(machine, type, osdupdate, osdprivate),
		m_textbuf(textbuf),
		m_at_bottom(true),
		m_topseq(0)
{
}


//-------------------------------------------------
//  ~debug_view_textbuf - destructor
//-------------------------------------------------

debug_view_textbuf::~debug_view_textbuf()
{
}


//-------------------------------------------------
//  view_update - update a text buffer-based view
//-------------------------------------------------

void debug_view_textbuf::view_update()
{
	// update the console info
	m_total.x = text_buffer_max_width(&m_textbuf);
	m_total.y = text_buffer_num_lines(&m_textbuf);
	if (m_total.x < 80)
		m_total.x = 80;

	// determine the starting sequence number
	UINT32 curseq = 0;
	if (!m_at_bottom)
	{
		curseq = m_topseq;
		if (!text_buffer_get_seqnum_line(&m_textbuf, curseq))
			m_at_bottom = true;
	}
	if (m_at_bottom)
	{
		curseq = text_buffer_line_index_to_seqnum(&m_textbuf, m_total.y - 1);
		if (m_total.y < m_visible.y)
			curseq -= m_total.y - 1;
		else
			curseq -= m_visible.y - 1;
	}
	m_topleft.y = curseq - text_buffer_line_index_to_seqnum(&m_textbuf, 0);

	// loop over visible rows
	debug_view_char *dest = m_viewdata;
	for (UINT32 row = 0; row < m_visible.y; row++)
	{
		const char *line = text_buffer_get_seqnum_line(&m_textbuf, curseq++);
		UINT32 col = 0;

		// if this visible row is valid, add it to the buffer
		if (line != NULL)
		{
			size_t len = strlen(line);
			UINT32 effcol = m_topleft.x;

			// copy data
			while (col < m_visible.x && effcol < len)
			{
				dest->byte = line[effcol++];
				dest->attrib = DCA_NORMAL;
				dest++;
				col++;
			}
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
}


//-------------------------------------------------
//  view_notify - handle notification of updates
//  to visible area
//-------------------------------------------------

void debug_view_textbuf::view_notify(debug_view_notification type)
{
	if (type == VIEW_NOTIFY_VISIBLE_CHANGED)
	{
		// if the bottom line is visible, just track the bottom
		m_at_bottom = (m_total.y >= m_topleft.y && m_total.y <= m_topleft.y + m_visible.y);

		/* otherwise, track the seqence number of the top line */
		if (!m_at_bottom)
			m_topseq = text_buffer_line_index_to_seqnum(&m_textbuf, m_topleft.y);
	}
}



//**************************************************************************
//  DEBUG VIEW CONSOLE
//**************************************************************************

//-------------------------------------------------
//  debug_view_console - constructor
//-------------------------------------------------

debug_view_console::debug_view_console(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate)
	: debug_view_textbuf(machine, DVT_CONSOLE, osdupdate, osdprivate, *debug_console_get_textbuf())
{
}



//**************************************************************************
//  DEBUG VIEW LOG
//**************************************************************************

//-------------------------------------------------
//  debug_view_log - constructor
//-------------------------------------------------

debug_view_log::debug_view_log(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate)
	: debug_view_textbuf(machine, DVT_LOG, osdupdate, osdprivate, *debug_errorlog_get_textbuf())
{
}
