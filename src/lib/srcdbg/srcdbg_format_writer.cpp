// license:BSD-3-Clause
// copyright-holders:David Broman
/*********************************************************************

    srcdbg_format_writer.cpp

    Internal implementation of portions of srcdbg_api.h.

    WARNING: Tools external to MAME should only use functionality
    declared in srcdg_format.h and srcdbg_api.h.

***************************************************************************/


#include "srcdbg_api.h"
#include "srcdbg_format_writer.h"
#include "srcdbg_format.h"
#include "srcdbg_util.h"
#include "srcdbg_format_reader.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include <vector>
#include <string>
#include <algorithm>


// Simple wrapper around std::find
template<typename T>
static std::size_t find(const std::vector<T> & vec, const T & elem)
{
	auto iter = std::find(vec.cbegin(), vec.cend(), elem);
	if (iter != vec.cend())
	{
		return iter - vec.cbegin();;
	}

	return (size_t) -1;
}

// Helper to add string to vector if no matching element exists
static std::size_t find_or_push_back(std::vector<std::string> & vec, const char * new_string, u32 & size)
{
	std::string str(new_string);
	size_t idx = find(vec, str);
	if (idx != (size_t) -1)
	{
		return idx;
	}

	size += str.size() + 1;
	vec.push_back(std::move(str));
	return vec.size() - 1;
}



// Implementation of srcdbg_format_reader_callback used when the writer needs
// to import a previously-written MDI (mame_srcdbg_simp_import API), e.g., to
// facilitate linking
class writer_importer : public srcdbg_format_reader_callback
{
public:
	writer_importer(srcdbg_simple_generator & gen, short offset)
		: m_gen(gen)
		, m_offset(offset)
		, m_old_to_new_source_path_indices()
		, m_symbol_names()
	{
	}
	~writer_importer() {}

	virtual bool on_read_header_base(const mame_debug_info_header_base & header_base) override  { return true; }
	virtual bool on_read_simp_header(const mame_debug_info_simple_header & simp_header) override { return true; }
	virtual bool on_read_source_path(u32 source_path_index, std::string && source_path) override;
	virtual bool on_read_line_mapping(const srcdbg_line_mapping & line_map) override;
	virtual bool on_read_symbol_name(u32 symbol_name_index, std::string && symbol_name) override;
	virtual bool on_read_global_fixed_symbol_value(const global_fixed_symbol_value & value) override;
	virtual bool on_read_local_fixed_symbol_value(const local_fixed_symbol_value & value) override;
	virtual bool on_read_local_relative_symbol_value(const local_relative_symbol_value & value) override;

private:
	srcdbg_simple_generator & m_gen;
	short                     m_offset;
	std::vector<u32>          m_old_to_new_source_path_indices;
	std::vector<std::string>  m_symbol_names;
};

bool writer_importer::on_read_source_path(u32 source_path_index, std::string && source_path)
{
	// srcdbg_format_simp_read is required to notify in (contiguous) index order
	assert (m_old_to_new_source_path_indices.size() == source_path_index);

	u32 new_index;
	m_gen.add_source_file_path(source_path.c_str(), new_index);

	m_old_to_new_source_path_indices.push_back(new_index);
	return true;
}

bool writer_importer::on_read_line_mapping(const srcdbg_line_mapping & line_map)
{
	assert (line_map.source_file_index < m_old_to_new_source_path_indices.size());

	m_gen.add_line_mapping(
		line_map.range.address_first + m_offset,
		line_map.range.address_last + m_offset,
		m_old_to_new_source_path_indices[line_map.source_file_index],
		line_map.line_number);
	return true;
}

bool writer_importer::on_read_symbol_name(u32 symbol_name_index, std::string && symbol_name)
{
	// srcdbg_format_simp_read is required to notify in (contiguous) index order
	assert (m_symbol_names.size() == symbol_name_index);
	m_symbol_names.push_back(std::move(symbol_name));
	return true;
}

bool writer_importer::on_read_global_fixed_symbol_value(const global_fixed_symbol_value & value)
{
	assert (value.symbol_name_index < m_symbol_names.size());
	const char * sym_name = m_symbol_names[value.symbol_name_index].c_str();
	m_gen.add_global_fixed_symbol(sym_name, value.symbol_value + m_offset, value.symbol_flags);
	return true;
}

