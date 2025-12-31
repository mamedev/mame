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
#include "srcdbg_format.h"
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
	virtual void complete_local_relative_initialization() override;
	virtual u32 num_files() const override { return m_source_file_paths.size(); }
	virtual const source_file_path & file_index_to_path(u32 file_index) const override { return m_source_file_paths[file_index]; };
	virtual std::optional<u32> file_path_to_index(const char * file_path) const override;
	virtual void file_line_to_address_ranges(u32 file_index, u32 line_number, std::vector<address_range> & ranges) const override;
	virtual bool address_to_file_line (offs_t address, file_line & loc) const override;
	virtual const std::vector<global_fixed_symbol> & global_fixed_symbols() const override { return m_global_fixed_symbols; };
	virtual const std::vector<local_fixed_symbol> & local_fixed_symbols() const override { return m_local_fixed_symbols; };
	virtual const std::vector<local_relative_symbol> & local_relative_symbols() const override { return m_local_relative_symbols; };
	virtual void set_offset(s32 offset) override { m_offset = offset; }

private:
	struct address_line
	{
		u16 address_first;
		u16 address_last;
		u32 line_number;
	};

	class local_relative_eval_rule_internal
	{
	public:
		local_relative_eval_rule_internal(std::pair<offs_t,offs_t> && range, char reg, s32 reg_offset)
			: m_range(std::move(range))
			, m_reg(reg)
			, m_reg_offset(reg_offset)
		{}
		std::pair<offs_t,offs_t> m_range;
		char m_reg;
		s32 m_reg_offset;
	};

	class local_relative_symbol_internal
	{
	public:
		local_relative_symbol_internal(const std::string & name, std::vector<local_relative_eval_rule_internal> eval_rules)
			: m_name(name)
			, m_eval_rules(eval_rules)
		{
		}
		std::string m_name;
		std::vector<local_relative_eval_rule_internal> m_eval_rules;
	};

	virtual s32 get_offset() const override { return m_offset; }

	const running_machine& m_machine;
	std::vector<source_file_path>               m_source_file_paths;      // Starting points for source file path strings
	std::vector<srcdbg_line_mapping>            m_linemaps_by_address;    // a list of srcdbg_line_mappings, sorted by address
	std::vector<std::vector<address_line>>      m_linemaps_by_line;       // m_linemaps_by_line[i] is a list of address/line pairs,
																		  // sorted by line, from file #i
	std::vector<global_fixed_symbol>            m_global_fixed_symbols;
	std::vector<local_fixed_symbol>             m_local_fixed_symbols;
	std::vector<local_relative_symbol_internal> m_local_relative_symbols_internal;
	std::vector<local_relative_symbol>          m_local_relative_symbols;
	s32                                         m_offset;
};


// Callbacks implementation for reading the "simple" source-debugging
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
