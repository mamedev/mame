// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/****************************************************************************

    debugger.h

    General debugging interfaces

****************************************************************************/

#ifndef MAME_EMU_DEBUGGER_H
#define MAME_EMU_DEBUGGER_H

#pragma once


// ======================> debugger_manager

class debugger_manager
{
public:
	// construction/destruction
	debugger_manager(running_machine &machine);
	~debugger_manager();

	// break into the debugger
	void debug_break();

	bool within_instruction_hook();

	// redraw the current video display
	void refresh_display();

	// getters
	running_machine &machine() const { return m_machine; }
	debugger_commands &commands() const { return *m_commands; }
	debugger_cpu &cpu() const { return *m_cpu; }
	debugger_console &console() const { return *m_console; }
	srcdbg_provider_base * srcdbg_provider() const { return m_debug_info.get(); }     // Note: can be null!

private:
	std::unique_ptr<srcdbg_provider_base> load_debug_info(running_machine &machine);
	running_machine &   m_machine;

	std::unique_ptr<debugger_commands> m_commands;
	std::unique_ptr<debugger_cpu> m_cpu;
	std::unique_ptr<debugger_console> m_console;
	std::unique_ptr<srcdbg_provider_base> m_debug_info;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* OSD can call this to safely flush all traces in the event of a crash */
void debugger_flush_all_traces_on_abnormal_exit();

#endif // MAME_EMU_DEBUGGER_H
