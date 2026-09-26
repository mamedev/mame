// license:BSD-3-Clause
// copyright-holders:David Broman
/*********************************************************************

    srcdbg_info.cpp

    Implementation of srcdbg_provider_base (the interface to
	source-debugging info consumed by the debugger) which
	aggregates 1 or more loaded source-debugging files.

***************************************************************************/


#include "emu.h"
#include "srcdbg_info.h"
#include "srcdbg_api.h"

#include "emuopts.h"
#include "fileio.h"

#include <filesystem>

//-------------------------------------------------
// create_debug_info - Factory to instantiate
// srcdbg_info, which in turn calls factories that
// load individual srcdbg info files and instantiate their 
// srcdbg_provider_base implementations.  This
// returns nullptr if there's an error or the user
// is not running with source-debugging enabled.
//-------------------------------------------------

// static 
srcdbg_info * srcdbg_info::create_debug_info(running_machine &machine)
{
	// Get paths to all srcdbg info files specified on command line
	const char * di_paths = machine.options().srcdbginfo();
	if (di_paths[0] == 0)
	{
		return nullptr;
	}

	// Instantiate aggregator to return
	srcdbg_info * ret = new srcdbg_info(machine);

	// Load each file
	path_iterator path_it(di_paths);
	std::string di_path;
	while (path_it.next(di_path))
	{
		srcdbg_provider_base * provider = srcdbg_provider_base::create_debug_info(machine, di_path);
		if (provider == nullptr)
		{
			return nullptr;
		}

		srcdbg_provider_entry sp(di_path, provider);
		ret->m_providers.push_back(std::move(sp));
	}

	ret->coalesce();
	return ret;
}


//-------------------------------------------------
// srcdbg_info constructor
//-------------------------------------------------

srcdbg_info::srcdbg_info(const running_machine& machine)
	: m_agg_file_to_provider_files()
	, m_provider_file_to_agg_file()
	, m_providers()
	, m_offset(machine.options().srcdbg_offset())
	, m_view_needs_full_refresh(true)
{
}


//-------------------------------------------------
// srcdbg_info - get_srcdbg_symbols
// Returns symbol_table objects populated with
// globals and locals from srcdbg info
//-------------------------------------------------

void srcdbg_info::get_srcdbg_symbols(
		symbol_table * symtable_srcdbg_globals,
		symbol_table * symtable_srcdbg_locals,
		const device_state_interface * state) const
{
	for (const srcdbg_provider_entry & sp : m_providers)
	{
		if (!sp.enabled())
		{
			continue;
		}

		// Global fixed symbols
		const std::vector<srcdbg_provider_base::global_fixed_symbol> & srcdbg_global_symbols = 
			sp.c_provider()->global_fixed_symbols();
		for (const srcdbg_provider_base::global_fixed_symbol & sym : srcdbg_global_symbols)
		{
			// Apply offset to symbol when appropriate
			offs_t value = sym.value();
			if ((sym.flags() & MAME_SRCDBG_SYMFLAG_CONSTANT) == 0)
			{
				value += m_offset;
			}

			symtable_srcdbg_globals->add(sym.name(), value);
		}

		// Local symbols require a PC getter function so they can test if they're
		// currently in scope
		auto pc_getter_binding = std::bind(&device_state_entry::value, state->state_find_entry(STATE_GENPC));

		// Local fixed symbols
		const std::vector<srcdbg_provider_base::local_fixed_symbol> & srcdbg_local_fixed_symbols = 
			sp.c_provider()->local_fixed_symbols();
		for (const srcdbg_provider_base::local_fixed_symbol & sym : srcdbg_local_fixed_symbols)
		{
			symtable_srcdbg_locals->add(sym.name(), pc_getter_binding, sym.ranges(), sym.value());
		}

		// Local "relative" symbols (values are offsets to a register)
		const std::vector<srcdbg_provider_base::local_relative_symbol> & srcdbg_local_relative_symbols = 
			sp.c_provider()->local_relative_symbols();
		for (const srcdbg_provider_base::local_relative_symbol & sym : srcdbg_local_relative_symbols)
		{
			symtable_srcdbg_locals->add(sym.name(), pc_getter_binding, sym.ranges());
		}
	}
}


