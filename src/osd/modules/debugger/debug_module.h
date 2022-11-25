// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * debug_module.h
 *
 */

#ifndef MAME_OSD_DEBUGGER_DEBUG_MODULE_H
#define MAME_OSD_DEBUGGER_DEBUG_MODULE_H

#pragma once

#include "osdepend.h"
#include "modules/osdmodule.h"

//============================================================
//  CONSTANTS
//============================================================

#define OSD_DEBUG_PROVIDER "debugger"

class debug_module
{
public:

	virtual ~debug_module() { }

	virtual void init_debugger(running_machine &machine) = 0;
	virtual void wait_for_debugger(device_t &device, bool firststop) = 0;
	virtual void debugger_update() = 0;
};



#endif // MAME_OSD_DEBUGGER_DEBUG_MODULE_H
