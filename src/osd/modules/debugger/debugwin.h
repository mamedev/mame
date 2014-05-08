// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  debugwin.h - Win32 debug window handling
//
//============================================================


#pragma once

#ifndef __DEBUGGER_WINDOWS_H__
#define __DEBUGGER_WINDOWS_H__

#include "osdepend.h"

class debugger_windows : public osd_debugger_interface
{
public:
	// construction/destruction
	debugger_windows(const osd_interface &osd);
	virtual ~debugger_windows() { }
	
	virtual void init_debugger();
	virtual void wait_for_debugger(device_t &device, bool firststop);
	virtual void debugger_update();
	virtual void debugger_exit();
};

extern const osd_debugger_type OSD_DEBUGGER_WINDOWS;

#endif /* __DEBUGGER_WINDOWS_H__ */
