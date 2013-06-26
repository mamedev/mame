/*********************************************************************

    dvtext.h

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

#ifndef __DVTEXT_H__
#define __DVTEXT_H__

#include "debugvw.h"
#include "textbuf.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// debug_view_textbuf contains data specific to a textbuffer view
class debug_view_textbuf : public debug_view
{
	friend class debug_view_manager;

protected:
	// construction/destruction
	debug_view_textbuf(running_machine &machine, debug_view_type type, debug_view_osd_update_func osdupdate, void *osdprivate, text_buffer &textbuf);
	virtual ~debug_view_textbuf();

protected:
	// view overrides
	virtual void view_update();
	virtual void view_notify(debug_view_notification type);

private:
	// internal state
	text_buffer &       m_textbuf;              /* pointer to the text buffer */
	bool                m_at_bottom;                /* are we tracking new stuff being added? */
	UINT32              m_topseq;                   /* sequence number of the top line */
};


// debug_view_console describes a console view
class debug_view_console : public debug_view_textbuf
{
	friend class debug_view_manager;
	debug_view_console(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate);
};


// debug_view_textbuf describes an error log view
class debug_view_log : public debug_view_textbuf
{
	friend class debug_view_manager;
	debug_view_log(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate);
};


#endif
