// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    debugcpu.h

    Debugger CPU/memory interface engine.

***************************************************************************/

#pragma once

#ifndef __DEBUGCPU_H__
#define __DEBUGCPU_H__

#include "express.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

const int COMMENT_VERSION               = 1;



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

enum
{
	EXECUTION_STATE_STOPPED,
	EXECUTION_STATE_RUNNING
};


//**************************************************************************
//  CPU DEBUGGING
//**************************************************************************

class debugger_cpu
{
public:
	debugger_cpu(running_machine &machine);

	/* ----- initialization and cleanup ----- */

	/* flushes all traces; this is useful if a trace is going on when we fatalerror */
	void flush_traces();

	void configure_memory(symbol_table &table);


	/* ----- debugging status & information ----- */

	/* return the visible CPU device (the one that commands should apply to) */
	device_t *get_visible_cpu();

	/* true if the debugger is currently stopped within an instruction hook callback */
	bool within_instruction_hook();

	/* return true if the current execution state is stopped */
	bool is_stopped();


	/* ----- symbol table interfaces ----- */

	/* return the global symbol table */
	symbol_table *get_global_symtable();

	/* return the locally-visible symbol table */
	symbol_table *get_visible_symtable();


	/* ----- misc debugger functions ----- */

	/* specifies a debug command script to execute */
	void source_script(const char *file);


	/* ----- debugger comment helpers ----- */

	// save all comments for a given machine
	bool comment_save();

	// load all comments for a given machine
	bool comment_load(bool is_inline);


	/* ----- debugger memory accessors ----- */

	/* return a byte from the specified memory space */
	UINT8 read_byte(address_space &space, offs_t address, int apply_translation);

	/* return a word from the specified memory space */
	UINT16 read_word(address_space &space, offs_t address, int apply_translation);

	/* return a dword from the specified memory space */
	UINT32 read_dword(address_space &space, offs_t address, int apply_translation);

	/* return a qword from the specified memory space */
	UINT64 read_qword(address_space &space, offs_t address, int apply_translation);

	/* return 1,2,4 or 8 bytes from the specified memory space */
	UINT64 read_memory(address_space &space, offs_t address, int size, int apply_translation);

	/* write a byte to the specified memory space */
	void write_byte(address_space &space, offs_t address, UINT8 data, int apply_translation);

	/* write a word to the specified memory space */
	void write_word(address_space &space, offs_t address, UINT16 data, int apply_translation);

	/* write a dword to the specified memory space */
	void write_dword(address_space &space, offs_t address, UINT32 data, int apply_translation);

	/* write a qword to the specified memory space */
	void write_qword(address_space &space, offs_t address, UINT64 data, int apply_translation);

	/* write 1,2,4 or 8 bytes to the specified memory space */
	void write_memory(address_space &space, offs_t address, UINT64 data, int size, int apply_translation);

	/* read 1,2,4 or 8 bytes at the given offset from opcode space */
	UINT64 read_opcode(address_space &space, offs_t offset, int size);

	// getters
	bool within_instruction_hook() const { return m_within_instruction_hook; }
	bool memory_modified() const { return m_memory_modified; }
	bool accessing_memory() const { return m_debugger_access; }
	int execution_state() const { return m_execution_state; }
	device_t *live_cpu() { return m_livecpu; }
	UINT32 get_breakpoint_index() { return m_bpindex++; }
	UINT32 get_watchpoint_index() { return m_wpindex++; }
	UINT32 get_registerpoint_index() { return m_rpindex++; }

	// setters
	void set_visible_cpu(device_t * visiblecpu) { m_visiblecpu = visiblecpu; }
	void set_break_cpu(device_t * breakcpu) { m_breakcpu = breakcpu; }
	void set_within_instruction(bool within_instruction) { m_within_instruction_hook = within_instruction; }
	void set_memory_modified(bool memory_modified) { m_memory_modified = memory_modified; }
	void set_execution_state(int execution_state) { m_execution_state = execution_state; }

	// device_debug helpers
	// [TODO] [RH]: Look into this more later, can possibly merge these two classes
	void start_hook(device_t *device, bool stop_on_vblank);
	void stop_hook(device_t *device);
	void go_next_device(device_t *device);
	void go_vblank();
	void halt_on_next_instruction(device_t *device, util::format_argument_pack<std::ostream> &&args);
	void ensure_comments_loaded();
	void reset_transient_flags();
	void process_source_file();

private:
	static const size_t NUM_TEMP_VARIABLES;

	/* expression handlers */
	UINT64 expression_read_memory(void *param, const char *name, expression_space space, UINT32 address, int size);
	UINT64 expression_read_program_direct(address_space &space, int opcode, offs_t address, int size);
	UINT64 expression_read_memory_region(const char *rgntag, offs_t address, int size);
	void expression_write_memory(void *param, const char *name, expression_space space, UINT32 address, int size, UINT64 data);
	void expression_write_program_direct(address_space &space, int opcode, offs_t address, int size, UINT64 data);
	void expression_write_memory_region(const char *rgntag, offs_t address, int size, UINT64 data);
	expression_error::error_code expression_validate(void *param, const char *name, expression_space space);
	device_t* expression_get_device(const char *tag);

	/* variable getters/setters */
	UINT64 get_cpunum(symbol_table &table, void *ref);
	UINT64 get_beamx(symbol_table &table, void *ref);
	UINT64 get_beamy(symbol_table &table, void *ref);
	UINT64 get_frame(symbol_table &table, void *ref);

	/* internal helpers */
	void on_vblank(screen_device &device, bool vblank_state);

	running_machine&    m_machine;

	device_t *  m_livecpu;
	device_t *  m_visiblecpu;
	device_t *  m_breakcpu;

	FILE *      m_source_file;          // script source file

	std::unique_ptr<symbol_table> m_symtable;           // global symbol table

	bool    m_within_instruction_hook;
	bool    m_vblank_occurred;
	bool    m_memory_modified;
	bool    m_debugger_access;

	int         m_execution_state;
	device_t *  m_stop_when_not_device; // stop execution when the device ceases to be this

	UINT32      m_bpindex;
	UINT32      m_wpindex;
	UINT32      m_rpindex;

	std::unique_ptr<UINT64[]> m_tempvar;

	osd_ticks_t m_last_periodic_update_time;

	bool        m_comments_loaded;
};

#endif
