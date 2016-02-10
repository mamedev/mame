// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
//============================================================
//
//  none.c - stubs for linking when NO_DEBUGGER is defined
//
//============================================================

#include "debug_module.h"
#include "modules/osdmodule.h"

#include "debug/debugcpu.h"

class debug_none : public osd_module, public debug_module
{
public:
	debug_none()
	: osd_module(OSD_DEBUG_PROVIDER, "none"), debug_module(),
		m_machine(nullptr)
	{
	}

	virtual ~debug_none() { }

	virtual int init(const osd_options &options) override { return 0; }
	virtual void exit() override { }

	virtual void init_debugger(running_machine &machine) override;
	virtual void wait_for_debugger(device_t &device, bool firststop) override;
	virtual void debugger_update() override;

private:
	running_machine *m_machine;
};

void debug_none::init_debugger(running_machine &machine)
{
	m_machine = &machine;
}

void debug_none::wait_for_debugger(device_t &device, bool firststop)
{
	debug_cpu_get_visible_cpu(*m_machine)->debug()->go();
}

void debug_none::debugger_update()
{
}

MODULE_DEFINITION(DEBUG_NONE, debug_none)
