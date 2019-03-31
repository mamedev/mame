// license:BSD-3-Clause
// copyright-holders:Bartman/Abyss
//============================================================
//
//  debuggdb.cpp - gdbserver interface
//
//============================================================

#include "emu.h"
#include "debug_module.h"
#include "modules/osdmodule.h"

#include "debug/debugcpu.h"
#include "debugger.h"

class debug_gdb : public osd_module, public debug_module
{
public:
	debug_gdb()
	: osd_module(OSD_DEBUG_PROVIDER, "none"), debug_module(),
		m_machine(nullptr)
	{
	}

	virtual ~debug_gdb() { }

	virtual int init(const osd_options &options) override { return 0; }
	virtual void exit() override { }

	virtual void init_debugger(running_machine &machine) override;
	virtual void wait_for_debugger(device_t &device, bool firststop) override;
	virtual void debugger_update() override;

private:
	running_machine *m_machine;
};

void debug_gdb::init_debugger(running_machine &machine)
{
	m_machine = &machine;
}

void debug_gdb::wait_for_debugger(device_t &device, bool firststop)
{
	m_machine->debugger().cpu().get_visible_cpu()->debug()->go();
}

void debug_gdb::debugger_update()
{
}

MODULE_DEFINITION(DEBUG_GDB, debug_gdb)
