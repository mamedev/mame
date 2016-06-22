// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/****************************************************************************

    debugger.h

    General debugging interfaces
****************************************************************************/

#pragma once

#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

class debugger_commands;
class debugger_cpu;
class debugger_console;


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

private:
	running_machine &   m_machine;

	std::unique_ptr<debugger_commands> m_commands;
	std::unique_ptr<debugger_cpu> m_cpu;
	std::unique_ptr<debugger_console> m_console;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* OSD can call this to safely flush all traces in the event of a crash */
void debugger_flush_all_traces_on_abnormal_exit(void);



/***************************************************************************
    CPU CORE STATIC FUNCTIONS
***************************************************************************/

void debugger_instruction_hook(device_t *device, offs_t curpc);
void debugger_exception_hook(device_t *device, int exception);



/***************************************************************************
    CPU EXECUTION SYSTEM STATIC FUNCTIONS
***************************************************************************/

void debugger_start_cpu_hook(device_t *device, const attotime &endtime);
void debugger_stop_cpu_hook(device_t *device);
void debugger_interrupt_hook(device_t *device, int irqline);


#endif  /* __DEBUGGER_H__ */
