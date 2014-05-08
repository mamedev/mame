//============================================================
//
//  debugqt.h - SDL/QT debug window handling
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#pragma once

#ifndef __DEBUGGER_QT_H__
#define __DEBUGGER_QT_H__

#include "osdepend.h"

class debugger_qt : public osd_debugger_interface
{
public:
	// construction/destruction
	debugger_qt(const osd_interface &osd);
	virtual ~debugger_qt();
	
	virtual void init_debugger();
	virtual void wait_for_debugger(device_t &device, bool firststop);
	virtual void debugger_update();
	virtual void debugger_exit();
};

extern const osd_debugger_type OSD_DEBUGGER_QT;

#endif /* __DEBUGGER_QT_H__ */