bool writer_importer::on_read_local_fixed_symbol_value(const local_fixed_symbol_value & value)
{
	assert (value.symbol_name_index < m_symbol_names.size());
	const char * sym_name = m_symbol_names[value.symbol_name_index].c_str();
	for (u32 i = 0; i < value.num_address_ranges; i++)
	{
		m_gen.add_local_fixed_symbol(
			sym_name,
			value.ranges[i].address_first + m_offset,
			value.ranges[i].address_last + m_offset,
			value.symbol_value + m_offset);
	}
	return true;
}

bool writer_importer::on_read_local_relative_symbol_value(const local_relative_symbol_value & value)
{
	assert (value.symbol_name_index < m_symbol_names.size());
	const char * sym_name = m_symbol_names[value.symbol_name_index].c_str();
	for (u32 i = 0; i < value.num_local_relative_eval_rules; i++)
	{
		m_gen.add_local_relative_symbol(
			sym_name,
			value.local_relative_eval_rules[i].range.address_first + m_offset,
			value.local_relative_eval_rules[i].range.address_last + m_offset,
			value.local_relative_eval_rules[i].reg,
			value.local_relative_eval_rules[i].reg_offset);
	}
	return true;
}


// ------------------------------------------------------------------
// srcdbg_simple_generator - Primary class for implementing
// the public writer API.
// ------------------------------------------------------------------

srcdbg_simple_generator::srcdbg_simple_generator()
	: m_output(nullptr)
	, m_source_file_paths()
	, m_line_mappings()
	, m_symbol_names()
	, m_global_fixed_symbol_values()
	, m_local_fixed_symbol_values()
	, m_local_relative_symbol_values()

{
	// Init header_base
	memcpy(&m_header.header_base.magic[0], "MDbI", 4);
	memcpy(&m_header.header_base.type[0], "simp", 4);
	m_header.header_base.version = 1;
	strncpy(
		&m_header.header_base.info[0],
		"Generated by MAME source-level debugging format writer.  https://docs.mamedev.org/debugger/index.html#generating-mame-debugging-information-files",
		sizeof(m_header.header_base.info));

	// Init rest of header
	m_header.source_file_paths_size = 0;
	m_header.num_line_mappings = 0;
	m_header.symbol_names_size = 0;
	m_header.num_global_fixed_symbol_values = 0;
	m_header.local_fixed_symbol_values_size = 0;
	m_header.local_relative_symbol_values_size = 0;
}

srcdbg_simple_generator::~srcdbg_simple_generator()
{
	if (m_output != nullptr)
	{
		fclose(m_output);
		m_output = nullptr;
	}
}

int srcdbg_simple_generator::open(const char * file_path)
{
	m_output = fopen(file_path, "wb");
	if (m_output == nullptr)
	{
		return MAME_SRCDBG_E_FOPEN_ERROR;
	}
	return MAME_SRCDBG_E_SUCCESS;
}

int srcdbg_simple_generator::add_source_file_path(const char * source_file_path, unsigned int & index)
{
	index = find_or_push_back(m_source_file_paths, source_file_path, m_header.source_file_paths_size);
	return MAME_SRCDBG_E_SUCCESS;
}

int srcdbg_simple_generator::add_line_mapping(unsigned short address_first, unsigned short address_last, unsigned int source_file_index, unsigned int line_number)
{
	if (source_file_index >= m_source_file_paths.size())
	{
		return MAME_SRCDBG_E_INVALID_SRC_IDX;
	}
	m_header.num_line_mappings++;
	srcdbg_line_mapping line_mapping = { { address_first, address_last }, source_file_index, line_number };
	m_line_mappings.push_back(line_mapping);
	return MAME_SRCDBG_E_SUCCESS;
}

int srcdbg_simple_generator::add_global_fixed_symbol(const char * symbol_name, int symbol_value, unsigned int symbol_flags)
{
	global_fixed_symbol_value global;
	global.symbol_name_index = find_or_push_back(m_symbol_names, symbol_name, m_header.symbol_names_size);
	global.symbol_value = symbol_value;
	global.symbol_flags = symbol_flags;
	m_global_fixed_symbol_values.push_back(global);
	m_header.num_global_fixed_symbol_values++;
	return MAME_SRCDBG_E_SUCCESS;
}

