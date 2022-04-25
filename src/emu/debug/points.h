// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    points.h

    Debugger breakpoints, watchpoints, etc.

***************************************************************************/

#ifndef MAME_EMU_DEBUG_POINTS_H
#define MAME_EMU_DEBUG_POINTS_H

#pragma once

#include "debugcpu.h"
#include "express.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> debug_breakpoint

class debug_breakpoint
{
	friend class device_debug;

public:
	// construction/destruction
	debug_breakpoint(
					device_debug* debugInterface,
					symbol_table &symbols,
					int index,
					offs_t address,
					const char *condition = nullptr,
					const char *action = nullptr);

	// getters
	const device_debug *debugInterface() const { return m_debugInterface; }
	int index() const { return m_index; }
	bool enabled() const { return m_enabled; }
	offs_t address() const { return m_address; }
	const char *condition() const { return m_condition.original_string(); }
	const char *action() const { return m_action.c_str(); }

	// setters
	void setEnabled(bool value) { m_enabled = value; } // FIXME: need to update breakpoint flags but it's a private method

private:
	// internals
	bool hit(offs_t pc);
	const device_debug * m_debugInterface;           // the interface we were created from
	int                  m_index;                    // user reported index
	bool                 m_enabled;                  // enabled?
	offs_t               m_address;                  // execution address
	parsed_expression    m_condition;                // condition
	std::string          m_action;                   // action
};

// ======================> debug_watchpoint

class debug_watchpoint
{
	friend class device_debug;

public:
	// construction/destruction
	debug_watchpoint(
					device_debug* debugInterface,
					symbol_table &symbols,
					int index,
					address_space &space,
					read_or_write type,
					offs_t address,
					offs_t length,
					const char *condition = nullptr,
					const char *action = nullptr);
	~debug_watchpoint();

	// getters
	const device_debug *debugInterface() const { return m_debugInterface; }
	address_space &space() const { return m_space; }
	int index() const { return m_index; }
	read_or_write type() const { return m_type; }
	bool enabled() const { return m_enabled; }
	offs_t address() const { return m_address; }
	offs_t length() const { return m_length; }
	const char *condition() const { return m_condition.original_string(); }
	const std::string &action() const { return m_action; }

	// setters
	void setEnabled(bool value);

	// internals
	bool hit(int type, offs_t address, int size);

private:
	void install(read_or_write mode);
	void triggered(read_or_write type, offs_t address, u64 data, u64 mem_mask);

	device_debug * m_debugInterface;                 // the interface we were created from
	memory_passthrough_handler m_phr;                // passthrough handler reference, read access
	memory_passthrough_handler m_phw;                // passthrough handler reference, write access
	address_space &      m_space;                    // address space
	int                  m_index;                    // user reported index
	bool                 m_enabled;                  // enabled?
	read_or_write        m_type;                     // type (read/write)
	offs_t               m_address;                  // start address
	offs_t               m_length;                   // length of watch area
	parsed_expression    m_condition;                // condition
	std::string          m_action;                   // action
	util::notifier_subscription m_notifier;          // address map change notifier ID

	offs_t               m_start_address[3];         // the start addresses of the checks to install
	offs_t               m_end_address[3];           // the end addresses
	u64                  m_masks[3];                 // the access masks
	bool                 m_installing;               // prevent recursive multiple installs
};

// ======================> debug_registerpoint

class debug_registerpoint
{
	friend class device_debug;

public:
	// construction/destruction
	debug_registerpoint(symbol_table &symbols, int index, const char *condition, const char *action = nullptr);

	// getters
	int index() const { return m_index; }
	bool enabled() const { return m_enabled; }
	const char *condition() const { return m_condition.original_string(); }
	const char *action() const { return m_action.c_str(); }

private:
	// internals
	bool hit();

	int                 m_index;                    // user reported index
	bool                m_enabled;                  // enabled?
	parsed_expression   m_condition;                // condition
	std::string         m_action;                   // action
};

#endif // MAME_EMU_DEBUG_POINTS_H
