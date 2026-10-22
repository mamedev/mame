// license:BSD-3-Clause
// copyright-holders:David Broman
/*********************************************************************

    srcdbg_provider_simple.h

    Implementation of interface to source-debugging info for the
    "simple" format

***************************************************************************/


#ifndef MAME_EMU_DEBUG_SRCDBG_PROVIDER_SIMPLE_H
#define MAME_EMU_DEBUG_SRCDBG_PROVIDER_SIMPLE_H

#pragma once

#include "srcdbg_provider.h"
#include "../../lib/srcdbg/srcdbg_format.h"
#include "srcdbg_format_reader.h"

class srcdbg_import;

// Implementation of srcdbg_provider_base specific to the "simple" source-debugging
// information file format
class srcdbg_provider_simple : public srcdbg_provider_base
{
	friend class srcdbg_import;

public:
	srcdbg_provider_simple(const running_machine& machine);
	~srcdbg_provider_simple() { }
	virtual u32 num_files() const override { return m_source_file_paths.size(); }
	virtual bool file_index_to_path(u32 file_index, const source_file_path ** path) const override { *path = m_source_file_paths.data()+file_index; return true;};
	virtual std::optional<u32> file_path_to_index(const char * file_path) const override;
	virtual void file_line_to_address_ranges(u32 file_index, u32 line_number, std::vector<address_range> & ranges) const override;
	virtual bool address_to_file_line (offs_t address, file_line & loc) const override;
	virtual bool is_global_fixed_sym_in_scope(u32 global_idx) const override { return true; }
	virtual u64 get_global_fixed_sym_value(u32 global_idx) const override;
	virtual bool is_local_fixed_sym_in_scope(u32 local_fixed_idx) const override;
	virtual u64 get_local_fixed_sym_value(u32 local_fixed_idx) const override;
	virtual bool is_local_relative_sym_in_scope(u32 m_local_relative_idx) const override;
	virtual u64 get_local_relative_sym_value(u32 local_relative_idx, symbol_table & symtable) const override;

private:
	struct address_line
	{
		u16 address_first;
		u16 address_last;
		u32 line_number;
	};

	s32 get_offset() const;
	u64 evaluate_local_relative_rule(const device_state_interface * state, const char * name, const local_relative_eval_rule & val, symbol_table & symtable) const;

	const running_machine& m_machine;
	std::vector<source_file_path>               m_source_file_paths;      // Starting points for source file path strings
	std::vector<srcdbg_line_mapping>            m_linemaps_by_address;    // a list of srcdbg_line_mappings, sorted by address
	std::vector<std::vector<address_line>>      m_linemaps_by_line;       // m_linemaps_by_line[i] is a list of address/line pairs,
																		  // sorted by line, from file #i
};


// Implementation of callbacks for reading the "simple" source-debugging
// information file format, and importing it into srcdbg_provider_simple
class srcdbg_import : public srcdbg_format_reader_callback
{
public:
	srcdbg_import(srcdbg_provider_simple & srcdbg_simple);
	virtual bool on_read_header_base(const mame_debug_info_header_base & header_base) override { return true; }
	virtual bool on_read_simp_header(const mame_debug_info_simple_header & simp_header) override { return true; }
	virtual bool on_read_source_path(u32 source_path_index, std::string && source_path) override;
	virtual bool end_read_source_paths() override;
	virtual bool on_read_line_mapping(const srcdbg_line_mapping & line_map) override;
	virtual bool end_read_line_mappings() override;
	virtual bool on_read_symbol_name(u32 symbol_name_index, std::string && symbol_name) override;
	virtual bool on_read_global_fixed_symbol_value(const global_fixed_symbol_value & value) override;
	virtual bool on_read_local_fixed_symbol_value(const local_fixed_symbol_value & value) override;
	virtual bool on_read_local_relative_symbol_value(const local_relative_symbol_value & value) override;

private:
	void generate_local_path(const std::string & built, std::string & local);
	void apply_source_prefix_map(std::string & local);

	srcdbg_provider_simple & m_srcdbg_simple;
	std::vector<std::string> m_symbol_names;
	std::string              m_normalized_debug_source_path_map;
};

#endif // MAME_EMU_DEBUG_SRCDBG_PROVIDER_SIMPLE_H