int srcdbg_simple_generator::add_local_fixed_symbol(const char * symbol_name, unsigned short address_first, unsigned short address_last, int symbol_value)
{
	local_fixed * entry_ptr;
	local_fixed local;
	local.symbol_name_index = find_or_push_back(m_symbol_names, symbol_name, m_header.symbol_names_size);
	auto entry = std::find_if(
		m_local_fixed_symbol_values.begin(),
		m_local_fixed_symbol_values.end(),
		[&](const local_fixed_symbol_value & elem) { return elem.symbol_name_index == local.symbol_name_index;});
	if (entry == m_local_fixed_symbol_values.end())
	{
		local.symbol_value = symbol_value;
		local.num_address_ranges = 0;
		m_local_fixed_symbol_values.push_back(local);
		entry_ptr = &m_local_fixed_symbol_values[m_local_fixed_symbol_values.size() - 1];
		m_header.local_fixed_symbol_values_size += sizeof(local_fixed_symbol_value);
	}
	else
	{
		entry_ptr = &(*entry);
	}

	address_range range;
	range.address_first = address_first;
	range.address_last = address_last;
	entry_ptr->m_ranges.push_back(range);
	m_header.local_fixed_symbol_values_size += sizeof(range);
	entry_ptr->num_address_ranges++;
	return MAME_SRCDBG_E_SUCCESS;
}

int srcdbg_simple_generator::add_local_relative_symbol(const char * symbol_name, unsigned short address_first, unsigned short address_last, unsigned char reg, int reg_offset)
{
	local_relative * entry_ptr;
	local_relative local;
	local.symbol_name_index = find_or_push_back(m_symbol_names, symbol_name, m_header.symbol_names_size);
	auto entry = std::find_if(
		m_local_relative_symbol_values.begin(),
		m_local_relative_symbol_values.end(),
		[&](const local_relative_symbol_value & elem) { return elem.symbol_name_index == local.symbol_name_index;});
	if (entry == m_local_relative_symbol_values.end())
	{
		local.num_local_relative_eval_rules = 0;
		m_local_relative_symbol_values.push_back(local);
		entry_ptr = &m_local_relative_symbol_values[m_local_relative_symbol_values.size() - 1];
		m_header.local_relative_symbol_values_size += sizeof(local_relative_symbol_value);
	}
	else
	{
		entry_ptr = &(*entry);
	}
	local_relative_eval_rule value;
	value.range.address_first = address_first;
	value.range.address_last = address_last;
	value.reg = reg;
	value.reg_offset = reg_offset;
	entry_ptr->m_values.push_back(value);
	m_header.local_relative_symbol_values_size += sizeof(value);
	entry_ptr->num_local_relative_eval_rules++;
	return MAME_SRCDBG_E_SUCCESS;
}

int srcdbg_simple_generator::import(const char * srcdbg_file_path_to_import, short offset, char * error_details, unsigned int num_bytes_error_details)
{
	std::string error;
	srcdbg_format format;
	if (!srcdbg_format_header_read(srcdbg_file_path_to_import, format, error))
	{
		std::string full_error;
		srcdbg_sprintf(
			full_error,
			"Error reading source-level debugging information file\n%s\n\n%s",
			srcdbg_file_path_to_import,
			error.c_str());

		strncpy(error_details, full_error.c_str(), num_bytes_error_details);
		return MAME_SRCDBG_E_IMPORT_FAILED;
	}

	switch (format)
	{
	case SRCDBG_FORMAT_SIMPLE:
	{
		writer_importer importer(*this, offset);
		if (!srcdbg_format_simp_read(srcdbg_file_path_to_import, importer, error))
		{
			// writer_importer always returns true from callbacks, so if we're here,
			// the reader provided an error
			assert (!error.empty());

			std::string full_error;
			srcdbg_sprintf(
				full_error,
				"Error reading source-level debugging information file\n%s\n\n%s",
				srcdbg_file_path_to_import,
				error.c_str());
			strncpy(error_details, full_error.c_str(), num_bytes_error_details);
			return MAME_SRCDBG_E_IMPORT_FAILED;
		}
		return MAME_SRCDBG_E_SUCCESS;
	}

	// FUTURE: If more file formats are invented, add cases for them here to read them

	default:
		assert(!"Unexpected source-level debugging information file format");
		return MAME_SRCDBG_E_IMPORT_FAILED;
	}

	// Unreachable
}

