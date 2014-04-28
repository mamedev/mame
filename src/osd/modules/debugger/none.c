//============================================================
//
//  none.c - stubs for linking when NO_DEBUGGER is defined
//
//============================================================

#include "none.h"
#include "debug/debugcpu.h"

const osd_debugger_type OSD_DEBUGGER_NONE = &osd_debugger_creator<debugger_none>;

//-------------------------------------------------
//  debugger_none - constructor
//-------------------------------------------------
debugger_none::debugger_none(const osd_interface &osd)
	: osd_debugger_interface(osd)
{
}

void debugger_none::init_debugger()
{
}

void debugger_none::wait_for_debugger(device_t &device, bool firststop)
{
	debug_cpu_get_visible_cpu(m_osd.machine())->debug()->go();
}

void debugger_none::debugger_update()
{
}

void debugger_none::debugger_exit()
{
}

