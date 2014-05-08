/*********************************************************************

    debugint.c

    Internal debugger frontend using render interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __DEBUGGER_INTERNAL_H__
#define __DEBUGGER_INTERNAL_H__

#include "osdepend.h"

class debugger_internal : public osd_debugger_interface
{
public:
	// construction/destruction
	debugger_internal(const osd_interface &osd);
	virtual ~debugger_internal() { }
	
	virtual void init_debugger();
	virtual void wait_for_debugger(device_t &device, bool firststop);
	virtual void debugger_update();
	virtual void debugger_exit();
};

extern const osd_debugger_type OSD_DEBUGGER_INTERNAL;

#endif /* __DEBUGGER_INTERNAL_H__ */
