// license:BSD-3-Clause
// copyright-holders:David Broman
/*********************************************************************

    srcdbg_info.h

    Implementation of srcdbg_provider_base (the interface to
    source-debugging info consumed by the debugger) which
    aggregates 1 or more loaded source-debugging files.

***************************************************************************/


#ifndef MAME_EMU_DEBUG_SRCDBG_INFO_H
#define MAME_EMU_DEBUG_SRCDBG_INFO_H

#pragma once

#include "srcdbg_provider.h"


// Concrete implementation of srcdbg_provider_base used by the debugger
// to access source-file-level debugging information.
//
// When multiple source-file-level debugging information files are loaded
// (e.g., to support MMU-aware debugging), this aggregates all those
// infos, and keeps track of which are enabled / disabled.
class srcdbg_info
{
public:
	// A single info aggregated by srcdbg_info
	class srcdbg_provider_entry
	{
		friend class srcdbg_info;

	public:
		srcdbg_provider_entry(const std::string & name, srcdbg_provider_base * provider)
			: m_name(name)
			, m_provider(provider)
			, m_enabled(true)
		{
		}

		srcdbg_provider_entry(srcdbg_provider_entry && that)
			: m_name(std::move(that.m_name))
			, m_provider(std::move(that.m_provider))
			, m_enabled(that.m_enabled)
		{
		}

		~srcdbg_provider_entry() {}

		const std::string & name() const { return m_name; }
		const srcdbg_provider_base * c_provider() const { return m_provider.get(); }
		srcdbg_provider_base * provider() { return m_provider.get(); }
		bool enabled() const { return m_enabled; }

	private:
		void set_enabled(bool enabled) { m_enabled = enabled; }

		std::string m_name;
		std::unique_ptr<srcdbg_provider_base> m_provider;
		bool m_enabled;
	};

	enum class disenable_retcode { SUCCESS, BAD_IDX, NO_CHANGE };

	static srcdbg_info * create_debug_info(running_machine &machine);

	srcdbg_info(const running_machine& machine);
	~srcdbg_info() { }

	void get_srcdbg_symbols(
		symbol_table * symtable_srcdbg_globals,
		symbol_table * symtable_srcdbg_locals,
		const device_state_interface * state);
	// u64 get_global_fixed_sym_value(const srcdbg_provider_entry & sp, const srcdbg_provider_base::global_fixed_symbol & sym) const;
	bool is_global_fixed_sym_in_scope(const char * name) const;
	u64 get_global_fixed_sym_value(const char * name) const;
	bool is_local_sym_in_scope(const char * name) const;
	u64 get_local_sym_value(const char * name, symbol_table & symtable) const;

	u32 num_files() const { return m_agg_file_to_provider_files.size(); }
	bool file_index_to_path(u32 file_index, const srcdbg_provider_base::source_file_path ** path) const;
	std::optional<u32> file_path_to_index(const char * file_path) const;
	void file_line_to_address_ranges(u32 file_index, u32 line_number, std::vector<srcdbg_provider_base::address_range> & ranges) const;
	bool address_to_file_line (offs_t address, file_line & loc) const;

	void set_offset(s32 offset) { m_offset = offset; }
	s32 get_offset() const { return m_offset; }

	const std::vector<srcdbg_provider_entry> & c_providers() const { return m_providers; }
	std::vector<srcdbg_provider_entry> & providers() { return m_providers; }
	bool update_view_needs_full_refresh();
	disenable_retcode disenable_provider(u64 index, bool enable);

private:
	// Pairs a provider index with a file index (scoped to the provider)
	struct provider_file
	{
		provider_file(std::size_t provider_idx, u32 file_idx)
		{
			m_provider_idx = provider_idx;
			m_file_idx = file_idx;
		}

		std::size_t     m_provider_idx;
		u32             m_file_idx;
	};

	// Pairs a provider with an index into its list of global syms
	struct provider_global
	{
		provider_global(const srcdbg_provider_entry & sp, u32 global_idx)
			: m_sp(sp)
			, m_global_idx(global_idx)
		{
		}

		const srcdbg_provider_entry & m_sp;
		u32                           m_global_idx;
	};

	// Associates a provider with two indices: one into its list of local fixed
	// syms, and one into its list of local-relative syms
	struct provider_local
	{
		provider_local(const srcdbg_provider_entry & sp)
			: m_sp(sp)
			, m_local_fixed_idx()
			, m_local_relative_idx()
		{
		}

		const srcdbg_provider_entry & m_sp;
		std::optional<u32>            m_local_fixed_idx;
		std::optional<u32>            m_local_relative_idx;
	};

	void coalesce();
	bool file_index_to_provider_files(u32 file_index, std::vector<provider_file> & ret) const;

	// agg file index to list of (provider index, local file index) pairs
	// [agg_file] = { (provider_idx, local_file_idx), ... }
	std::vector<std::vector<provider_file>>  m_agg_file_to_provider_files;

	// provider index + local file index to agg file index
	// [provider_idx[local_file_idx]] = agg_file
	std::vector<std::vector<u32>>                                 m_provider_file_to_agg_file;

	// these map a symbol name to a list of provider symbols with that name
	std::unordered_map<std::string, std::vector<provider_global>> m_provider_globals;
	std::unordered_map<std::string, std::vector<provider_local>>  m_provider_locals;

	std::vector<srcdbg_provider_entry>                            m_providers;
	s32                                                           m_offset;
	bool                                                          m_view_needs_full_refresh;
};


#endif // MAME_EMU_DEBUG_SRCDBG_INFO_H
