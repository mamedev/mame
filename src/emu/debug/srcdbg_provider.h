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
#include "../../lib/srcdbg/srcdbg_format.h"

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


// Each source-debugging information file format provider derives from this
// abstract base class to provide debugging information to the rest of
// the debugger
class srcdbg_provider_base
{
public:
	typedef std::pair<offs_t,offs_t> address_range;

	// Represents a single source file to be debugged.  Includes the original
	// "built" path (as output by the assembler / compiler that generated the
	// source-debugging information file), and a "local" path as it appears on
	// the MAME user's host system.  These can be different when the debugged
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
	// single variable address or label)
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


	// These represent a local fixed symbol to the rest of the debugger.  Such symbols
	// are limited to ranges of addresses (scopes), and represent a fixed value
	// (such as a single variable address).
	class local_fixed_symbol
	{
	public:
		local_fixed_symbol(const std::string & name, std::vector<local_fixed_address_range> && ranges)
			: m_name(name)
			, m_ranges(std::move(ranges))
		{
		}

		const char * name() const { return m_name.c_str(); };
		const std::vector<local_fixed_address_range> & ranges() const { return m_ranges; };

	private:
		std::string m_name;
		std::vector<local_fixed_address_range> m_ranges;
	};


	// These represent a local relative symbol to the rest of the debugger.  Such symbols
	// are limited to ranges of addresses (scopes), and represent a value calculated
	// as an offset to a register (such as an offset to a stack or frame register).
	// Each scope has its own, potentially different, register-offset calculation
	// to determine the symbol's value.
	// local_range_expression pairs an expression with an address range

	class local_relative_symbol
	{
	public:
		local_relative_symbol(const std::string & name, std::vector<local_relative_eval_rule> eval_rules)
			: m_name(name)
			, m_eval_rules(eval_rules)
		{
		}

		const char * name() const { return m_name.c_str(); };
		const std::vector<local_relative_eval_rule> & eval_rules() const { return m_eval_rules; };

	private:
		std::string m_name;
		std::vector<local_relative_eval_rule> m_eval_rules;
	};




	
	// ------------------------------------------------------------------------
	// Base implementation
	// ------------------------------------------------------------------------


	// Factory called by srcdbg_info::create_debug_info to read a debugging
	// information file and construct an instance of a concrete subclass
	// of srcdbg_provider_base from it
	static srcdbg_provider_base * create_debug_info(running_machine &machine, const std::string & di_path);

	srcdbg_provider_base();
	virtual ~srcdbg_provider_base() {};

	// ------------------------------------------------------------------------
	// Interface implemented by derived classes
	// ------------------------------------------------------------------------

	// Returns the number of source file paths
	virtual u32 num_files() const = 0;

	// Returns the source_file_path corresponding to the specified 0-based file index
	virtual bool file_index_to_path(u32 file_index, const source_file_path ** path) const = 0;

	// Returns the 0-based file index corresponding to the specified source file path
	virtual std::optional<u32> file_path_to_index(const char * file_path) const = 0;

	// Returns all address ranges corresponding to the specified source file / line number pair
	virtual void file_line_to_address_ranges(u32 file_index, u32 line_number, std::vector<address_range> & ranges) const = 0;

	// Returns true and populates loc with the source file / line number pair corresponding
	// to the specified address, or returns false if no such pair exists
	virtual bool address_to_file_line (offs_t address, file_line & loc) const = 0;

	// These are called by symbol table entries' is_in_scope() and value()
	// for the different kinds of srcdbg symbols.  Index parameters correspond
	// to the symbol vector fields on this class.
	virtual bool is_global_fixed_sym_in_scope(u32 global_idx) const = 0;
	virtual u64 get_global_fixed_sym_value(u32 global_idx) const = 0;
	virtual bool is_local_fixed_sym_in_scope(u32 local_fixed_idx) const = 0;
	virtual u64 get_local_fixed_sym_value(u32 local_fixed_idx) const = 0;
	virtual bool is_local_relative_sym_in_scope(u32 m_local_relative_idx) const = 0;
	virtual u64 get_local_relative_sym_value(u32 local_relative_idx, symbol_table & symtable) const = 0;

	// Return lists of various types of symbols
	const std::vector<global_fixed_symbol> & global_fixed_symbols() const { return m_global_fixed_symbols; }
	const std::vector<local_fixed_symbol> & local_fixed_symbols() const { return m_local_fixed_symbols; }
	const std::vector<local_relative_symbol> & local_relative_symbols() const { return m_local_relative_symbols; }

protected:
	std::vector<global_fixed_symbol>            m_global_fixed_symbols;
	std::vector<local_fixed_symbol>             m_local_fixed_symbols;
	std::vector<local_relative_symbol>          m_local_relative_symbols;
};


#endif // MAME_EMU_DEBUG_SRCDBG_PROVIDER_H
