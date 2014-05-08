// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    none.h

    Dummy debugger interface.

*******************************************************************c********/

#pragma once

#ifndef __DEBUGGER_NONE_H__
#define __DEBUGGER_NONE_H__

#include "osdepend.h"

class debugger_none : public osd_debugger_interface
{
public:
	// construction/destruction
	debugger_none(const osd_interface &osd);
	virtual ~debugger_none() { }
	
	virtual void init_debugger();
	virtual void wait_for_debugger(device_t &device, bool firststop);
	virtual void debugger_update();
	virtual void debugger_exit();
};

extern const osd_debugger_type OSD_DEBUGGER_NONE;

#endif  /* __DEBUGGER_NONE_H__ */
