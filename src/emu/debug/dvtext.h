// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    dvtext.h

    Debugger simple text-based views.

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
	virtual void view_update() override;
	virtual void view_notify(debug_view_notification type) override;

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