//-------------------------------------------------
// srcdbg_info - complete_local_relative_initialization
// Called later during startup, after device_state_interfaces
// are available. Generates expressions required to implement
// local relative symbol evaluation rules.
//-------------------------------------------------

void srcdbg_info::complete_local_relative_initialization()
{
	for (srcdbg_provider_entry & sp : m_providers)
	{
		if (!sp.enabled())
		{
			continue;
		}

		sp.provider()->complete_local_relative_initialization();
	}
}


//-------------------------------------------------
// srcdbg_info - file_index_to_path
// Returns path associated with aggregated file index
//-------------------------------------------------

bool srcdbg_info::file_index_to_path(u32 file_index, const source_file_path ** path) const
{ 
	std::vector<provider_file> provider_files;
	if (!file_index_to_provider_files(file_index, provider_files))
	{
		return false;
	}

	for (const provider_file & pf : provider_files)
	{
		std::size_t prov_idx = pf.m_provider_idx;
		if (!m_providers[prov_idx].enabled())
		{
			continue;
		}

		u32 local_file_idx = pf.m_file_idx;
		return m_providers[prov_idx].c_provider()->file_index_to_path(local_file_idx, path);
	}

	return false;
}



//-------------------------------------------------
// srcdbg_info - file_path_to_index
// Looks up source file path in aggregated srcdbg
// info.  Returns the associated aggregated
// file index, or no value if no such path
// exists (e.g., b/c the owning provider is disabled)
//-------------------------------------------------

std::optional<u32> srcdbg_info::file_path_to_index(const char * file_path) const
{
	// Find first enabled provider who claims this path, to look up
	// the aggregated file index
	for (offs_t provider_idx = 0; provider_idx < m_providers.size(); provider_idx++)
	{
		const srcdbg_provider_entry & sp = m_providers[provider_idx];
		if (!sp.enabled())
		{
			continue;
		}

		std::optional<u32> file_idx = sp.c_provider()->file_path_to_index(file_path);
		if (file_idx.has_value())
		{
			return m_provider_file_to_agg_file[provider_idx][file_idx.value()];
		}
	}

	return std::optional<u32>();
}



//-------------------------------------------------
// srcdbg_info - file_index_to_provider_files
// Private helper to look up aggregated file index, and return list of
// (provider, local index) pairs
//-------------------------------------------------

bool srcdbg_info::file_index_to_provider_files(u32 file_index, std::vector<provider_file> & ret) const
{
	if (file_index >= m_agg_file_to_provider_files.size())
	{
		return false;
	}

	ret = m_agg_file_to_provider_files[file_index];
	return true;
}



//-------------------------------------------------
// srcdbg_info - file_line_to_address_ranges
// Looks up an aggregated file index & line number,
// and returns a list of address_range instances
// corresponding to that source line.
//-------------------------------------------------

void srcdbg_info::file_line_to_address_ranges(u32 file_index, u32 line_number, std::vector<address_range> & ranges) const
{
	std::vector<provider_file> provider_files;	
	if (!file_index_to_provider_files(file_index, provider_files))
	{
		return;
	}

	// Multiple providers might know about this file.  Find the first one who
	// knows about this line
	for (u32 i = 0; i < provider_files.size(); i++)
	{
		std::size_t provider_idx = provider_files[i].m_provider_idx;
		const srcdbg_provider_entry & provider = m_providers[provider_idx];
		if (!provider.enabled())
		{
			continue;
		}

		provider.c_provider()->file_line_to_address_ranges(provider_files[i].m_file_idx, line_number, ranges);
		if (ranges.size() > 0)
		{
			return;
		}
	}
}


//-------------------------------------------------
// srcdbg_info - address_to_file_line
// Looks up an address and returns the aggregated
// source file index & line number corresponding
// to that address
//-------------------------------------------------

bool srcdbg_info::address_to_file_line(offs_t address, file_line & loc) const
{
	for (offs_t provider_idx = 0; provider_idx < m_providers.size(); provider_idx++)
	{
		const srcdbg_provider_entry & sp = m_providers[provider_idx];
		if (!sp.enabled())
		{
			continue;
		}

		if (sp.c_provider()->address_to_file_line(address, loc))
		{
			// Convert from provider index space into coalesced index space
			loc.set(
				m_provider_file_to_agg_file[provider_idx][loc.file_index()],
				loc.line_number());
			return true;
		}
	}

	return false;
}