int srcdbg_simple_generator::close()
{
	// Write all buffered contents to file

	// Header
	m_header.source_file_paths_size = little_endianize_int32(m_header.source_file_paths_size);
	m_header.num_line_mappings = little_endianize_int32(m_header.num_line_mappings);
	m_header.symbol_names_size = little_endianize_int32(m_header.symbol_names_size);
	m_header.num_global_fixed_symbol_values = little_endianize_int32(m_header.num_global_fixed_symbol_values);
	m_header.local_fixed_symbol_values_size = little_endianize_int32(m_header.local_fixed_symbol_values_size);
	m_header.local_relative_symbol_values_size = little_endianize_int32(m_header.local_relative_symbol_values_size);
	FWRITE_OR_RETURN(&m_header, sizeof(m_header), 1, m_output);

	// Source file paths
	for (const std::string & path : m_source_file_paths)
	{
		FWRITE_OR_RETURN(path.c_str(), path.size() + 1, 1, m_output);
	}

	// Line mappings
	for (srcdbg_line_mapping & map : m_line_mappings)
	{
		map.range.address_first = little_endianize_int16(map.range.address_first);
		map.range.address_last = little_endianize_int16(map.range.address_last);
		map.source_file_index = little_endianize_int32(map.source_file_index);
		map.line_number = little_endianize_int32(map.line_number);
		FWRITE_OR_RETURN(&map, sizeof(map), 1, m_output);
	}

	// Symbol names
	for (const std::string & symname : m_symbol_names)
	{
		FWRITE_OR_RETURN(symname.c_str(), symname.size() + 1, 1, m_output);
	}

	// global fixed symbols
	for (global_fixed_symbol_value & sym : m_global_fixed_symbol_values)
	{
		sym.symbol_name_index = little_endianize_int32(sym.symbol_name_index);
		sym.symbol_value = little_endianize_int32(sym.symbol_value);
		sym.symbol_flags = little_endianize_int32(sym.symbol_flags);
		FWRITE_OR_RETURN(&sym, sizeof(sym), 1, m_output);
	}

	// local fixed symbols
	for (local_fixed & sym : m_local_fixed_symbol_values)
	{
		if (sym.m_ranges.size() == 0)
		{
			continue;
		}

		// Write the fixed length portion first
		sym.symbol_name_index = little_endianize_int32(sym.symbol_name_index);
		sym.symbol_value = little_endianize_int32(sym.symbol_value);
		sym.num_address_ranges = little_endianize_int32(sym.num_address_ranges);
		FWRITE_OR_RETURN(&sym, sizeof(local_fixed_symbol_value), 1, m_output);

		// Write the var length portion next (address ranges)
		for (address_range & range : sym.m_ranges)
		{
			range.address_first = little_endianize_int16(range.address_first);
			range.address_last = little_endianize_int16(range.address_last);
			FWRITE_OR_RETURN(&range, sizeof(range), 1, m_output);
		}
	}

	// local relative symbols
	for (local_relative & sym : m_local_relative_symbol_values)
	{
		if (sym.m_values.size() == 0)
		{
			continue;
		}

		// Write the fixed length portion first
		sym.symbol_name_index = little_endianize_int32(sym.symbol_name_index);
		sym.num_local_relative_eval_rules = little_endianize_int32(sym.num_local_relative_eval_rules);
		FWRITE_OR_RETURN(&sym, sizeof(local_relative_symbol_value), 1, m_output);

		// Write the var length portion next (eval rules)
		for (local_relative_eval_rule & rule : sym.m_values)
		{
			rule.range.address_first = little_endianize_int16(rule.range.address_first);
			rule.range.address_last = little_endianize_int16(rule.range.address_last);
			rule.reg_offset = little_endianize_int32(rule.reg_offset);
			FWRITE_OR_RETURN(&rule, sizeof(rule), 1, m_output);
		}
	}

	// All done
	FCLOSE_OR_RETURN(m_output);
	m_output = nullptr;
	return MAME_SRCDBG_E_SUCCESS;
}
