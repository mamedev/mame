// license:BSD-3-Clause
// copyright-holders:David Broman
/*********************************************************************

    srcdbg_provider.h

    Format-agnostic interface to source-debugging info

***************************************************************************/


#ifndef MAME_EMU_DEBUG_SRCDBG_PROVIDER_H
#define MAME_EMU_DEBUG_SRCDBG_PROVIDER_H

#pragma once

#include "express.h"

// Simple file index / line number pair
class file_line
{
public:
	file_line(u32 file_index, u32 line_number)
	{
		set(file_index, line_number);
	}

	file_line(const file_line & that)
	{
		set(that.m_file_index, that.m_line_number);
	}

	file_line()
	{
		set(0, 0);
	}

	void set(u32 file_index, u32 line_number)
	{
		m_file_index = file_index;
		m_line_number = line_number;
	}

	u32 file_index() { return m_file_index; };
	u32 line_number() { return m_line_number; };

	bool operator == (const file_line & that)
	{
		return (this->m_file_index == that.m_file_index) && (this->m_line_number == that.m_line_number);
	}

	bool operator != (const file_line & that)
	{
		return !(*this == that);
	}

private:
	u32 m_file_index;
	u32 m_line_number;
};


// Each source-debugging information file format derives from this
// abstract base class to provide debugging information to the rest of
// the debugger
class srcdbg_provider_base
{
public:
	// Represents a single source file to be debugged.  Includes the original
	// "built" path (as output by the assembler / compiler that generated the
	// source-debugging information file), and a "local" path as it appears on
	// the MAME user's system.  These can be different when the debugged
	// program was built on a different system from the one in which MAME runs
	class source_file_path
	{
	public:
		source_file_path(std::string && built_p, std::string && local_p)
			: m_built(std::move(built_p))
			, m_local(std::move(local_p))
		{
		}

		const char * built() const { return m_built.c_str(); }
		const char * local() const { return m_local.c_str(); }

	private:
		std::string m_built;
		std::string m_local;
	};


	// Represents a global fixed symbol to the rest of the debugger.  Such symbols
	// are not limited to a scope, and represent a single value (such as a
	// single variable address)
	class global_fixed_symbol
	{
	public:
		global_fixed_symbol(const std::string & name, offs_t value, u32 flags)
			: m_name(name)
			, m_value(value)
			, m_flags(flags)
		{
		}

		const char * name() const { return m_name.c_str(); };
		offs_t value() const { return m_value; };
		u32 flags() const { return m_flags; };

	private:
		std::string m_name;
		offs_t m_value;
		u32 m_flags;
	};


	// Represents a local fixed symbol to the rest of the debugger.  Such symbols
	// are limited to ranges of addresses (scopes), and represent a single value
	// (such as a single variable address).
	class local_fixed_symbol
	{
	public:
		local_fixed_symbol(const std::string & name, std::vector<std::pair<offs_t,offs_t>> && ranges, offs_t value)
			: m_name(name)
			, m_ranges(std::move(ranges))
			, m_value_integer(value)
		{
		}

		const char * name() const { return m_name.c_str(); };
		const std::vector<std::pair<offs_t,offs_t>> & ranges() const { return m_ranges; };
		offs_t value() const { return m_value_integer; };

	private:
		std::string m_name;
		std::vector<std::pair<offs_t,offs_t>> m_ranges;
		offs_t m_value_integer;
	};


	// Represents a local relative symbol to the rest of the debugger.  Such symbols
	// are limited to ranges of addresses (scopes), and represent a value calculated
	// as an offset to a register (such as an offset to a stack or frame register).
	// Each scope has its own, potentially different, register-offset calculation
	// to determine the symbol's value.
	class local_relative_symbol
	{
	public:
		local_relative_symbol(const std::string & name, std::vector<symbol_table::local_range_expression> && ranges)
			: m_name(name)
			, m_ranges(std::move(ranges))
		{
		}
		const char * name() const { return m_name.c_str(); };
		const std::vector<symbol_table::local_range_expression> & ranges() const { return m_ranges; };

	private:
		std::string m_name;
		std::vector<symbol_table::local_range_expression> m_ranges;
	};

	typedef std::pair<offs_t,offs_t> address_range;

	// ------------------------------------------------------------------------
	// Base implementation
	// ------------------------------------------------------------------------

	// Called on startup to load the source-debugging information file and return
	// an instance of a subclass of this abstract base class
	static std::unique_ptr<srcdbg_provider_base> create_debug_info(running_machine &machine);

	virtual ~srcdbg_provider_base() {};

	// Return symbol_table objects for globals and locals present in the source-level
	// debugging information file.
	virtual void get_srcdbg_symbols(
		symbol_table ** symtable_srcdbg_globals,
		symbol_table ** symtable_srcdbg_locals,
		symbol_table * parent,
		device_t * device,
		const device_state_interface * state) const;


	// ------------------------------------------------------------------------
	// Interface implemented by derived classes
	// ------------------------------------------------------------------------

	// Called later during startup, after device_state_interfaces are available.
	// Generates expressions required to implement local relative symbol evaluation rules.
	virtual void complete_local_relative_initialization() = 0;

	// Returns the number of source file paths
	virtual u32 num_files() const = 0;

	// Returns the source_file_path corresponding to the specified 0-based file index
	virtual const source_file_path & file_index_to_path(u32 file_index) const = 0;

	// Returns the 0-based file index corresponding to the specified source file path
	virtual std::optional<u32> file_path_to_index(const char * file_path) const = 0;

	// Returns all address ranges corresponding to the specified source file / line number pair
	virtual void file_line_to_address_ranges(u32 file_index, u32 line_number, std::vector<address_range> & ranges) const = 0;

	// Returns true and populates loc with the source file / line number pair corresponding
	// to the specified address, or returns false if no such pair exists
	virtual bool address_to_file_line (offs_t address, file_line & loc) const = 0;

	// Return lists of various types of symbols
	virtual const std::vector<global_fixed_symbol> & global_fixed_symbols() const = 0;
	virtual const std::vector<local_fixed_symbol> & local_fixed_symbols() const = 0;
	virtual const std::vector<local_relative_symbol> & local_relative_symbols() const = 0;

	// Called to process command-line option or user-invoked command to change the
	// address offset for line numbers and symbols
	virtual void set_offset(s32 offset) = 0;

private:
	// Returns address offset currently in use.  Used internally when populating symbol tables
	virtual s32 get_offset() const = 0;
};


#endif // MAME_EMU_DEBUG_SRCDBG_PROVIDER_H
