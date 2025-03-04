// license:BSD-3-Clause
// copyright-holders:David Broman
/*********************************************************************

    srcdbg_provider.cpp

    Factory to create format-specific implementation of
    format-agnostic interface to source-debugging info

***************************************************************************/


#include "emu.h"
#include "srcdbg_provider_simple.h"
#include "srcdbg_provider.h"
#include "srcdbg_format_reader.h"
#include "srcdbg_api.h"

#include "emuopts.h"


// Create and initialize debug info provider of the correct type by reading
// the debug info file
//
// static
std::unique_ptr<srcdbg_provider_base> srcdbg_provider_base::create_debug_info(running_machine &machine)
{
	const char * di_path = machine.options().srcdbg_info();
	if (di_path[0] == 0)
	{
		return nullptr;
	}

	std::string error;
	srcdbg_format format;
	if (!srcdbg_format_header_read(di_path, format, error))
	{
		throw emu_fatalerror("Error reading source-level debugging information file\n%s\n\n%s", di_path, error.c_str());
	}

	switch (format)
	{
	case SRCDBG_FORMAT_SIMPLE:
	{
		std::unique_ptr<srcdbg_provider_simple> ret = std::make_unique<srcdbg_provider_simple>(machine);
		srcdbg_import importer(*ret);
		if (!srcdbg_format_simp_read(di_path, importer, error))
		{
			if (!error.empty())
			{
				throw emu_fatalerror("Error importing source-level debugging information\n\n%s", error.c_str());
			}
			return nullptr;
		}
		return ret;
	}

	// FUTURE: If more file formats are invented, add cases for them here to read them

	default:
		assert(!"Unexpected source-level debugging information file format");
		return nullptr;
	}

	assert (!"This code should be unreachable");
	return nullptr;
}


// Return symbol_table objects for globals and locals present in the source-level
// debugging information file.  Establish the following symbol table parent chain:
// *symtable_srcdbg_locals -> *symtable_srcdbg_globals -> parent
void srcdbg_provider_base::get_srcdbg_symbols(
	symbol_table ** symtable_srcdbg_globals,       // [out] global variables from source-level debugging info
	symbol_table ** symtable_srcdbg_locals,        // [out] local variables from source-level debugging info
	symbol_table * parent,                         // [in] symbol_table to establish as parent of globals
	device_t * device,                             // [in] device to use when initializing symbol_tables
	const device_state_interface * state) const    // [in] device_state_entry for accessing PC for scoped locals
{
	// User-provided offset to accomodate relocated code
	s32 offset = get_offset();

	// Global fixed symbols
	const std::vector<srcdbg_provider_base::global_fixed_symbol> & srcdbg_global_symbols = global_fixed_symbols();
	*symtable_srcdbg_globals = new symbol_table(parent->machine(), symbol_table::SRCDBG_GLOBALS, parent, device);
	for (const srcdbg_provider_base::global_fixed_symbol & sym : srcdbg_global_symbols)
	{
		// Apply offset to symbol when appropriate
		offs_t value = sym.value();
		if ((sym.flags() & MAME_SRCDBG_SYMFLAG_CONSTANT) == 0)
		{
			value += offset;
		}

		(*symtable_srcdbg_globals)->add(sym.name(), value);
	}

	// Local symbols require a PC getter function so they can test if they're
	// currently in scope
	auto pc_getter_binding = std::bind(&device_state_entry::value, state->state_find_entry(STATE_GENPC));

	// Local fixed symbols
	const std::vector<srcdbg_provider_base::local_fixed_symbol> & srcdbg_local_fixed_symbols = local_fixed_symbols();
	*symtable_srcdbg_locals = new symbol_table(parent->machine(), symbol_table::SRCDBG_LOCALS, *symtable_srcdbg_globals, device);
	for (const srcdbg_provider_base::local_fixed_symbol & sym : srcdbg_local_fixed_symbols)
	{
		(*symtable_srcdbg_locals)->add(sym.name(), pc_getter_binding, sym.ranges(), sym.value());
	}

	// Local "relative" symbols (values are offsets to a register)
	const std::vector<srcdbg_provider_base::local_relative_symbol> & srcdbg_local_relative_symbols = local_relative_symbols();
	for (const srcdbg_provider_base::local_relative_symbol & sym : srcdbg_local_relative_symbols)
	{
		(*symtable_srcdbg_locals)->add(sym.name(), pc_getter_binding, sym.ranges());
	}
}