//-------------------------------------------------
// srcdbg_info - update_view_needs_full_refresh
// Returns and then resets bool indicating whether 
// debug_view_sourcecode needs a full refresh
// (e.g., b/c a source-debugging info was enabled
// or disabled during MMU-aware debugging)
//-------------------------------------------------

bool srcdbg_info::update_view_needs_full_refresh()
{
	bool ret = m_view_needs_full_refresh;
	m_view_needs_full_refresh = false;
	return ret;
}


//-------------------------------------------------
// srcdbg_info - coalesce
// Private helper called on startup to build maps
// between aggregated indices and provider-local indices.
//-------------------------------------------------

void srcdbg_info::coalesce()
{
	namespace fs = std::filesystem;

	m_provider_file_to_agg_file.clear();
	m_agg_file_to_provider_files.clear();

	// Keep track of duplicate file paths.
	struct path_and_agg_idx
	{
		path_and_agg_idx(const char * path, u32 agg_idx)
		{
			m_path = path;
			m_agg_idx = agg_idx;
		}
		const char *	m_path;
		u32				m_agg_idx;
	};
	std::vector<path_and_agg_idx> path_and_agg_idxs;

	// Ensure m_provider_file_to_agg_file is pre-sized so as we encounter
	// each provider, we'll always have an entry ready for it
	m_provider_file_to_agg_file.reserve(m_providers.size());
	m_provider_file_to_agg_file.resize(m_providers.size());

	for (offs_t provider_idx = 0; provider_idx < m_providers.size(); provider_idx++)
	{
		const srcdbg_provider_entry & sp = m_providers[provider_idx];
		const srcdbg_provider_base * provider = sp.c_provider();
		if (provider->num_files() == 0)
		{
			continue;
		}

		m_provider_file_to_agg_file[provider_idx] = std::vector<u32>();
		for (u32 file_idx = 0; file_idx < provider->num_files(); file_idx++)
		{
			const source_file_path * sfp = nullptr;
			if (!provider->file_index_to_path(file_idx, &sfp))
			{
				break;
			}

			// If this file has already been encountered from another provider,
			// reuse the same aggregated file index
			u32 agg_file_idx = u32(-1);
			
			for (const path_and_agg_idx & path_agg : path_and_agg_idxs)
			{
				std::error_code err;
				bool ret = fs::equivalent(path_agg.m_path, sfp->local(), err);
				if (!err && ret)
				{
					// Already seen.  Reuse its aggregated file index
					agg_file_idx = path_agg.m_agg_idx;
					break;
				}
			}

			if (agg_file_idx == u32(-1))
			{
				// New.  Use the next available aggregated index
				agg_file_idx = m_agg_file_to_provider_files.size();
				m_agg_file_to_provider_files.push_back(std::vector<provider_file>());
				path_and_agg_idxs.push_back(path_and_agg_idx(sfp->local(), agg_file_idx));
			}

			// (provider_idx, file_idx) maps to agg_file_idx
			m_provider_file_to_agg_file[provider_idx].push_back(agg_file_idx);

			// To the agg_file_idx list, append (provider_idx, file_idx)
			m_agg_file_to_provider_files[agg_file_idx].push_back(provider_file(provider_idx, file_idx));
		}
	}
}


//-------------------------------------------------
// srcdbg_info - disenable_provider
// Enable / disable a given provider aggregated by
// this srcdbg_info.  Useful for MMU-aware debugging
//-------------------------------------------------

srcdbg_info::disenable_retcode srcdbg_info::disenable_provider(u64 index, bool enable)
{
	// std::vector<srcdbg_info::srcdbg_provider_entry> & providers = srcdbg->providers();
	if (index >= m_providers.size())
	{
		return disenable_retcode::BAD_IDX;
	}

	srcdbg_info::srcdbg_provider_entry & sp = m_providers[index];
	if (sp.enabled() == enable)
	{
		return disenable_retcode::NO_CHANGE;
	}

	sp.set_enabled(enable);

	// Next time debug_view_sourcecode updates itself, it
	// should do a full refresh
	m_view_needs_full_refresh = true;

	return disenable_retcode::SUCCESS;
}
